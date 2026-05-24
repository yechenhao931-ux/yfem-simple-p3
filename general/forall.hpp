// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MFEM_FORALL_HPP
#define MFEM_FORALL_HPP

#include "../config/config.hpp"
#include "backends.hpp"
#include "mem_manager.hpp"
#include "../linalg/dtensor.hpp"

namespace mfem
{

namespace internal
{

struct DofQuadLimits_CPU
{
#ifndef _WIN32
   static constexpr int MAX_D1D = 24;
   static constexpr int MAX_Q1D = 24;
#else
   static constexpr int MAX_D1D = 14;
   static constexpr int MAX_Q1D = 14;
#endif
   static constexpr int MAX_T1D = 32;
   static constexpr int HCURL_MAX_D1D = 10;
   static constexpr int HCURL_MAX_Q1D = 10;
   static constexpr int HDIV_MAX_D1D = 10;
   static constexpr int HDIV_MAX_Q1D = 10;
   static constexpr int MAX_INTERP_1D = MAX_D1D;
   static constexpr int MAX_DET_1D = MAX_D1D;
};

} // namespace internal

using DofQuadLimits = internal::DofQuadLimits_CPU;

struct DeviceDofQuadLimits
{
   int MAX_D1D;
   int MAX_Q1D;
   int HCURL_MAX_D1D;
   int HCURL_MAX_Q1D;
   int HDIV_MAX_D1D;
   int HDIV_MAX_Q1D;
   int MAX_INTERP_1D;
   int MAX_DET_1D;

   static const DeviceDofQuadLimits &Get()
   {
      static const DeviceDofQuadLimits dof_quad_limits;
      return dof_quad_limits;
   }

private:
   DeviceDofQuadLimits()
      : MAX_D1D(DofQuadLimits::MAX_D1D),
        MAX_Q1D(DofQuadLimits::MAX_Q1D),
        HCURL_MAX_D1D(DofQuadLimits::HCURL_MAX_D1D),
        HCURL_MAX_Q1D(DofQuadLimits::HCURL_MAX_Q1D),
        HDIV_MAX_D1D(DofQuadLimits::HDIV_MAX_D1D),
        HDIV_MAX_Q1D(DofQuadLimits::HDIV_MAX_Q1D),
        MAX_INTERP_1D(DofQuadLimits::MAX_INTERP_1D),
        MAX_DET_1D(DofQuadLimits::MAX_DET_1D)
   { }
};

#define MFEM_PRAGMA(X) _Pragma(#X)
#define MFEM_UNROLL(N)

template <const int DIM, typename d_lambda, typename h_lambda>
inline void ForallWrap(const bool use_dev, const int N,
                       d_lambda &&d_body, h_lambda &&h_body,
                       const int X=0, const int Y=0, const int Z=0,
                       const int G=0)
{
   (void)DIM;
   (void)use_dev;
   (void)d_body;
   (void)X;
   (void)Y;
   (void)Z;
   (void)G;
   for (int k = 0; k < N; k++) { h_body(k); }
}

template <const int DIM, typename lambda>
inline void ForallWrap(const bool use_dev, const int N, lambda &&body,
                       const int X=0, const int Y=0, const int Z=0,
                       const int G=0)
{
   ForallWrap<DIM>(use_dev, N, body, body, X, Y, Z, G);
}

#define MFEM_FORALL(i,N,...) \
   mfem::ForallWrap<1>(true,N,[=] MFEM_HOST_DEVICE (int i) {__VA_ARGS__;})

#define MFEM_FORALL_2D(i,N,X,Y,BZ,...) \
   mfem::ForallWrap<2>(true,N,[=] MFEM_HOST_DEVICE (int i) {__VA_ARGS__;},X,Y,BZ)

#define MFEM_FORALL_3D(i,N,X,Y,Z,...) \
   mfem::ForallWrap<3>(true,N,[=] MFEM_HOST_DEVICE (int i) {__VA_ARGS__;},X,Y,Z)

#define MFEM_FORALL_3D_GRID(i,N,X,Y,Z,G,...) \
   mfem::ForallWrap<3>(true,N,[=] MFEM_HOST_DEVICE (int i) {__VA_ARGS__;},X,Y,Z,G)

#define MFEM_FORALL_SWITCH(use_dev,i,N,...) \
   mfem::ForallWrap<1>(use_dev,N,[=] MFEM_HOST_DEVICE (int i) {__VA_ARGS__;})

#define MFEM_GPU_FORALL(i,N,...) do { } while (false)

template<typename lambda>
inline void forall(int N, lambda &&body)
{
   for (int i = 0; i < N; i++) { body(i); }
}

template<typename lambda>
inline void forall(int Nx, int Ny, lambda &&body)
{
   for (int j = 0; j < Ny; ++j)
   {
      for (int i = 0; i < Nx; ++i)
      {
         body(i, j);
      }
   }
}

template<typename lambda>
inline void forall(int Nx, int Ny, int Nz, lambda &&body)
{
   for (int k = 0; k < Nz; ++k)
   {
      for (int j = 0; j < Ny; ++j)
      {
         for (int i = 0; i < Nx; ++i)
         {
            body(i, j, k);
         }
      }
   }
}

template<typename lambda>
inline void forall_switch(bool use_dev, int N, lambda &&body)
{
   (void)use_dev;
   forall(N, body);
}

template<typename lambda>
inline void forall_2D(int N, int X, int Y, lambda &&body)
{
   ForallWrap<2>(true, N, body, X, Y, 1);
}

template<typename lambda>
inline void forall_2D_batch(int N, int X, int Y, int BZ, lambda &&body)
{
   ForallWrap<2>(true, N, body, X, Y, BZ);
}

template<typename lambda>
inline void forall_3D(int N, int X, int Y, int Z, lambda &&body)
{
   ForallWrap<3>(true, N, body, X, Y, Z, 0);
}

template<typename lambda>
inline void forall_3D_grid(int N, int X, int Y, int Z, int G, lambda &&body)
{
   ForallWrap<3>(true, N, body, X, Y, Z, G);
}

template<typename lambda>
inline void hypre_forall_cpu(int N, lambda &&body)
{
   forall(N, body);
}

template<typename lambda>
inline void hypre_forall_gpu(int N, lambda &&body)
{
   forall(N, body);
}

template<typename lambda>
inline void hypre_forall(int N, lambda &&body)
{
   forall(N, body);
}

inline MemoryClass GetHypreForallMemoryClass()
{
   return MemoryClass::HOST;
}

} // namespace mfem

#endif // MFEM_FORALL_HPP
