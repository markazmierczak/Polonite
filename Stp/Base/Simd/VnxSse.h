// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SIMD_VNXSSE_H_
#define STP_BASE_SIMD_VNXSSE_H_

// This file may assume <= SSE2, but must check CPU_SIMD() for anything more recent.

#include "Base/Debug/Assert.h"

#include <emmintrin.h>

namespace stp {

template<>
struct VecNx<2, float> {
  static constexpr int Size = 2;

  VecNx(const __m128& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(float val) : vec_(_mm_set1_ps(val)) {}
  VecNx(float a, float b) : vec_(_mm_setr_ps(a,b,0,0)) {}

  static VecNx load(const float* ptr) {
    return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)ptr));
  }
  void store(float* ptr) const { _mm_storel_pi((__m64*)ptr, vec_); }

  bool allTrue() const {
    return (_mm_movemask_epi8(_mm_castps_si128(vec_)) & 0xFF) == 0xFF;
  }
  bool anyTrue() const {
    return (_mm_movemask_epi8(_mm_castps_si128(vec_)) & 0xFF) != 0x00;
  }

  VecNx mathAbs() const { return _mm_andnot_ps(_mm_set1_ps(-0.f), vec_); }
  VecNx reciprocal() const { return _mm_rcp_ps(vec_); }
  VecNx mathSqrt() const { return _mm_sqrt_ps(vec_); }
  VecNx mathRsqrt() const { return _mm_rsqrt_ps(vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return _mm_min_ps(l.vec_, r.vec_);
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return _mm_max_ps(l.vec_, r.vec_);
  }

  VecNx operator+(const VecNx& o) const { return _mm_add_ps(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_ps(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return _mm_mul_ps(vec_, o.vec_); }
  VecNx operator/(const VecNx& o) const { return _mm_div_ps(vec_, o.vec_); }

  VecNx operator==(const VecNx& o) const { return _mm_cmpeq_ps(vec_, o.vec_); }
  VecNx operator!=(const VecNx& o) const { return _mm_cmpneq_ps(vec_, o.vec_); }
  VecNx operator<(const VecNx& o) const { return _mm_cmplt_ps(vec_, o.vec_); }
  VecNx operator>(const VecNx& o) const { return _mm_cmpgt_ps(vec_, o.vec_); }
  VecNx operator<=(const VecNx& o) const { return _mm_cmple_ps(vec_, o.vec_); }
  VecNx operator>=(const VecNx& o) const { return _mm_cmpge_ps(vec_, o.vec_); }

  float operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128 v; float fs[4]; } pun = {vec_};
    return pun.fs[k];
  }

  __m128 vec_;
};

template<>
struct VecNx<4, float> {
  static constexpr int Size = 4;

