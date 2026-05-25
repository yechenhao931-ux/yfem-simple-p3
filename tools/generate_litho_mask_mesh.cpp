#include "mfem.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

using namespace mfem;

namespace
{
bool InRect(real_t x, real_t y,
            real_t x0, real_t x1, real_t y0, real_t y1)
{
   return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

bool InCircle(real_t x, real_t y, real_t cx, real_t cy, real_t r)
{
   const real_t dx = x - cx;
   const real_t dy = y - cy;
   return dx*dx + dy*dy <= r*r;
}

bool InRing(real_t x, real_t y, real_t cx, real_t cy,
            real_t r0, real_t r1)
{
   const real_t dx = x - cx;
   const real_t dy = y - cy;
   const real_t r2 = dx*dx + dy*dy;
   return r2 >= r0*r0 && r2 <= r1*r1;
}

bool LithoMaskPattern(real_t x, real_t y)
{
   bool mask = false;

   // Long traces.
   mask = mask || InRect(x, y, 0.40, 4.60, 0.55, 0.78);
   mask = mask || InRect(x, y, 0.40, 4.60, 4.22, 4.45);
   mask = mask || InRect(x, y, 0.55, 0.78, 0.60, 4.40);
   mask = mask || InRect(x, y, 4.22, 4.45, 0.60, 4.40);

   // Central cross and pads.
   mask = mask || InRect(x, y, 2.35, 2.65, 0.95, 4.05);
   mask = mask || InRect(x, y, 0.95, 4.05, 2.35, 2.65);
   mask = mask || InRect(x, y, 1.05, 1.65, 1.05, 1.65);
   mask = mask || InRect(x, y, 3.35, 3.95, 1.05, 1.65);
   mask = mask || InRect(x, y, 1.05, 1.65, 3.35, 3.95);
   mask = mask || InRect(x, y, 3.35, 3.95, 3.35, 3.95);

   // Ring-like and via-like features.
   mask = mask || InRing(x, y, 2.50, 2.50, 0.78, 1.05);
   mask = mask || InCircle(x, y, 1.15, 2.50, 0.18);
   mask = mask || InCircle(x, y, 3.85, 2.50, 0.18);
   mask = mask || InCircle(x, y, 2.50, 1.15, 0.18);
   mask = mask || InCircle(x, y, 2.50, 3.85, 0.18);

   // Small staggered slots.
   for (int i = 0; i < 6; i++)
   {
      const real_t x0 = 0.95 + 0.52*i;
      const real_t y0 = (i % 2 == 0) ? 1.82 : 2.92;
      mask = mask || InRect(x, y, x0, x0 + 0.28, y0, y0 + 0.16);
   }

   return mask;
}
}

int main()
{
   const int nx = 48;
   const int ny = 48;
   const int nz = 4;
   const real_t sx = 5.0;
   const real_t sy = 5.0;
   const real_t sz = 1.0;
   const real_t base_height = 0.25;

   Mesh mesh(3, 0, 0, 0, 3);
   std::vector<int> vertex_id((nx + 1)*(ny + 1)*(nz + 1), -1);

   auto grid_index = [nx, ny](int i, int j, int k)
   {
      return i + (nx + 1)*(j + (ny + 1)*k);
   };

   auto get_vertex = [&](int i, int j, int k)
   {
      const int idx = grid_index(i, j, k);
      if (vertex_id[idx] < 0)
      {
         vertex_id[idx] = mesh.AddVertex(sx*i/nx, sy*j/ny, sz*k/nz);
      }
      return vertex_id[idx];
   };

   for (int k = 0; k < nz; k++)
   {
      for (int j = 0; j < ny; j++)
      {
         for (int i = 0; i < nx; i++)
         {
            const real_t xc = sx*(i + 0.5)/nx;
            const real_t yc = sy*(j + 0.5)/ny;
            const real_t zc = sz*(k + 0.5)/nz;
            const bool base = zc <= base_height;
            const bool pattern = zc > base_height && LithoMaskPattern(xc, yc);

            if (!base && !pattern) { continue; }

            int vi[8];
            vi[0] = get_vertex(i,     j,     k);
            vi[1] = get_vertex(i + 1, j,     k);
            vi[2] = get_vertex(i + 1, j + 1, k);
            vi[3] = get_vertex(i,     j + 1, k);
            vi[4] = get_vertex(i,     j,     k + 1);
            vi[5] = get_vertex(i + 1, j,     k + 1);
            vi[6] = get_vertex(i + 1, j + 1, k + 1);
            vi[7] = get_vertex(i,     j + 1, k + 1);

            mesh.AddHexAsTets(vi, pattern ? 2 : 1);
         }
      }
   }

   mesh.FinalizeTopology(true);
   mesh.Finalize(false, true);
   mesh.SetAttributes();

   std::ofstream out("data/litho_mask_5x5x1.mesh");
   out.precision(16);
   mesh.Print(out, "# Tetrahedral lithography-mask mesh: base plate plus raised pattern\n"
                   "# Overall design box: 5 x 5 x 1\n"
                   "# Element attributes: 1=base plate, 2=raised mask pattern\n");

   std::cout << "Wrote data/litho_mask_5x5x1.mesh\n";
   std::cout << "Tetrahedral elements: " << mesh.GetNE()
             << ", boundary elements: "
             << mesh.GetNBE() << "\n";
   return 0;
}
