// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.

#include "mesh_headers.hpp"
#include "ncnurbs.hpp"
#include "../fem/fem.hpp"
#include "../general/text.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

namespace mfem
{

bool Mesh::remove_unused_vertices = true;

void Mesh::ReadMFEMMesh(std::istream &input, int version, int &curved)
{
   MFEM_VERIFY(version == 10 || version == 12 || version == 13,
               "unknown MFEM mesh version");

   string ident;

   skip_comment_lines(input, '#');
   input >> ident;

   MFEM_VERIFY(ident == "dimension", "invalid mesh file");
   input >> Dim;

   skip_comment_lines(input, '#');
   input >> ident;

   MFEM_VERIFY(ident == "elements", "invalid mesh file");
   input >> NumOfElements;
   elements.SetSize(NumOfElements);
   for (int j = 0; j < NumOfElements; j++)
   {
      elements[j] = ReadElement(input);
   }

   if (version == 13)
   {
      skip_comment_lines(input, '#');
      input >> ident;

      MFEM_VERIFY(ident == "attribute_sets", "invalid mesh file");

      attribute_sets.attr_sets.Load(input);
      attribute_sets.attr_sets.SortAll();
      attribute_sets.attr_sets.UniqueAll();
   }

   skip_comment_lines(input, '#');
   input >> ident;

   MFEM_VERIFY(ident == "boundary", "invalid mesh file");
   input >> NumOfBdrElements;
   boundary.SetSize(NumOfBdrElements);
   for (int j = 0; j < NumOfBdrElements; j++)
   {
      boundary[j] = ReadElement(input);
   }

   if (version == 13)
   {
      skip_comment_lines(input, '#');
      input >> ident;

      MFEM_VERIFY(ident == "bdr_attribute_sets", "invalid mesh file");

      bdr_attribute_sets.attr_sets.Load(input);
      bdr_attribute_sets.attr_sets.SortAll();
      bdr_attribute_sets.attr_sets.UniqueAll();
   }

   skip_comment_lines(input, '#');
   input >> ident;

   MFEM_VERIFY(ident == "vertices", "invalid mesh file");
   input >> NumOfVertices;
   vertices.SetSize(NumOfVertices);

   input >> ws >> ident;
   if (ident != "nodes")
   {
      spaceDim = atoi(ident.c_str());
      for (int j = 0; j < NumOfVertices; j++)
      {
         for (int i = 0; i < spaceDim; i++)
         {
            input >> vertices[j](i);
         }
      }
   }
   else
   {
      input >> ws;
      curved = 1;
   }

   if (remove_unused_vertices) { RemoveUnusedVertices(); }
}

void Mesh::ReadLineMesh(std::istream &input)
{
   int j, p1, p2, a;

   Dim = 1;

   input >> NumOfVertices;
   vertices.SetSize(NumOfVertices);
   for (j = 0; j < NumOfVertices; j++)
   {
      input >> vertices[j](0);
   }

   input >> NumOfElements;
   elements.SetSize(NumOfElements);
   for (j = 0; j < NumOfElements; j++)
   {
      input >> a >> p1 >> p2;
      elements[j] = new Segment(p1-1, p2-1, a);
   }

   int ind[1];
   input >> NumOfBdrElements;
   boundary.SetSize(NumOfBdrElements);
   for (j = 0; j < NumOfBdrElements; j++)
   {
      input >> a >> ind[0];
      ind[0]--;
      boundary[j] = new Point(ind, a);
   }
}

void Mesh::ReadNetgen2DMesh(std::istream &input, int &curved)
{
   int ints[32], attr, n;

   Dim = 2;

   input >> NumOfBdrElements;
   boundary.SetSize(NumOfBdrElements);
   for (int i = 0; i < NumOfBdrElements; i++)
   {
      input >> attr >> ints[0] >> ints[1];
      ints[0]--;
      ints[1]--;
      boundary[i] = new Segment(ints, attr);
   }

   input >> NumOfElements;
   elements.SetSize(NumOfElements);
   for (int i = 0; i < NumOfElements; i++)
   {
      input >> attr >> n;
      for (int j = 0; j < n; j++)
      {
         input >> ints[j];
         ints[j]--;
      }
      switch (n)
      {
         case 2:
            elements[i] = new Segment(ints, attr);
            break;
         case 3:
            elements[i] = new Triangle(ints, attr);
            break;
         case 4:
            elements[i] = new Quadrilateral(ints, attr);
            break;
         default:
            MFEM_ABORT("unsupported Netgen 2D element");
      }
   }

   if (!curved)
   {
      input >> NumOfVertices;
      vertices.SetSize(NumOfVertices);
      for (int i = 0; i < NumOfVertices; i++)
      {
         for (int j = 0; j < Dim; j++)
         {
            input >> vertices[i](j);
         }
      }
   }
   else
   {
      input >> NumOfVertices;
      vertices.SetSize(NumOfVertices);
      input >> ws;
   }
}

void Mesh::ReadNetgen3DMesh(std::istream &input)
{
   int ints[32], attr;

   Dim = 3;

   input >> NumOfVertices;
   vertices.SetSize(NumOfVertices);
   for (int i = 0; i < NumOfVertices; i++)
   {
      for (int j = 0; j < Dim; j++)
      {
         input >> vertices[i](j);
      }
   }

   input >> NumOfElements;
   elements.SetSize(NumOfElements);
   for (int i = 0; i < NumOfElements; i++)
   {
      input >> attr;
      for (int j = 0; j < 4; j++)
      {
         input >> ints[j];
         ints[j]--;
      }
#ifdef MFEM_USE_MEMALLOC
      Tetrahedron *tet = TetMemory.Alloc();
      tet->SetVertices(ints);
      tet->SetAttribute(attr);
      elements[i] = tet;
#else
      elements[i] = new Tetrahedron(ints, attr);
#endif
   }

   input >> NumOfBdrElements;
   boundary.SetSize(NumOfBdrElements);
   for (int i = 0; i < NumOfBdrElements; i++)
   {
      input >> attr;
      for (int j = 0; j < 3; j++)
      {
         input >> ints[j];
         ints[j]--;
      }
      boundary[i] = new Triangle(ints, attr);
   }
}

void Mesh::ReadTrueGridMesh(std::istream &input)
{
   int i, j, ints[32], attr;
   const int buflen = 1024;
   char buf[buflen];

   Dim = 3;

   if (Dim == 3)
   {
      int vari;
      real_t varf;
      input >> vari >> NumOfVertices >> NumOfElements;
      input.getline(buf, buflen);
      input.getline(buf, buflen);
      input >> vari >> vari >> NumOfBdrElements;
      input.getline(buf, buflen);
      input.getline(buf, buflen);
      input.getline(buf, buflen);

      vertices.SetSize(NumOfVertices);
      for (i = 0; i < NumOfVertices; i++)
      {
         input >> vari >> varf >> vertices[i](0) >> vertices[i](1)
               >> vertices[i](2);
         input.getline(buf, buflen);
      }

      elements.SetSize(NumOfElements);
      for (i = 0; i < NumOfElements; i++)
      {
         input >> vari >> attr;
         for (j = 0; j < 8; j++)
         {
            input >> ints[j];
            ints[j]--;
         }
         input.getline(buf, buflen);
         elements[i] = new Hexahedron(ints, attr);
      }

      boundary.SetSize(NumOfBdrElements);
      for (i = 0; i < NumOfBdrElements; i++)
      {
         input >> attr;
         for (j = 0; j < 4; j++)
         {
            input >> ints[j];
            ints[j]--;
         }
         input.getline(buf, buflen);
         boundary[i] = new Quadrilateral(ints, attr);
      }
   }
}

const int Mesh::vtk_quadratic_tet[10] =
{ 0, 1, 2, 3, 4, 7, 5, 6, 8, 9 };

const int Mesh::vtk_quadratic_pyramid[13] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

const int Mesh::vtk_quadratic_wedge[18] =
{ 0, 2, 1, 3, 5, 4, 8, 7, 6, 11, 10, 9, 12, 14, 13, 17, 16, 15 };

const int Mesh::vtk_quadratic_hex[27] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
   24, 22, 21, 23, 20, 25, 26
};

