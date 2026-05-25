//                 Transient Heat Dataset Generator (for GNN surrogate)
//
// Compile with: cmake --build build --target gen_heat_dataset
//
// Sample run:   ./build/gen_heat_dataset -n 64 -d heat_dataset
//
// Description:  Batch data factory built on top of MFEM example 16. It solves
//               the transient nonlinear heat equation
//                   du/dt = div( (kappa + alpha*u) grad u )
//               many times over randomized material parameters (alpha, kappa)
//               and randomized initial conditions (a "hot blob" of random
//               center / radius / temperature), and dumps each run as a graph
//               time-series suitable for training a graph neural network
//               surrogate (MeshGraphNet-style autoregressive next-step model).
//
//               The geometry / mesh is fixed across samples, so the graph
//               (node coordinates + edges) is identical for every sample and is
//               written exactly once at the dataset root. Each sample then only
//               stores its scalar parameters (manifest.csv row) and the full
//               temperature trajectory frames.csv (num_frames x num_nodes).
//
//               Output layout (under --out-dir):
//                 dataset.json      global metadata + parameter ranges
//                 graph_nodes.csv   N rows: x,y            (dof order)
//                 graph_edges.csv   M rows: src,dst        (undirected, once)
//                 manifest.csv      one row per sample with its parameters
//                 sample_XXXXX/frames.csv   num_frames x N temperature field

#include "mfem.hpp"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <sys/stat.h>
#include <utility>

using namespace std;
using namespace mfem;

// Nonlinear conduction operator, identical in form to MFEM example 16:
//   du/dt = M^{-1}(-K u),  K = diffusion with coefficient (kappa + alpha*u).
class ConductionOperator : public TimeDependentOperator
{
protected:
   FiniteElementSpace &fespace;
   Array<int> ess_tdof_list; // empty: all boundaries are natural (Neumann)

   BilinearForm *M;
   BilinearForm *K;

   SparseMatrix Mmat, Kmat;
   SparseMatrix *T; // T = M + dt K
   real_t current_dt;

   CGSolver M_solver;
   DSmoother M_prec;

   CGSolver T_solver;
   DSmoother T_prec;

   real_t alpha, kappa;

   mutable Vector z;

public:
   ConductionOperator(FiniteElementSpace &f, real_t alpha, real_t kappa,
                      const Vector &u);

   void Mult(const Vector &u, Vector &du_dt) const override;
   void ImplicitSolve(const real_t dt, const Vector &u, Vector &k) override;

   void SetParameters(const Vector &u);

   ~ConductionOperator() override;
};

// Parameterized initial condition: a circular "hot blob".
struct BlobIC
{
   real_t cx, cy, r, hot, bg;
   real_t operator()(const Vector &x) const
   {
      const real_t dx = x[0] - cx;
      const real_t dy = (x.Size() > 1) ? x[1] - cy : 0.0;
      return (std::sqrt(dx * dx + dy * dy) < r) ? hot : bg;
   }
};

// Recursively create a directory path (like `mkdir -p`).
static void make_dir(const string &path)
{
   string cur;
   for (size_t i = 0; i < path.size(); ++i)
   {
      cur += path[i];
      const bool at_sep = (path[i] == '/');
      const bool at_end = (i + 1 == path.size());
      if ((at_sep || at_end) && cur != "/" && cur != "." && cur != "./")
      {
         if (mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST)
         {
            cerr << "Warning: could not create directory '" << cur
                 << "' (errno " << errno << ")" << endl;
         }
      }
   }
}

