// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MFEM_DEVICE_HPP
#define MFEM_DEVICE_HPP

#include "enzyme.hpp"
#include "globals.hpp"
#include "mem_manager.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace mfem
{

struct Backend
{
   enum Id: unsigned long
   {
      CPU = 1 << 0
   };

   enum
   {
      NUM_BACKENDS = 1,
      CPU_MASK = CPU,
      OMP_MASK = 0,
      DEVICE_MASK = 0
   };
};

class Device
{
private:
   friend class MemoryManager;

   static bool device_env, mem_host_env, mem_device_env, mem_types_set;
   MFEM_ENZYME_INACTIVE static MFEM_EXPORT Device device_singleton;

   int dev = 0;
   int ngpu = -1;
   unsigned long backends = Backend::CPU;
   bool destroy_mm = false;
   bool mpi_gpu_aware = false;
   MemoryType host_mem_type = MemoryType::HOST;
   MemoryClass host_mem_class = MemoryClass::HOST;
   MemoryType device_mem_type = MemoryType::HOST;
   MemoryClass device_mem_class = MemoryClass::HOST;

   Device(const Device &) = delete;
   Device &operator=(const Device &) = delete;

   static Device &Get() { return device_singleton; }

public:
   Device();
   Device(const std::string &device, const int device_id = 0)
   { Configure(device, device_id); }
   ~Device();

   void Configure(const std::string &device, const int device_id = 0);
   static void SetMemoryTypes(MemoryType h_mt, MemoryType d_mt);
   void Print(std::ostream &os = mfem::out);

   static inline bool IsConfigured() { return Get().ngpu >= 0; }
   static inline bool IsAvailable() { return false; }
   static inline bool IsEnabled() { return false; }
   static inline bool IsDisabled() { return true; }
   static inline int GetId() { return Get().dev; }
   static int GetDeviceCount();
   static inline bool Allows(unsigned long b_mask)
   { return Get().backends & b_mask; }

   static inline MemoryType GetHostMemoryType() { return Get().host_mem_type; }
   static inline MemoryClass GetHostMemoryClass() { return Get().host_mem_class; }
   static inline MemoryType GetDeviceMemoryType() { return Get().device_mem_type; }
   static inline MemoryType GetMemoryType() { return Get().device_mem_type; }
   static inline MemoryClass GetDeviceMemoryClass() { return Get().device_mem_class; }
   static inline MemoryClass GetMemoryClass() { return Get().device_mem_class; }

   static void SetGPUAwareMPI(const bool force = true)
   { Get().mpi_gpu_aware = force; }
   static bool GetGPUAwareMPI() { return Get().mpi_gpu_aware; }

   static MemoryType QueryMemoryType(const void *ptr);
   static int NumMultiprocessors(int device_id);
   static int NumMultiprocessors();
   static int WarpSize(int device_id);
   static int WarpSize();
   static void DeviceMem(size_t *free, size_t *total);
};

template <typename T>
inline MemoryClass GetMemoryClass(const Memory<T> &mem, bool on_dev)
{
   if (on_dev) { mem.UseDevice(true); }
   return MemoryClass::HOST;
}

template <typename T>
inline const T *Read(const Memory<T> &mem, int size, bool on_dev = true)
{
   return mem.Read(GetMemoryClass(mem, on_dev), size);
}

template <typename T>
inline const T *HostRead(const Memory<T> &mem, int size)
{
   return mfem::Read(mem, size, false);
}

template <typename T>
inline T *Write(Memory<T> &mem, int size, bool on_dev = true)
{
   return mem.Write(GetMemoryClass(mem, on_dev), size);
}

template <typename T>
inline T *HostWrite(Memory<T> &mem, int size)
{
   return mfem::Write(mem, size, false);
}

template <typename T>
inline T *ReadWrite(Memory<T> &mem, int size, bool on_dev = true)
{
   return mem.ReadWrite(GetMemoryClass(mem, on_dev), size);
}

template <typename T>
inline T *HostReadWrite(Memory<T> &mem, int size)
{
   return mfem::ReadWrite(mem, size, false);
}

template <typename T>
inline void Read(Memory<T> &dst, const Memory<T> &src, int size,
                 bool on_dev = true)
{
   dst.CopyFrom(src, size);
}

} // namespace mfem

#endif
