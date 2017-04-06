/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief API for getting and comparing version numbers
 */


 #define STRINGIFY(x) #x
 #define EFU_STR(x) STRINGIFY(x)
 //#define EFU_STR(x) #X

#include <cassert>
#include <common/version_num.h>
#include <cstdio>

static_assert(EFU_VER_MAJ >= 0, "version number error");
static_assert(EFU_VER_MIN >= 0, "version number error");
static_assert(EFU_VER_BUILD >= 0, "version number error");
static_assert(EFU_VER_MAJ < 1024, "version number error");
static_assert(EFU_VER_MIN < 1024, "version number error");
static_assert(EFU_VER_BUILD < 4096, "version number error");

#define EFU_VERSION_NUM(maj,min,build) ((maj) << 22 | (min) << 12 | (build))

#define EFU_VERSION EFU_VERSION_NUM(EFU_VER_MAJ, EFU_VER_MIN, EFU_VER_BUILD)

static inline const char * efu_version() {
  static char version[32];
  if (version[0] != 0)
    return version;

  snprintf(version, sizeof(version), "%d.%d.%d",
            EFU_VER_MAJ,
            EFU_VER_MIN,
            EFU_VER_BUILD);
  return version;
}

static inline const char * efu_buildstr() {
  return EFU_STR(BUILDSTR);
}
