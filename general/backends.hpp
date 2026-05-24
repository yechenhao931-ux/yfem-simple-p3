// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MFEM_BACKENDS_HPP
#define MFEM_BACKENDS_HPP

#include "../config/config.hpp"

#define MFEM_DEVICE
#define MFEM_HOST
#define MFEM_LAMBDA
#define MFEM_DEVICE_SYNC
#define MFEM_STREAM_SYNC
#define MFEM_SHARED
#define MFEM_SYNC_THREAD
#define MFEM_BLOCK_ID(k) 0
#define MFEM_THREAD_ID(k) 0
#define MFEM_THREAD_SIZE(k) 1
#define MFEM_FOREACH_THREAD(i,k,N) for (int i = 0; i < (N); i++)
#define MFEM_FOREACH_THREAD_DIRECT(i,k,N) MFEM_FOREACH_THREAD(i,k,N)

template <typename T>
MFEM_HOST_DEVICE T AtomicAdd(T &add, const T val)
{
   T old = add;
   add += val;
   return old;
}

#endif