int main(int argc, char *argv[])
{
   // 1. Options.
   const char *mesh_file = "../data/star.mesh";
   const char *out_dir = "heat_dataset";
   int ref_levels = 3;
   int order = 1;             // order 1 -> graph nodes == mesh vertices
   int num_samples = 64;
   int ode_solver_type = 23;  // SDIRK33Solver, as in ex16
   real_t t_final = 0.5;
   real_t dt = 1.0e-2;
   int save_every = 1;
   int seed = 1;

   // Randomized parameter ranges.
   real_t alpha_min = 0.0, alpha_max = 2.0e-2;
   real_t kappa_min = 0.1, kappa_max = 1.0;
   real_t r_min = 0.2, r_max = 0.8;
   real_t hot_min = 1.5, hot_max = 3.0;
   real_t bg = 1.0;

   OptionsParser args(argc, argv);
   args.AddOption(&mesh_file, "-m", "--mesh", "Mesh file to use.");
   args.AddOption(&out_dir, "-d", "--out-dir", "Output dataset directory.");
   args.AddOption(&ref_levels, "-r", "--refine",
                  "Number of uniform mesh refinements.");
   args.AddOption(&order, "-o", "--order", "Finite element polynomial order.");
   args.AddOption(&num_samples, "-n", "--num-samples",
                  "Number of FEM runs (samples) to generate.");
   args.AddOption(&ode_solver_type, "-s", "--ode-solver",
                  ODESolver::Types.c_str());
   args.AddOption(&t_final, "-tf", "--t-final", "Final time.");
   args.AddOption(&dt, "-dt", "--time-step", "Time step.");
   args.AddOption(&save_every, "-se", "--save-every",
                  "Record one frame every N time steps.");
   args.AddOption(&seed, "-seed", "--seed", "Base RNG seed.");
   args.AddOption(&alpha_min, "-amin", "--alpha-min", "Min alpha.");
   args.AddOption(&alpha_max, "-amax", "--alpha-max", "Max alpha.");
   args.AddOption(&kappa_min, "-kmin", "--kappa-min", "Min kappa.");
   args.AddOption(&kappa_max, "-kmax", "--kappa-max", "Max kappa.");
   args.AddOption(&r_min, "-rmin", "--radius-min", "Min blob radius.");
   args.AddOption(&r_max, "-rmax", "--radius-max", "Max blob radius.");
   args.AddOption(&hot_min, "-hmin", "--hot-min", "Min blob temperature.");
   args.AddOption(&hot_max, "-hmax", "--hot-max", "Max blob temperature.");
   args.AddOption(&bg, "-bg", "--background", "Background temperature.");
   args.Parse();
   if (!args.Good())
   {
      args.PrintUsage(cout);
      return 1;
   }
   args.PrintOptions(cout);

   if (save_every < 1) { save_every = 1; }
   const int num_steps = (int)std::lround(t_final / dt);

   // 2. Mesh (fixed for all samples) + finite element space.
   Mesh mesh(mesh_file, 1, 1);
   const int dim = mesh.Dimension();
   for (int lev = 0; lev < ref_levels; lev++) { mesh.UniformRefinement(); }

   H1_FECollection fe_coll(order, dim);
   FiniteElementSpace fespace(&mesh, &fe_coll);
   const int N = fespace.GetNDofs(); // scalar field, vdim == 1
   cout << "Number of nodes (dofs): " << N << endl;

   // 3. Node coordinates in dof order (projecting coordinate functions keeps
   //    them aligned with the solution vector regardless of dof numbering).
   FunctionCoefficient xcoef([](const Vector &p) -> real_t { return p[0]; });
   FunctionCoefficient ycoef([](const Vector &p) -> real_t
   {
      return p.Size() > 1 ? p[1] : real_t(0.0);
   });
   GridFunction node_x(&fespace), node_y(&fespace);
   node_x.ProjectCoefficient(xcoef);
   node_y.ProjectCoefficient(ycoef);

   // 4. Edges in dof order. For order-1 H1 the element dofs are the element
   //    vertices in cyclic boundary order, so connecting consecutive dofs
   //    yields exactly the mesh edges (no spurious quad diagonals).
   set<pair<int, int>> edge_set;
   Array<int> edofs;
   for (int e = 0; e < mesh.GetNE(); ++e)
   {
      fespace.GetElementDofs(e, edofs);
      const int ne = edofs.Size();
      for (int k = 0; k < ne; ++k)
      {
         int a = edofs[k], b = edofs[(k + 1) % ne];
         if (a == b) { continue; }
         if (a > b) { std::swap(a, b); }
         edge_set.insert({a, b});
      }
   }
   const int M = (int)edge_set.size();
   cout << "Number of edges: " << M << endl;

   // 5. Write the shared graph + metadata.
   make_dir(out_dir);
   const string root = string(out_dir);
   {
      ofstream nf(root + "/graph_nodes.csv");
      nf.precision(9);
      nf << "x,y\n";
      for (int i = 0; i < N; ++i)
      {
         nf << node_x(i) << "," << node_y(i) << "\n";
      }
   }
   {
      ofstream ef(root + "/graph_edges.csv");
      ef << "src,dst\n";
      for (const auto &e : edge_set) { ef << e.first << "," << e.second << "\n"; }
   }

   const int num_frames = 1 + num_steps / save_every;
   {
      ofstream jf(root + "/dataset.json");
      jf.precision(9);
      jf << "{\n"
         << "  \"mesh_file\": \"" << mesh_file << "\",\n"
         << "  \"dim\": " << dim << ",\n"
         << "  \"order\": " << order << ",\n"
         << "  \"ref_levels\": " << ref_levels << ",\n"
         << "  \"num_nodes\": " << N << ",\n"
         << "  \"num_edges\": " << M << ",\n"
         << "  \"num_samples\": " << num_samples << ",\n"
         << "  \"num_frames\": " << num_frames << ",\n"
         << "  \"num_steps\": " << num_steps << ",\n"
         << "  \"dt\": " << dt << ",\n"
         << "  \"save_every\": " << save_every << ",\n"
         << "  \"t_final\": " << t_final << ",\n"
         << "  \"frame_dt\": " << dt * save_every << ",\n"
         << "  \"background\": " << bg << ",\n"
         << "  \"alpha_range\": [" << alpha_min << ", " << alpha_max << "],\n"
         << "  \"kappa_range\": [" << kappa_min << ", " << kappa_max << "],\n"
         << "  \"radius_range\": [" << r_min << ", " << r_max << "],\n"
         << "  \"hot_range\": [" << hot_min << ", " << hot_max << "]\n"
         << "}\n";
   }

   // 6. Sampling bounds for the blob center (slightly inset bounding box).
   Vector bb_min, bb_max;
   mesh.GetBoundingBox(bb_min, bb_max, max(order, 1));
   const real_t mx = 0.15 * (bb_max(0) - bb_min(0));
   const real_t my = (dim > 1) ? 0.15 * (bb_max(1) - bb_min(1)) : 0.0;

   ofstream manifest(root + "/manifest.csv");
   manifest.precision(9);
   manifest << "sample,alpha,kappa,blob_cx,blob_cy,blob_r,hot,bg\n";

   mt19937 rng((unsigned)seed);
   uniform_real_distribution<real_t> U(0.0, 1.0);
   auto unif = [&](real_t lo, real_t hi) { return lo + (hi - lo) * U(rng); };

   // 7. Generate samples.
   GridFunction u_gf(&fespace);
   for (int s = 0; s < num_samples; ++s)
   {
      const real_t alpha = unif(alpha_min, alpha_max);
      const real_t kappa = unif(kappa_min, kappa_max);
      BlobIC ic;
      ic.cx = unif(bb_min(0) + mx, bb_max(0) - mx);
      ic.cy = (dim > 1) ? unif(bb_min(1) + my, bb_max(1) - my) : 0.0;
      ic.r = unif(r_min, r_max);
      ic.hot = unif(hot_min, hot_max);
      ic.bg = bg;

      // Initial condition -> true-dof vector.
      FunctionCoefficient u0([ic](const Vector &x) -> real_t { return ic(x); });
      u_gf.ProjectCoefficient(u0);
      Vector u;
      u_gf.GetTrueDofs(u);

      ConductionOperator oper(fespace, alpha, kappa, u);
      unique_ptr<ODESolver> ode_solver = ODESolver::Select(ode_solver_type);
      ode_solver->Init(oper);

      // Sample directory + trajectory file.
      char dirbuf[32];
      std::snprintf(dirbuf, sizeof(dirbuf), "/sample_%05d", s);
      const string sdir = root + dirbuf;
      make_dir(sdir);
      ofstream ff(sdir + "/frames.csv");
      ff.precision(7);

      auto write_frame = [&](const Vector &uu)
      {
         u_gf.SetFromTrueDofs(uu);
         for (int i = 0; i < N; ++i)
         {
            ff << (i ? "," : "") << u_gf(i);
         }
         ff << "\n";
      };

      // Frame 0 (initial condition), then time integration.
      write_frame(u);
      real_t t = 0.0;
      for (int ti = 1; ti <= num_steps; ++ti)
      {
         real_t dt_step = dt;
         ode_solver->Step(u, t, dt_step);
         oper.SetParameters(u);
         if (ti % save_every == 0) { write_frame(u); }
      }

      manifest << s << "," << alpha << "," << kappa << "," << ic.cx << ","
               << ic.cy << "," << ic.r << "," << ic.hot << "," << ic.bg << "\n";

      if ((s + 1) % 10 == 0 || s + 1 == num_samples)
      {
         cout << "Generated " << (s + 1) << " / " << num_samples
              << " samples" << endl;
      }
   }

   cout << "Dataset written to '" << root << "' (" << num_samples
        << " samples, " << num_frames << " frames each, " << N
        << " nodes, " << M << " edges)." << endl;
   return 0;
}

