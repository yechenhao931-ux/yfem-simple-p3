// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MFEM_SCAN_HPP
#define MFEM_SCAN_HPP

#include <algorithm>
#include <cstddef>
#include <functional>

namespace mfem
{

template <class InputIt, class OutputIt>
void InclusiveScan(bool, InputIt d_in, OutputIt d_out, size_t num_items)
{
   if (!num_items) { return; }
   *d_out = *d_in;
   auto prev = d_out;
   ++d_in;
   ++d_out;
   for (size_t i = 1; i < num_items; ++i)
   {
      *d_out = (*prev) + (*d_in);
      prev = d_out;
      ++d_in;
      ++d_out;
   }
}

template <class InputIt, class OutputIt, class ScanOp>
void InclusiveScan(bool, InputIt d_in, OutputIt d_out, size_t num_items,
                   ScanOp scan_op)
{
   if (!num_items) { return; }
   *d_out = *d_in;
   auto prev = d_out;
   ++d_in;
   ++d_out;
   for (size_t i = 1; i < num_items; ++i)
   {
      *d_out = scan_op(*prev, *d_in);
      prev = d_out;
      ++d_in;
      ++d_out;
   }
}

template <class InputIt, class OutputIt, class T, class ScanOp>
void ExclusiveScan(bool, InputIt d_in, OutputIt d_out, size_t num_items,
                   T init_value, ScanOp scan_op)
{
   for (size_t i = 0; i < num_items; ++i)
   {
      auto next = scan_op(init_value, *d_in);
      *d_out = init_value;
      init_value = next;
      ++d_out;
      ++d_in;
   }
}

template <class InputIt, class OutputIt, class T>
void ExclusiveScan(bool use_dev, InputIt d_in, OutputIt d_out, size_t num_items,
                   T init_value)
{
   ExclusiveScan(use_dev, d_in, d_out, num_items, init_value, std::plus<> {});
}

template <class InputIt, class FlagIt, class OutputIt, class NumSelectedIt>
void CopyFlagged(bool, InputIt d_in, FlagIt d_flags, OutputIt d_out,
                 NumSelectedIt d_num_selected_out, size_t num_items)
{
   *d_num_selected_out = 0;
   for (size_t i = 0; i < num_items; ++i, ++d_in, ++d_flags)
   {
      if (*d_flags)
      {
         *d_out = *d_in;
         ++d_out;
         ++*d_num_selected_out;
      }
   }
}

template <class InputIt, class OutputIt, class NumSelectedIt, class SelectOp>
void CopyIf(bool, InputIt d_in, OutputIt d_out,
            NumSelectedIt d_num_selected_out, size_t num_items,
            SelectOp select_op)
{
   *d_num_selected_out = 0;
   for (size_t i = 0; i < num_items; ++i, ++d_in)
   {
      if (select_op(*d_in))
      {
         *d_out = *d_in;
         ++d_out;
         ++*d_num_selected_out;
      }
   }
}

template <class InputIt, class OutputIt, class NumSelectedIt>
void CopyUnique(bool, InputIt d_in, OutputIt d_out,
                NumSelectedIt d_num_selected_out, size_t num_items)
{
   *d_num_selected_out =
      std::unique_copy(d_in, d_in + num_items, d_out) - d_out;
}

} // namespace mfem

#endif