  VecNx(const __m128& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(float val) : vec_( _mm_set1_ps(val) ) {}
  VecNx(float a, float b, float c, float d) : vec_(_mm_setr_ps(a,b,c,d)) {}

  static VecNx load(const float* ptr) { return _mm_loadu_ps(ptr); }
  void store(float* ptr) const { _mm_storeu_ps(ptr, vec_); }

  bool allTrue() const {
    return _mm_movemask_epi8(_mm_castps_si128(vec_)) == 0xFFFF;
  }
  bool anyTrue() const {
    return _mm_movemask_epi8(_mm_castps_si128(vec_)) != 0x0000;
  }

  VecNx mathAbs() const { return _mm_andnot_ps(_mm_set1_ps(-0.f), vec_); }
  VecNx mathSqrt() const { return _mm_sqrt_ps(vec_); }
  VecNx mathRsqrt() const { return _mm_rsqrt_ps(vec_); }
  VecNx reciprocal() const { return _mm_rcp_ps(vec_); }

  VecNx mathFloor() const {
    #if CPU_SIMD(SSE41)
    return _mm_floor_ps(x.vec_);
    #else
    // Emulate _mm_floor_ps() with SSE2:
    //   - roundtrip through integers via truncation
    //   - subtract 1 if that's too big (possible for negative values).
    // This restricts the domain of our inputs to a maximum somehwere around 2^31.
    // Seems plenty big.
    __m128 roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(vec_));
    __m128 too_big = _mm_cmpgt_ps(roundtrip, vec_);
    return _mm_sub_ps(roundtrip, _mm_and_ps(too_big, _mm_set1_ps(1.f)));
    #endif
  }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return _mm_min_ps(l.vec_, r.vec_);
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return _mm_max_ps(l.vec_, r.vec_);
  }

  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    #if CPU_SIMD(SSE41)
    return _mm_blendv_ps(e.vec_, t.vec_, c.vec_);
    #else
    return _mm_or_ps(_mm_and_ps   (c.vec_, t.vec_),
                     _mm_andnot_ps(c.vec_, e.vec_));
    #endif
  }

  VecNx operator+(const VecNx& o) const { return _mm_add_ps(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_ps(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return _mm_mul_ps(vec_, o.vec_); }
  VecNx operator/(const VecNx& o) const { return _mm_div_ps(vec_, o.vec_); }

  VecNx operator==(const VecNx& o) const { return _mm_cmpeq_ps(vec_, o.vec_); }
  VecNx operator!=(const VecNx& o) const { return _mm_cmpneq_ps(vec_, o.vec_); }
  VecNx operator<(const VecNx& o) const { return _mm_cmplt_ps(vec_, o.vec_); }
  VecNx operator>(const VecNx& o) const { return _mm_cmpgt_ps(vec_, o.vec_); }
  VecNx operator<=(const VecNx& o) const { return _mm_cmple_ps(vec_, o.vec_); }
  VecNx operator>=(const VecNx& o) const { return _mm_cmpge_ps(vec_, o.vec_); }

  float operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128 v; float fs[4]; } pun = {vec_};
    return pun.fs[k];
  }

  __m128 vec_;
};

template<>
struct VecNx<4, int32_t> {
  static constexpr int Size = 4;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(int32_t val) : vec_(_mm_set1_epi32(val)) {}
  VecNx(int32_t a, int32_t b, int32_t c, int32_t d) : vec_(_mm_setr_epi32(a,b,c,d)) {}

  static VecNx load(const int32_t* ptr) {
    return _mm_loadu_si128((const __m128i*)ptr);
  }
  void store(int32_t* ptr) const { _mm_storeu_si128((__m128i*)ptr, vec_); }

  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    #if CPU_SIMD(SSE41)
    return _mm_blendv_epi8(e.vec_, t.vec_, c.vec_);
    #else
    return _mm_or_si128(_mm_and_si128   (c.vec_, t.vec_),
                        _mm_andnot_si128(c.vec_, e.vec_));
    #endif
  }

  VecNx operator&(const VecNx& o) const { return _mm_and_si128(vec_, o.vec_); }
  VecNx operator|(const VecNx& o) const { return _mm_or_si128(vec_, o.vec_); }
  VecNx operator^(const VecNx& o) const { return _mm_xor_si128(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return _mm_slli_epi32(vec_, amount); }
  VecNx operator>>(int amount) const { return _mm_srai_epi32(vec_, amount); }

  VecNx operator+(const VecNx& o) const { return _mm_add_epi32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_epi32(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const {
    __m128i mul20 = _mm_mul_epu32(vec_, o.vec_),
            mul31 = _mm_mul_epu32(_mm_srli_si128(vec_, 4),
                                  _mm_srli_si128(o.vec_, 4));
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(mul20, _MM_SHUFFLE(0,0,2,0)),
                              _mm_shuffle_epi32(mul31, _MM_SHUFFLE(0,0,2,0)));
  }

  VecNx operator==(const VecNx& o) const { return _mm_cmpeq_epi32 (vec_, o.vec_); }
  VecNx operator <(const VecNx& o) const { return _mm_cmplt_epi32 (vec_, o.vec_); }
  VecNx operator >(const VecNx& o) const { return _mm_cmpgt_epi32 (vec_, o.vec_); }

  int32_t operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128i v; int32_t is[4]; } pun = {vec_};
    return pun.is[k];
  }

  __m128i vec_;
};