ConductionOperator::ConductionOperator(FiniteElementSpace &f, real_t al,
                                       real_t kap, const Vector &u)
   : TimeDependentOperator(f.GetTrueVSize(), (real_t) 0.0), fespace(f),
     M(NULL), K(NULL), T(NULL), current_dt(0.0), z(height)
{
   const real_t rel_tol = 1e-8;

   M = new BilinearForm(&fespace);
   M->AddDomainIntegrator(new MassIntegrator());
   M->Assemble();
   M->FormSystemMatrix(ess_tdof_list, Mmat);

   M_solver.iterative_mode = false;
   M_solver.SetRelTol(rel_tol);
   M_solver.SetAbsTol(0.0);
   M_solver.SetMaxIter(30);
   M_solver.SetPrintLevel(0);
   M_solver.SetPreconditioner(M_prec);
   M_solver.SetOperator(Mmat);

   alpha = al;
   kappa = kap;

   T_solver.iterative_mode = false;
   T_solver.SetRelTol(rel_tol);
   T_solver.SetAbsTol(0.0);
   T_solver.SetMaxIter(100);
   T_solver.SetPrintLevel(0);
   T_solver.SetPreconditioner(T_prec);

   SetParameters(u);
}

void ConductionOperator::Mult(const Vector &u, Vector &du_dt) const
{
   Kmat.Mult(u, z);
   z.Neg();
   M_solver.Mult(z, du_dt);
}

