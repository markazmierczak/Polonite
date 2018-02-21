// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SIMD_VNXNEON_H_
#define STP_BASE_SIMD_VNXNEON_H_

#include "Base/Debug/Assert.h"
#include "Base/Simd/Vnx.h"

#include <arm_neon.h>

namespace stp {

#if !CPU(ARM64)
// ARMv8 has vrndmq_f32 to floor 4 floats.  Here we emulate it:
//   - roundtrip through integers via truncation
//   - subtract 1 if that's too big (possible for negative values).
// This restricts the domain of our inputs to a maximum somewhere around 2^31.
// Seems plenty big.
static inline float32x4_t armv7_vrndmq_f32(float32x4_t v) {
  auto roundtrip = vcvtq_f32_s32(vcvtq_s32_f32(v));
  auto too_big = vcgtq_f32(roundtrip, v);
  return vsubq_f32(roundtrip, (float32x4_t)vandq_u32(too_big, (uint32x4_t)vdupq_n_f32(1)));
}
#endif // !CPU(ARM64)

template<>
struct VecNx<2, float> {
  static constexpr int Size() { return 2; }

  VecNx(float32x2_t vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(float val) : vec_(vdup_n_f32(val)) {}
  VecNx(float a, float b) { vec_ = (float32x2_t) { a, b }; }

  static VecNx Load(const float* ptr) { return vld1_f32(ptr); }
  void Store(float* ptr) const { vst1_f32(ptr, vec_); }

  bool allTrue() const {
    auto v = vreinterpret_u32_f32(vec_);
    return vget_lane_u32(v,0) && vget_lane_u32(v,1);
  }
  bool anyTrue() const {
    auto v = vreinterpret_u32_f32(vec_);
    return vget_lane_u32(v,0) || vget_lane_u32(v,1);
  }

  VecNx mathAbs() const { return vabs_f32(vec_); }

  VecNx Reciprocal() const {
    float32x2_t est0 = vrsqrte_f32(vec_);
    return vmul_f32(vrsqrts_f32(vec_, vmul_f32(est0, est0)), est0);
  }

  VecNx mathSqrt() const {
    #if CPU(ARM64)
    return vsqrt_f32(vec_);
    #else
    float32x2_t est0 = vrsqrte_f32(vec_),
                est1 = vmul_f32(vrsqrts_f32(vec_, vmul_f32(est0, est0)), est0),
                est2 = vmul_f32(vrsqrts_f32(vec_, vmul_f32(est1, est1)), est1);
    return vmul_f32(vec_, est2);
    #endif
  }

  VecNx mathRsqrt() const {
    float32x2_t est0 = vrsqrte_f32(vec_);
    return vmul_f32(vrsqrts_f32(vec_, vmul_f32(est0, est0)), est0);
  }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vmin_f32(l.vec_, r.vec_);
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return vmax_f32(l.vec_, r.vec_);
  }

  VecNx operator+(const VecNx& o) const { return vadd_f32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsub_f32(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmul_f32(vec_, o.vec_); }
  VecNx operator/(const VecNx& o) const {
    #if CPU(ARM64)
    return vdiv_f32(vec_, o.vec_);
    #else
    float32x2_t est0 = vrecpe_f32(o.vec_),
                est1 = vmul_f32(vrecps_f32(est0, o.vec_), est0),
                est2 = vmul_f32(vrecps_f32(est1, o.vec_), est1);
    return vmul_f32(vec_, est2);
    #endif
  }

  VecNx operator==(const VecNx& o) const { return vreinterpret_f32_u32(vceq_f32(vec_, o.vec_)); }
  VecNx operator <(const VecNx& o) const { return vreinterpret_f32_u32(vclt_f32(vec_, o.vec_)); }
  VecNx operator >(const VecNx& o) const { return vreinterpret_f32_u32(vcgt_f32(vec_, o.vec_)); }
  VecNx operator<=(const VecNx& o) const { return vreinterpret_f32_u32(vcle_f32(vec_, o.vec_)); }
  VecNx operator>=(const VecNx& o) const { return vreinterpret_f32_u32(vcge_f32(vec_, o.vec_)); }
  VecNx operator!=(const VecNx& o) const { return vreinterpret_f32_u32(vmvn_u32(vceq_f32(vec_, o.vec_))); }

  float operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { float32x2_t v; float fs[2]; } pun = {vec_};
    return pun.fs[k];
  }

  float32x2_t vec_;
};

template<>
struct VecNx<4, float> {
  static constexpr int Size() { return 4; }