void Mesh::CreateVTKMesh(const Vector &, const Array<int> &,
                         const Array<int> &, const Array<int> &,
                         const Array<int> &, int &, int &, bool &)
{
   MFEM_ABORT("VTK mesh input is not included in this serial extract.");
}

void Mesh::ReadXML_VTKMesh(std::istream &, int &, int &, bool &,
                           const std::string &)
{
   MFEM_ABORT("XML VTK mesh input is not included in this serial extract.");
}

void Mesh::ReadVTKMesh(std::istream &, int &, int &, bool &)
{
   MFEM_ABORT("VTK mesh input is not included in this serial extract.");
}

void Mesh::ReadNURBSMesh(std::istream &input, int &curved, int &read_gf,
                         bool spacing, bool nc)
{
   NURBSext = nc ? new NCNURBSExtension(input, spacing):
              new NURBSExtension(input, spacing);

   Dim              = NURBSext->Dimension();
   NumOfVertices    = NURBSext->GetNV();
   NumOfElements    = NURBSext->GetNE();
   NumOfBdrElements = NURBSext->GetNBE();

   NURBSext->GetElementTopo(elements);
   NURBSext->GetBdrElementTopo(boundary);

   vertices.SetSize(NumOfVertices);
   curved = 1;
   if (NURBSext->HavePatches())
   {
      NURBSFECollection  *fec = new NURBSFECollection(NURBSext->GetOrder());
      FiniteElementSpace *fes = new FiniteElementSpace(this, fec, Dim,
                                                       Ordering::byVDIM);
      Nodes = new GridFunction(fes);
      Nodes->MakeOwner(fec);
      NURBSext->SetCoordsFromPatches(*Nodes);
      own_nodes = 1;
      read_gf = 0;
      spaceDim = Nodes->VectorDim();
      for (int i = 0; i < spaceDim; i++)
      {
         Vector vert_val;
         Nodes->GetNodalValues(vert_val, i+1);
         for (int j = 0; j < NumOfVertices; j++)
         {
            vertices[j](i) = vert_val(j);
         }
      }
   }
   else
   {
      read_gf = 1;
   }
}

