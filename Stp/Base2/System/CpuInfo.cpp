// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/CpuInfo.h"

#include "Base/Compiler/Config.h"
#include "Base/Compiler/Os.h"
#include "Base/Debug/Log.h"

#include <limits.h>

#if OS(WIN)
#include <windows.h>
#elif OS(DARWIN)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#elif OS(LINUX)
#include <unistd.h>
#endif

#if CPU(X86_FAMILY)
# if COMPILER(MSVC)
#  include <intrin.h>
#  include <immintrin.h>  // For _xgetbv()
# else
#  include <cpuid.h>
# endif
#elif CPU(ARM_FAMILY)
# if CPU(ARM32) && __has_include(<sys/auxv.h>)
#  include <sys/auxv.h>
# elif CPU(ARM32) && __has_include(<cpu-features.h>)
#  include <cpu-features.h>
# endif
#endif

namespace stp {

int CpuInfo::g_number_of_cores_ = 0;
CpuInfo::Features CpuInfo::g_features_ = 0;

static int DetectNumberOfCores() {
  int number_of_cores = 1;

  #if OS(WIN)
  SYSTEM_INFO system_info = {};
  ::GetNativeSystemInfo(&system_info);
  number_of_cores = system_info.dwNumberOfProcessors;
  #elif OS(LINUX) || OS(ANDROID)
  number_of_cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
  #elif OS(DARWIN) || OS(FREEBSD)
  int name[] = { CTL_HW, HW_AVAILCPU };
  size_t size = sizeof(number_of_cores);
  if (0 != sysctl(name, sizeof(name) / sizeof(int), &number_of_cores, &size, NULL, 0)) {
    RELEASE_LOG(ERROR, "failed to get number of cores");
    number_of_cores = 1;
  }
  #else
  # error "not implemented"
  #endif // OS(*)
  return number_of_cores;
}

#if CPU(X86_FAMILY)
#if COMPILER(MSVC)
static void cpuid(uint32_t level, uint32_t abcd[4]) {
  __cpuid  ((int*)abcd, level);
}
static void cpuidex(uint32_t level, uint32_t count, uint32_t abcd[4]) {
  __cpuidex((int*)abcd, level, count);
}
static uint64_t xgetbv(uint32_t xcr) { return _xgetbv(xcr); }
#else

static void cpuid(uint32_t level, uint32_t abcd[4]) {
  __get_cpuid(level, abcd+0, abcd+1, abcd+2, abcd+3);
}
static void cpuidex(uint32_t level, uint32_t count, uint32_t abcd[4]) {
  __cpuid_count(level, count, abcd[0], abcd[1], abcd[2], abcd[3]);
}

// _xgetbv returns the value of an Intel Extended Control Register (XCR).
// Currently only XCR0 is defined by Intel so |xcr| should always be zero.
static uint64_t xgetbv(uint32_t xcr) {
  uint32_t eax, edx;
  __asm__ volatile ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(xcr));
  return (static_cast<uint64_t>(edx) << 32) | eax;
}
#endif // COMPILER(*)
#endif // CPU(X86_FAMILY)

CpuInfo::Features CpuInfo::RuntimeFeatures() {
  Features features = 0;

  auto Add = [&features](CpuFeature feature) {
    features |= static_cast<Features>(feature);
  };
  #if CPU(X86_FAMILY)
  uint32_t abcd[4] = {0,0,0,0};

  cpuid(1, abcd);

  if (abcd[3] & (1<<25)) { Add(CpuFeature::Sse1); }
  if (abcd[3] & (1<<26)) { Add(CpuFeature::Sse2); }
  if (abcd[2] & (1<< 0)) { Add(CpuFeature::Sse3); }
  if (abcd[2] & (1<< 9)) { Add(CpuFeature::Ssse3); }
  if (abcd[2] & (1<<19)) { Add(CpuFeature::Sse41); }
  if (abcd[2] & (1<<20)) { Add(CpuFeature::Sse42); }

  if ((abcd[2] & (3<<26)) == (3<<26)         // XSAVE + OSXSAVE
      && (xgetbv(0) & (3<<1)) == (3<<1)) {  // XMM and YMM state enabled.
    if (abcd[2] & (1<<28)) { Add(CpuFeature::Avx); }
    if (abcd[2] & (1<<29)) { Add(CpuFeature::Cvt16); }
    if (abcd[2] & (1<<12)) { Add(CpuFeature::Fma); }

    cpuidex(7, 0, abcd);
    if (abcd[1] & (1<<5)) { Add(CpuFeature::Avx2); }
    if (abcd[1] & (1<<3)) { Add(CpuFeature::Bmi1); }
    if (abcd[1] & (1<<8)) { Add(CpuFeature::Bmi2); }

    if ((xgetbv(0) & (7<<5)) == (7<<5)) {  // All ZMM state bits enabled too.
      if (abcd[1] & (1<<16)) { Add(CpuFeature::Avx512F); }
      if (abcd[1] & (1<<17)) { Add(CpuFeature::Avx512DQ); }
      if (abcd[1] & (1<<21)) { Add(CpuFeature::Avx512IFMA); }
      if (abcd[1] & (1<<26)) { Add(CpuFeature::Avx512PF); }
      if (abcd[1] & (1<<27)) { Add(CpuFeature::Avx512ER); }
      if (abcd[1] & (1<<28)) { Add(CpuFeature::Avx512CD); }
      if (abcd[1] & (1<<30)) { Add(CpuFeature::Avx512BW); }
      if (abcd[1] & (1<<31)) { Add(CpuFeature::Avx512VL); }
    }
  }

  // Query extended IDs.
  cpuid(0x80000000, abcd);
  uint32_t num_ext_ids = abcd[0];

  // Check if CPU has non stoppable time stamp counter.
  constexpr uint32_t NonStopTscParameter = 0x80000007;
  if (num_ext_ids >= NonStopTscParameter) {
    cpuid(NonStopTscParameter, abcd);
    if (abcd[3] & (1 << 8))
      Add(CpuFeature::NonStopTsc);
  }

  #elif  CPU(ARM_FAMILY)
  #if CPU(ARM32) && __has_include(<sys/auxv.h>)
  constexpr uint32_t HwCapVFPv4 = (1<<16);

  uint32_t hwcaps = getauxval(AT_HWCAP);
  if (hwcaps & HwCapVFPv4) {
    Add(CpuFeature::Neon);
    Add(CpuFeature::NeonFma);
    Add(CpuFeature::Fp16);
  }

  #elif CPU(ARM32) && __has_include(<cpu-features.h>)
  uint64_t acf = android_getCpuFeatures();
  if (acf & ANDROID_CPU_ARM_FEATURE_NEON)     { Add(CpuFeature::Neon); }
  if (acf & ANDROID_CPU_ARM_FEATURE_NEON_FMA) { Add(CpuFeature::NeonFma); }
  if (acf & ANDROID_CPU_ARM_FEATURE_VFP_FP16) { Add(CpuFeature::Fp16); }
  #endif // CPU(ARM*)

  #endif // CPU(*)

  return features;
}

void CpuInfo::ClassInit() {
  g_number_of_cores_ = DetectNumberOfCores();
  g_features_ = RuntimeFeatures();
}

} // namespace stp