  VecNx(float32x4_t vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(float val) : vec_(vdupq_n_f32(val)) {}
  VecNx(float a, float b, float c, float d) { vec_ = (float32x4_t) { a, b, c, d }; }

  static VecNx Load(const float* ptr) { return vld1q_f32(ptr); }
  void Store(float* ptr) const { vst1q_f32(ptr, vec_); }

  bool allTrue() const {
    auto v = vreinterpretq_u32_f32(vec_);
    return vgetq_lane_u32(v,0) && vgetq_lane_u32(v,1) && vgetq_lane_u32(v,2) && vgetq_lane_u32(v,3);
  }

  bool anyTrue() const {
    auto v = vreinterpretq_u32_f32(vec_);
    return vgetq_lane_u32(v,0) || vgetq_lane_u32(v,1) || vgetq_lane_u32(v,2) || vgetq_lane_u32(v,3);
  }

  VecNx mathAbs() const { return vabsq_f32(vec_); }

  VecNx Reciprocal() const {
    float32x4_t est0 = vrecpeq_f32(vec_),
                est1 = vmulq_f32(vrecpsq_f32(est0, vec_), est0);
    return est1;
  }

  VecNx mathSqrt() const {
    #if CPU(ARM64)
    return vsqrtq_f32(vec_);
    #else
    float32x4_t est0 = vrsqrteq_f32(vec_),
                est1 = vmulq_f32(vrsqrtsq_f32(vec_, vmulq_f32(est0, est0)), est0),
                est2 = vmulq_f32(vrsqrtsq_f32(vec_, vmulq_f32(est1, est1)), est1);
    return vmulq_f32(vec_, est2);
    #endif
  }

  VecNx mathRsqrt() const {
    float32x4_t est0 = vrsqrteq_f32(vec_);
    return vmulq_f32(vrsqrtsq_f32(vec_, vmulq_f32(est0, est0)), est0);
  }

  VecNx mathFloor() const {
    #if CPU(ARM64)
    return vrndmq_f32(vec_);
    #else
    return armv7_vrndmq_f32(vec_);
    #endif
  }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vminq_f32(l.vec_, r.vec_);
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return vmaxq_f32(l.vec_, r.vec_);
  }

  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbslq_f32(vreinterpretq_u32_f32(c.vec_), t.vec_, e.vec_);
  }

  VecNx operator+(const VecNx& o) const { return vaddq_f32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsubq_f32(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmulq_f32(vec_, o.vec_); }
  VecNx operator/(const VecNx& o) const {
    #if CPU(ARM64)
    return vdivq_f32(vec_, o.vec_);
    #else
    float32x4_t est0 = vrecpeq_f32(o.vec_),
                est1 = vmulq_f32(vrecpsq_f32(est0, o.vec_), est0),
                est2 = vmulq_f32(vrecpsq_f32(est1, o.vec_), est1);
    return vmulq_f32(vec_, est2);
    #endif
  }

  VecNx operator==(const VecNx& o) const { return vreinterpretq_f32_u32(vceqq_f32(vec_, o.vec_)); }
  VecNx operator <(const VecNx& o) const { return vreinterpretq_f32_u32(vcltq_f32(vec_, o.vec_)); }
  VecNx operator >(const VecNx& o) const { return vreinterpretq_f32_u32(vcgtq_f32(vec_, o.vec_)); }
  VecNx operator<=(const VecNx& o) const { return vreinterpretq_f32_u32(vcleq_f32(vec_, o.vec_)); }
  VecNx operator>=(const VecNx& o) const { return vreinterpretq_f32_u32(vcgeq_f32(vec_, o.vec_)); }
  VecNx operator!=(const VecNx& o) const { return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(vec_, o.vec_))); }

  float operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { float32x4_t v; float fs[4]; } pun = {vec_};
    return pun.fs[k];
  }

  float32x4_t vec_;
};

template<>
struct VecNx<4, int32_t> {
  static constexpr int Size() { return 4; }

