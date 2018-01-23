// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_SYSTEM_CPUINFO_H_
#define STP_BASE_SYSTEM_CPUINFO_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Compiler/Simd.h"
#include "Base/Export.h"
#include "Base/Type/Basic.h"

namespace stp {

enum class CpuFeature : uint32_t {
  #if CPU(X86_FAMILY)
  Sse1       = 1u << 0,
  Sse2       = 1u << 1,
  Sse3       = 1u << 2,
  Ssse3      = 1u << 3,
  Sse41      = 1u << 4,
  Sse42      = 1u << 5,
  Avx        = 1u << 6,
  Cvt16      = 1u << 7,
  Fma        = 1u << 8,
  Avx2       = 1u << 9,
  Bmi1       = 1u << 10,
  Bmi2       = 1u << 11,
  Haswell    = Avx2 | Bmi1 | Bmi2 | Cvt16 | Fma,

  Avx512F    = 1u << 12,
  Avx512DQ   = 1u << 13,
  Avx512IFMA = 1u << 14,
  Avx512PF   = 1u << 15,
  Avx512ER   = 1u << 16,
  Avx512CD   = 1u << 17,
  Avx512BW   = 1u << 18,
  Avx512VL   = 1u << 19,

  Skylake    = Avx512F  | Avx512DQ | Avx512CD | Avx512BW | Avx512VL,

  NonStopTsc = 1u << 31,
  #endif
  #if CPU(ARM_FAMILY)
  Neon       = 1u << 0,
  NeonFma    = 1u << 1,
  Fp16       = 1u << 2,
  #endif
};

class BASE_EXPORT CpuInfo {
 public:
  static int NumberOfCores() { return g_number_of_cores_; }

  static bool Supports(CpuFeature feature);

 private:
  friend class BaseApplicationPart;

  using Features = TUnderlying<CpuFeature>;

  static void ClassInit();

  static constexpr Features CompilerFeatures();
  static Features RuntimeFeatures();

  static int g_number_of_cores_;

  static Features g_features_;
};

constexpr CpuInfo::Features CpuInfo::CompilerFeatures() {
  Features features = 0;
  #if CPU(X86_FAMILY)
  #if CPU_SIMD(SSE1)
  features |= static_cast<Features>(CpuFeature::Sse1);
  #endif
  #if CPU_SIMD(SSE2)
  features |= static_cast<Features>(CpuFeature::Sse2);
  #endif
  #if CPU_SIMD(SSE3)
  features |= static_cast<Features>(CpuFeature::Sse3);
  #endif
  #if CPU_SIMD(SSSE3)
  features |= static_cast<Features>(CpuFeature::Ssse3);
  #endif
  #if CPU_SIMD(SSE41)
  features |= static_cast<Features>(CpuFeature::Sse41);
  #endif
  #if CPU_SIMD(SSE42)
  features |= static_cast<Features>(CpuFeature::Sse42);
  #endif
  #if CPU_SIMD(AVX)
  features |= static_cast<Features>(CpuFeature::Avx);
  #endif
  #if CPU_SIMD(AVX2)
  features |= static_cast<Features>(CpuFeature::Avx2);
  #endif

  #elif CPU(ARM_FAMILY)
  #if CPU_SIMD(NEON)
  features |= static_cast<Features>(CpuFeature::Neon);
  #endif
  #if CPU(ARM64)
  features |= static_cast<Features>(CpuFeature::Neon);
  features |= static_cast<Features>(CpuFeature::NeonFma);
  features |= static_cast<Features>(CpuFeature::Fp16);
  #endif
  #endif // CPU(*)
  return features;
}

inline bool CpuInfo::Supports(CpuFeature feature) {
  constexpr Features compiler_features = CompilerFeatures();
  return ((g_features_ | compiler_features) & ToUnderlying(feature)) != 0;
}

} // namespace stp

#endif // STP_BASE_SYSTEM_CPUINFO_H_