template<>
struct VecNx<4, uint32_t> {
  static constexpr int Size = 4;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint32_t val) : vec_(_mm_set1_epi32(val)) {}
  VecNx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) : vec_(_mm_setr_epi32(a,b,c,d)) {}

  static VecNx load(const uint32_t* ptr) {
    return _mm_loadu_si128((const __m128i*)ptr);
  }
  void store(uint32_t* ptr) const { _mm_storeu_si128((__m128i*)ptr, vec_); }

  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    #if CPU_SIMD(SSE41)
    return _mm_blendv_epi8(e.vec_, t.vec_, c.vec_);
    #else
    return _mm_or_si128(_mm_and_si128   (c.vec_, t.vec_),
                        _mm_andnot_si128(c.vec_, e.vec_));
    #endif
  }

  VecNx operator&(const VecNx& o) const { return _mm_and_si128(vec_, o.vec_); }
  VecNx operator|(const VecNx& o) const { return _mm_or_si128(vec_, o.vec_); }
  VecNx operator^(const VecNx& o) const { return _mm_xor_si128(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return _mm_slli_epi32(vec_, amount); }
  VecNx operator>>(int amount) const { return _mm_srli_epi32(vec_, amount); }

  VecNx operator+(const VecNx& o) const { return _mm_add_epi32(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_epi32(vec_, o.vec_); }

  VecNx operator==(const VecNx& o) const { return _mm_cmpeq_epi32 (vec_, o.vec_); }

  uint32_t operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128i v; uint32_t is[4]; } pun = {vec_};
    return pun.is[k];
  }

  __m128i vec_;
};

template<>
struct VecNx<4, uint16_t> {
  static constexpr int Size = 4;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint16_t val) : vec_(_mm_set1_epi16(val)) {}
  VecNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
      : vec_(_mm_setr_epi16(a,b,c,d,0,0,0,0)) {}

  static Vec4h load(const uint16_t* ptr) {
    return _mm_loadl_epi64((const __m128i*)ptr);
  }
  void store(uint16_t* ptr) const { _mm_storel_epi64((__m128i*)ptr, vec_); }

  VecNx operator+(const VecNx& o) const { return _mm_add_epi16(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_epi16(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return _mm_mullo_epi16(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return _mm_slli_epi16(vec_, amount); }
  VecNx operator>>(int amount) const { return _mm_srli_epi16(vec_, amount); }

  uint16_t operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128i v; uint16_t us[8]; } pun = {vec_};
    return pun.us[k];
  }

  __m128i vec_;
};

