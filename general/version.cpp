// Copyright (c) 2010-2025, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#include "../config/config.hpp"
#include "version.hpp"

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

namespace mfem
{


int GetVersion()
{
   return MFEM_VERSION;
}


int GetVersionMajor()
{
   return MFEM_VERSION_MAJOR;
}


int GetVersionMinor()
{
   return MFEM_VERSION_MINOR;
}


int GetVersionPatch()
{
   return MFEM_VERSION_PATCH;
}


const char *GetVersionStr()
{
#if MFEM_VERSION_TYPE == MFEM_VERSION_TYPE_RELEASE
#define MFEM_VERSION_TYPE_STR " (release)"
#elif MFEM_VERSION_TYPE == MFEM_VERSION_TYPE_DEVELOPMENT
#define MFEM_VERSION_TYPE_STR " (development)"
#endif
   static const char *version_str =
      "MFEM v" MFEM_VERSION_STRING MFEM_VERSION_TYPE_STR;
   return version_str;
}


const char *GetGitStr()
{
   static const char *git_str = MFEM_GIT_STRING;
   return git_str;
}


const char *GetConfigStr()
{
   static const char *config_str =
      ""
#ifdef MFEM_USE_DOUBLE
      "MFEM_USE_DOUBLE\n"
#endif
#ifdef MFEM_USE_EXCEPTIONS
      "MFEM_USE_EXCEPTIONS\n"
#endif
#ifdef MFEM_USE_MEMALLOC
      "MFEM_USE_MEMALLOC\n"
#endif
#ifdef MFEM_USE_SINGLE
      "MFEM_USE_SINGLE\n"
#endif
      "MFEM_TIMER_TYPE = " EXPAND_AND_QUOTE(MFEM_TIMER_TYPE)
      ;

   return config_str;
}

}