  VecNx(const int32x4_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(int32_t val) : vec_(vdupq_n_s32(val)) {}
  VecNx(int32_t a, int32_t b, int32_t c, int32_t d) {
    vec_ = (int32x4_t) { a, b, c, d };
  }

  static VecNx Load(const int32_t* ptr) { return vld1q_s32(ptr); }
  void Store(int32_t* ptr) const { vst1q_s32(ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vminq_s32(l.vec_, r.vec_);
  }

  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbslq_f32(vreinterpretq_u32_s32(c.vec_), t.vec_, e.vec_);
  }

  VecNx operator&(const VecNx& o) const { return vandq_s32(vec_, o.vec_); }
  VecNx operator|(const VecNx& o) const { return vorrq_s32(vec_, o.vec_); }
  VecNx operator^(const VecNx& o) const { return veorq_s32(vec_, o.vec_); }

  VecNx operator+(const VecNx& o) const { return vaddq_s32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsubq_s32(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmulq_s32(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return vec_ << VecNx(amount).vec_; }
  VecNx operator>>(int amount) const { return vec_ >> VecNx(amount).vec_; }

  VecNx operator==(const VecNx& o) const {
    return vreinterpretq_s32_u32(vceqq_s32(vec_, o.vec_));
  }
  VecNx operator<(const VecNx& o) const {
    return vreinterpretq_s32_u32(vcltq_s32(vec_, o.vec_));
  }
  VecNx operator>(const VecNx& o) const {
    return vreinterpretq_s32_u32(vcgtq_s32(vec_, o.vec_));
  }

  int32_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { int32x4_t v; int32_t is[4]; } pun = {vec_};
    return pun.is[k];
  }

  int32x4_t vec_;
};

template<>
struct VecNx<4, uint32_t> {
  static constexpr int Size() { return 4; }

  VecNx(const uint32x4_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint32_t val) : vec_(vdupq_n_u32(val)) {}
  VecNx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    vec_ = (uint32x4_t) { a, b, c, d };
  }

  static VecNx Load(const uint32_t* ptr) { return vld1q_u32(ptr); }
  void Store(uint32_t* ptr) const { vst1q_u32(ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vminq_u32(l.vec_, r.vec_);
  }

  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbslq_u32(c.vec_, t.vec_, e.vec_);
  }

  VecNx operator&(const VecNx& o) const { return vandq_u32(vec_, o.vec_); }
  VecNx operator|(const VecNx& o) const { return vorrq_u32(vec_, o.vec_); }
  VecNx operator^(const VecNx& o) const { return veorq_u32(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return vec_ << VecNx(amount).vec_; }
  VecNx operator>>(int amount) const { return vec_ >> VecNx(amount).vec_; }

  VecNx operator+(const VecNx& o) const { return vaddq_u32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsubq_u32(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmulq_u32(vec_, o.vec_); }

  VecNx operator==(const VecNx& o) const { return vceqq_u32(vec_, o.vec_); }
  VecNx operator< (const VecNx& o) const { return vcltq_u32(vec_, o.vec_); }
  VecNx operator> (const VecNx& o) const { return vcgtq_u32(vec_, o.vec_); }

  uint32_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { uint32x4_t v; uint32_t is[4]; } pun = {vec_};
    return pun.is[k];
  }

  uint32x4_t vec_;
};

template<>
struct VecNx<4, uint16_t> {
  static constexpr int Size() { return 4; }

  VecNx(const uint16x4_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint16_t val) : vec_(vdup_n_u16(val)) {}
  VecNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
    vec_ = (uint16x4_t) { a,b,c,d };
  }

  static VecNx Load(const uint16_t* ptr) { return vld1_u16(ptr); }
  void Store(uint16_t* ptr) const { vst1_u16(ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vmin_u16(l.vec_, r.vec_);
  }

  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbsl_u16(c.vec_, t.vec_, e.vec_);
  }

  VecNx operator+(const VecNx& o) const { return vadd_u16(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsub_u16(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmul_u16(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return vec_ << VecNx(amount).vec_; }
  VecNx operator>>(int amount) const { return vec_ >> VecNx(amount).vec_; }

  uint16_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { uint16x4_t v; uint16_t us[4]; } pun = {vec_};
    return pun.us[k];
  }

  uint16x4_t vec_;
};

template<>
struct VecNx<8, uint16_t> {
  static constexpr int Size() { return 8; }

  VecNx(const uint16x8_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint16_t val) : vec_(vdupq_n_u16(val)) {}
  VecNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
        uint16_t e, uint16_t f, uint16_t g, uint16_t h) {
    vec_ = (uint16x8_t) { a,b,c,d, e,f,g,h };
  }

  static VecNx Load(const uint16_t* ptr) { return vld1q_u16(ptr); }
  void Store(uint16_t* ptr) const { vst1q_u16(ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vminq_u16(l.vec_, r.vec_);
  }
  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbslq_u16(c.vec_, t.vec_, e.vec_);
  }

  VecNx operator+(const VecNx& o) const { return vaddq_u16(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsubq_u16(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return vmulq_u16(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return vec_ << VecNx(amount).vec_; }
  VecNx operator>>(int amount) const { return vec_ >> VecNx(amount).vec_; }

  uint16_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { uint16x8_t v; uint16_t us[8]; } pun = {vec_};
    return pun.us[k];
  }

  uint16x8_t vec_;
};

template<>
struct VecNx<4, uint8_t> {
  static constexpr int Size() { return 4; }

  typedef uint32_t alignas(1) unaligned_uint32_t;

  VecNx(const uint8x8_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    vec_ = (uint8x8_t){a,b,c,d, 0,0,0,0};
  }

  static VecNx Load(const uint8_t* ptr) {
    return (uint8x8_t)vld1_dup_u32((const uint32_t*)ptr);
  }
  void Store(uint8_t* ptr) const {
    vst1_lane_u32((unaligned_uint32_t*)ptr, (uint32x2_t)vec_, 0);
  }

  uint8_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { uint8x8_t v; uint8_t us[8]; } pun = {vec_};
    return pun.us[k];
  }

  // Implement as needed.

  uint8x8_t vec_;
};

template<>
struct VecNx<16, uint8_t> {
  static constexpr int Size() { return 16; }

  VecNx(const uint8x16_t& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint8_t val) : vec_(vdupq_n_u8(val)) {}
  VecNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
        uint8_t e, uint8_t f, uint8_t g, uint8_t h,
        uint8_t i, uint8_t j, uint8_t k, uint8_t l,
        uint8_t m, uint8_t n, uint8_t o, uint8_t p) {
    vec_ = (uint8x16_t) { a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p };
  }

  static VecNx Load(const uint8_t* ptr) { return vld1q_u8(ptr); }
  void Store(uint8_t* ptr) const { vst1q_u8(ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return vminq_u8(l.vec_, r.vec_);
  }

  static VecNx SaturatedAdd(const VecNx& l, const VecNx& r) {
    return vqaddq_u8(l.vec_, r.vec_);
  }

  static VecNx Ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return vbslq_u8(c.vec_, t.vec_, e.vec_);
  }

  VecNx operator+(const VecNx& o) const { return vaddq_u8(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return vsubq_u8(vec_, o.vec_); }

  VecNx operator<(const VecNx& o) const { return vcltq_u8(vec_, o.vec_); }

  uint8_t operator[](int k) const {
    ASSERT(0 <= k && k < Size());
    union { uint8x16_t v; uint8_t us[16]; } pun = {vec_};
    return pun.us[k];
  }

  uint8x16_t vec_;
};

template<>
inline Vec4i vnx_cast<int32_t, float>(const Vec4f& src) {
  return vcvtq_s32_f32(src.vec_);

}
template<>
inline Vec4f vnx_cast<float, int32_t>(const Vec4i& src) {
  return vcvtq_f32_s32(src.vec_);
}

template<>
inline Vec4h vnx_cast<uint16_t, float>(const Vec4f& src) {
  return vqmovn_u32(vcvtq_u32_f32(src.vec_));
}

template<>
inline Vec4f vnx_cast<float, uint16_t>(const Vec4h& src) {
  return vcvtq_f32_u32(vmovl_u16(src.vec_));
}

template<>
inline Vec4b vnx_cast<uint8_t, float>(const Vec4f& src) {
  uint32x4_t _32 = vcvtq_u32_f32(src.vec_);
  uint16x4_t _16 = vqmovn_u32(_32);
  return vqmovn_u16(vcombine_u16(_16, _16));
}

template<>
inline Vec4f vnx_cast<float, uint8_t>(const Vec4b& src) {
  uint16x8_t _16 = vmovl_u8 (src.vec_) ;
  uint32x4_t _32 = vmovl_u16(vget_low_u16(_16));
  return vcvtq_f32_u32(_32);
}

template<>
inline Vec16b vnx_cast<uint8_t, float>(const Vec16f& src) {
  Vec8f ab, cd;
  VnxMath::Split(src, &ab, &cd);

  Vec4f a,b,c,d;
  VnxMath::Split(ab, &a, &b);
  VnxMath::Split(cd, &c, &d);
  return vuzpq_u8(vuzpq_u8((uint8x16_t)vcvtq_u32_f32(a.vec_),
                           (uint8x16_t)vcvtq_u32_f32(b.vec_)).val[0],
                  vuzpq_u8((uint8x16_t)vcvtq_u32_f32(c.vec_),
                           (uint8x16_t)vcvtq_u32_f32(d.vec_)).val[0]).val[0];
}

template<>
inline Vec4h vnx_cast<uint16_t, uint8_t>(const Vec4b& src) {
  return vget_low_u16(vmovl_u8(src.vec_));
}

template<>
inline Vec4b vnx_cast<uint8_t, uint16_t>(const Vec4h& src) {
  return vmovn_u16(vcombine_u16(src.vec_, src.vec_));
}

template<>
inline Vec4b vnx_cast<uint8_t, int32_t>(const Vec4i& src) {
  uint16x4_t _16 = vqmovun_s32(src.vec_);
  return vqmovn_u16(vcombine_u16(_16, _16));
}

template<>
inline Vec4i vnx_cast<int32_t, uint16_t>(const Vec4h& src) {
  return vreinterpretq_s32_u32(vmovl_u16(src.vec_));
}

template<>
inline Vec4h vnx_cast<uint16_t, int32_t>(const Vec4i& src) {
  return vmovn_u32(vreinterpretq_u32_s32(src.vec_));
}

template<>
inline Vec4i vnx_cast<int32_t, uint32_t>(const Vec4u& src) {
  return vreinterpretq_s32_u32(src.vec_);
}

} // namespace stp

#endif // STP_BASE_SIMD_VNXNEON_H_