template<>
struct VecNx<8, uint16_t> {
  static constexpr int Size = 8;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint16_t val) : vec_(_mm_set1_epi16(val)) {}
  VecNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
        uint16_t e, uint16_t f, uint16_t g, uint16_t h)
      : vec_(_mm_setr_epi16(a,b,c,d,e,f,g,h)) {}

  static VecNx load(const uint16_t* ptr) {
    return _mm_loadu_si128((const __m128i*)ptr);
  }
  void store(uint16_t* ptr) const { _mm_storeu_si128((__m128i*)ptr, vec_); }

  static VecNx min(const VecNx& a, const VecNx& b) {
    // No unsigned _mm_min_epu16, so we'll shift into a space where we can use
    // the signed version, _mm_min_epi16, then shift back.
    const uint16_t top = 0x8000;
    // Keep this separate from _mm_set1_epi16 or MSVC will whine.
    const __m128i top_8x = _mm_set1_epi16(top);
    return _mm_add_epi8(top_8x, _mm_min_epi16(_mm_sub_epi8(a.vec_, top_8x),
                                              _mm_sub_epi8(b.vec_, top_8x)));
  }

  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    #if CPU_SIMD(SSE41)
    return _mm_blendv_epi8(e.vec_, t.vec_, c.vec_);
    #else
    return _mm_or_si128(_mm_and_si128   (c.vec_, t.vec_),
                        _mm_andnot_si128(c.vec_, e.vec_));
    #endif
  }

  VecNx operator+(const VecNx& o) const { return _mm_add_epi16(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_epi16(vec_, o.vec_); }
  VecNx operator*(const VecNx& o) const { return _mm_mullo_epi16(vec_, o.vec_); }

  VecNx operator<<(int amount) const { return _mm_slli_epi16(vec_, amount); }
  VecNx operator>>(int amount) const { return _mm_srli_epi16(vec_, amount); }

  uint16_t operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128i v; uint16_t us[8]; } pun = {vec_};
    return pun.us[k];
  }

  __m128i vec_;
};

template<>
struct VecNx<4, uint8_t> {
  static constexpr int Size = 4;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
      : vec_(_mm_setr_epi8(a,b,c,d, 0,0,0,0, 0,0,0,0, 0,0,0,0)) {}

  static VecNx load(const uint8_t* ptr) {
    return _mm_cvtsi32_si128(*(const int*)ptr);
  }
  void store(uint8_t* ptr) const { *(int*)ptr = _mm_cvtsi128_si32(vec_); }

  __m128i vec_;
};

template<>
struct VecNx<16, uint8_t> {
  static constexpr int Size = 16;

  VecNx(const __m128i& vec) : vec_(vec) {}

  VecNx() = default;
  VecNx(uint8_t val) : vec_(_mm_set1_epi8(val)) {}

  VecNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
      uint8_t e, uint8_t f, uint8_t g, uint8_t h,
      uint8_t i, uint8_t j, uint8_t k, uint8_t l,
      uint8_t m, uint8_t n, uint8_t o, uint8_t p)
      : vec_(_mm_setr_epi8(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p)) {}

  static VecNx load(const uint8_t* ptr) {
    return _mm_loadu_si128((const __m128i*)ptr);
  }
  void store(uint8_t* ptr) const { _mm_storeu_si128((__m128i*)ptr, vec_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return _mm_min_epu8(l.vec_, r.vec_);
  }

  static VecNx saturatedAdd(const VecNx& l, const VecNx& r) {
    return _mm_adds_epu8(l.vec_, r.vec_);
  }

  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return _mm_or_si128(_mm_and_si128   (c.vec_, t.vec_),
                        _mm_andnot_si128(c.vec_, e.vec_));
  }

  VecNx operator+(const VecNx& o) const { return _mm_add_epi8(vec_, o.vec_); }
  VecNx operator-(const VecNx& o) const { return _mm_sub_epi8(vec_, o.vec_); }

  VecNx operator<(const VecNx& o) const {
    // There's no unsigned _mm_cmplt_epu8, so we flip the sign bits then use
    // a signed compare.
    auto flip = _mm_set1_epi8(static_cast<char>(static_cast<uint8_t>(0x80)));
    return _mm_cmplt_epi8(_mm_xor_si128(flip, vec_),
                          _mm_xor_si128(flip, o.vec_));
  }

  uint8_t operator[](int k) const {
    ASSERT(0 <= k && k < Size);
    union { __m128i v; uint8_t us[16]; } pun = {vec_};
    return pun.us[k];
  }

  __m128i vec_;
};

template<>
inline Vec4f vnxCast<float, int32_t>(const Vec4i& src) {
  return _mm_cvtepi32_ps(src.vec_);
}

template<>
inline Vec4i vnxCast<int32_t, float, 4>(const Vec4f& src) {
  return _mm_cvttps_epi32(src.vec_);
}