void ConductionOperator::ImplicitSolve(const real_t dt,
                                       const Vector &u, Vector &du_dt)
{
   if (!T)
   {
      T = Add(1.0, Mmat, dt, Kmat);
      current_dt = dt;
      T_solver.SetOperator(*T);
   }
   MFEM_VERIFY(dt == current_dt, ""); // SDIRK methods use the same dt
   Kmat.Mult(u, z);
   z.Neg();
   T_solver.Mult(z, du_dt);
}

void ConductionOperator::SetParameters(const Vector &u)
{
   GridFunction u_alpha_gf(&fespace);
   u_alpha_gf.SetFromTrueDofs(u);
   for (int i = 0; i < u_alpha_gf.Size(); i++)
   {
      u_alpha_gf(i) = kappa + alpha * u_alpha_gf(i);
   }

   delete K;
   K = new BilinearForm(&fespace);

   GridFunctionCoefficient u_coeff(&u_alpha_gf);

   K->AddDomainIntegrator(new DiffusionIntegrator(u_coeff));
   K->Assemble();
   K->FormSystemMatrix(ess_tdof_list, Kmat);
   delete T;
   T = NULL; // re-compute T on the next ImplicitSolve
}

ConductionOperator::~ConductionOperator()
{
   delete T;
   delete M;
   delete K;
}