void Mesh::ReadInlineMesh(std::istream &input, bool generate_edges)
{
   int nx = -1;
   int ny = -1;
   int nz = -1;
   real_t sx = -1.0;
   real_t sy = -1.0;
   real_t sz = -1.0;
   Element::Type type = Element::POINT;

   while (true)
   {
      skip_comment_lines(input, '#');
      if (!input.good())
      {
         break;
      }

      std::string name;
      input >> name;
      input >> std::ws;
      MFEM_VERIFY(input.get() == '=',
                  "Inline mesh expected '=' after keyword " << name);
      input >> std::ws;

      if (name == "nx")
      {
         input >> nx;
      }
      else if (name == "ny")
      {
         input >> ny;
      }
      else if (name == "nz")
      {
         input >> nz;
      }
      else if (name == "sx")
      {
         input >> sx;
      }
      else if (name == "sy")
      {
         input >> sy;
      }
      else if (name == "sz")
      {
         input >> sz;
      }
      else if (name == "type")
      {
         std::string eltype;
         input >> eltype;
         if (eltype == "segment")
         {
            type = Element::SEGMENT;
         }
         else if (eltype == "quad")
         {
            type = Element::QUADRILATERAL;
         }
         else if (eltype == "tri")
         {
            type = Element::TRIANGLE;
         }
         else if (eltype == "hex")
         {
            type = Element::HEXAHEDRON;
         }
         else if (eltype == "wedge")
         {
            type = Element::WEDGE;
         }
         else if (eltype == "pyramid")
         {
            type = Element::PYRAMID;
         }
         else if (eltype == "tet")
         {
            type = Element::TETRAHEDRON;
         }
         else
         {
            MFEM_ABORT("unrecognized inline mesh element type: " << eltype);
         }
      }
      else
      {
         MFEM_ABORT("unrecognized inline mesh keyword: " << name);
      }

      input >> std::ws;
      if (input.peek() == ';')
      {
         input.get();
      }

      if (!input)
      {
         break;
      }
   }

   if (type == Element::SEGMENT)
   {
      MFEM_VERIFY(nx > 0 && sx > 0.0, "invalid 1D inline mesh format");
      Make1D(nx, sx);
   }
   else if (type == Element::TRIANGLE || type == Element::QUADRILATERAL)
   {
      MFEM_VERIFY(nx > 0 && ny > 0 && sx > 0.0 && sy > 0.0,
                  "invalid 2D inline mesh format");
      Make2D(nx, ny, type, sx, sy, generate_edges, true);
   }
   else if (type == Element::TETRAHEDRON || type == Element::WEDGE ||
            type == Element::HEXAHEDRON  || type == Element::PYRAMID)
   {
      MFEM_VERIFY(nx > 0 && ny > 0 && nz > 0 &&
                  sx > 0.0 && sy > 0.0 && sz > 0.0,
                  "invalid 3D inline mesh format");
      Make3D(nx, ny, nz, type, sx, sy, sz, true);
   }
   else
   {
      MFEM_ABORT("inline mesh must specify an element type");
   }
}

void Mesh::ReadGmshMesh(std::istream &, int &, int &)
{
   MFEM_ABORT("Gmsh mesh input is not included in this serial extract.");
}

} // namespace mfem