template<>
inline Vec4h vnxCast<uint16_t, int32_t>(const Vec4i& src) {
  #if CPU_SIMD(SSE3)
  // With SSSE3, we can just shuffle the low 2 bytes from each lane right into place.
  const int _ = ~0;
  return _mm_shuffle_epi8(src.vec_, _mm_setr_epi8(0,1, 4,5, 8,9, 12,13, _,_,_,_,_,_,_,_));
  #else
  // With SSE2, we have to sign extend our input, making _mm_packs_epi32 do the pack we want.
  __m128i x = _mm_srai_epi32(_mm_slli_epi32(src.vec_, 16), 16);
  return _mm_packs_epi32(x,x);
  #endif
}

template<>
inline Vec4h vnxCast<uint16_t, float>(const Vec4f& src) {
  return vnxCast<uint16_t>(vnxCast<int32_t>(src));
}

template<>
inline Vec4b vnxCast<uint8_t, float>(const Vec4f& src) {
  auto _32 = _mm_cvttps_epi32(src.vec_);
  #if CPU_SIMD(SSE3)
  const int _ = ~0;
  return _mm_shuffle_epi8(_32, _mm_setr_epi8(0,4,8,12, _,_,_,_, _,_,_,_, _,_,_,_));
  #else
  auto _16 = _mm_packus_epi16(_32, _32);
  return     _mm_packus_epi16(_16, _16);
  #endif
}

template<>
inline Vec4f vnxCast<float, uint8_t>(const Vec4b& src) {
  #if CPU_SIMD(SSE3)
  const int _ = ~0;
  auto _32 = _mm_shuffle_epi8(src.vec_, _mm_setr_epi8(0,_,_,_, 1,_,_,_, 2,_,_,_, 3,_,_,_));
  #else
  __m128i _16 = _mm_unpacklo_epi8(src.vec_, _mm_setzero_si128()),
          _32 = _mm_unpacklo_epi16(_16,     _mm_setzero_si128());
  #endif
  return _mm_cvtepi32_ps(_32);
}

template<>
inline Vec4f vnxCast<float, uint16_t>(const Vec4h& src) {
  auto _32 = _mm_unpacklo_epi16(src.vec_, _mm_setzero_si128());
  return _mm_cvtepi32_ps(_32);
}

template<>
inline Vec16b vnxCast<uint8_t, float>(const Vec16f& src) {
  Vec8f ab, cd;
  VnxMath::split(src, &ab, &cd);

  Vec4f a,b,c,d;
  VnxMath::split(ab, &a, &b);
  VnxMath::split(cd, &c, &d);

  return _mm_packus_epi16(_mm_packus_epi16(_mm_cvttps_epi32(a.vec_),
                                           _mm_cvttps_epi32(b.vec_)),
                          _mm_packus_epi16(_mm_cvttps_epi32(c.vec_),
                                           _mm_cvttps_epi32(d.vec_)));
}

template<>
inline Vec4h vnxCast<uint16_t, uint8_t>(const Vec4b& src) {
  return _mm_unpacklo_epi8(src.vec_, _mm_setzero_si128());
}

template<>
inline Vec4b vnxCast<uint8_t, uint16_t>(const Vec4h& src) {
  return _mm_packus_epi16(src.vec_, src.vec_);
}

template<>
inline Vec4i vnxCast<int32_t, uint16_t>(const Vec4h& src) {
  return _mm_unpacklo_epi16(src.vec_, _mm_setzero_si128());
}

template<>
inline Vec4b vnxCast<uint8_t, int32_t>(const Vec4i& src) {
  return _mm_packus_epi16(_mm_packus_epi16(src.vec_, src.vec_), src.vec_);
}

template<>
inline Vec4i vnxCast<int32_t, uint32_t>(const Vec4u& src) {
  return src.vec_;
}

} // namespace stp

#endif // STP_BASE_SIMD_VNXSSE_H_
