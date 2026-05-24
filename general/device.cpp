// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC.
// SPDX-License-Identifier: BSD-3-Clause

#include "device.hpp"
#include "error.hpp"

#include <ostream>

namespace mfem
{

Device Device::device_singleton;
bool Device::device_env = false;
bool Device::mem_host_env = false;
bool Device::mem_device_env = false;
bool Device::mem_types_set = false;

Device::Device()
{
   ngpu = 0;
   backends = Backend::CPU;
   host_mem_type = device_mem_type = MemoryType::HOST;
   host_mem_class = device_mem_class = MemoryClass::HOST;
}

Device::~Device() = default;

void Device::Configure(const std::string &device, const int device_id)
{
   MFEM_VERIFY(device.empty() || device == "cpu",
               "this serial-only MFEM extract supports only '-d cpu'");
   dev = device_id;
   ngpu = 0;
   backends = Backend::CPU;
   host_mem_type = device_mem_type = MemoryType::HOST;
   host_mem_class = device_mem_class = MemoryClass::HOST;
}

void Device::SetMemoryTypes(MemoryType h_mt, MemoryType)
{
   MFEM_VERIFY(IsHostMemory(h_mt), "serial-only memory must be host memory");
   Get().host_mem_type = h_mt;
   Get().device_mem_type = h_mt;
   Get().host_mem_class = MemoryClass::HOST;
   Get().device_mem_class = MemoryClass::HOST;
   mem_types_set = true;
}

void Device::Print(std::ostream &os)
{
   os << "Device configuration: cpu\n";
   os << "Memory configuration: host-std\n";
}

int Device::GetDeviceCount() { return 0; }

MemoryType Device::QueryMemoryType(const void *)
{
   return MemoryType::HOST;
}

int Device::NumMultiprocessors(int) { return 1; }

int Device::NumMultiprocessors() { return 1; }

int Device::WarpSize(int) { return 1; }

int Device::WarpSize() { return 1; }

void Device::DeviceMem(size_t *free, size_t *total)
{
   if (free) { *free = 0; }
   if (total) { *total = 0; }
}

} // namespace mfem
