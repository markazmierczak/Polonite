// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2015 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_SIMD_VNX_H_
#define STP_BASE_SIMD_VNX_H_

#include "Base/Compiler/Simd.h"
#include "Base/Debug/Assert.h"
#include "Base/Math/Math.h"
#include "Base/Type/Limits.h"
#include "Base/Type/IntegerSelection.h"
#include "Base/Type/Variable.h"

#include <string.h>

namespace stp {

// #define DISABLE_VNX_SIMD 1

// The default implementations just fall back on a pair of size N/2.
template<unsigned N, typename T>
struct VecNx {
  static_assert(0 == (N & (N-1)), "!");
  typedef VecNx<N/2,T> Half;

  VecNx() = default;
  VecNx(const Half& lo, const Half& hi) : lo_(lo), hi_(hi) {}
  VecNx(T val) : lo_(val), hi_(val) {}

  VecNx(T a, T b) : lo_(a), hi_(b) { static_assert(N==2, "!"); }
  VecNx(T a, T b, T c, T d) : lo_(a,b), hi_(c,d) { static_assert(N==4, "!"); }
  VecNx(T a, T b, T c, T d,  T e, T f, T g, T h)
      : lo_(a,b,c,d), hi_(e,f,g,h) { static_assert(N==8, "!"); }
  VecNx(T a, T b, T c, T d,  T e, T f, T g, T h,
        T i, T j, T k, T l,  T m, T n, T o, T p)
      : lo_(a,b,c,d, e,f,g,h),
        hi_(i,j,k,l, m,n,o,p) { static_assert(N==16, "!"); }

  // Loads vector from memory.
  static VecNx load(const T ptr[N]) {
    return { Half::load(ptr), Half::load(ptr + N/2) };
  }

  // Stores vector to memory.
  void store(T* ptr) const {
    lo_.store(ptr + 0);
    hi_.store(ptr + N/2);
  }

  // Returns true if all components are != 0.
  bool allTrue() const { return lo_.allTrue() && hi_.allTrue(); }

  // Returns true if any component is != 0.
  bool anyTrue() const { return lo_.anyTrue() || hi_.anyTrue(); }

  // Returns absolute value for each component.
  VecNx mathAbs() const { return { lo_.mathAbs(), hi_.mathAbs() }; }

  // Returns reciprocal value (1/x) for each component in |x|.
  VecNx reciprocal() const { return { lo_.reciprocal(), hi_.reciprocal() }; }

  // Returns square root value for each component in |x|.
  VecNx mathSqrt() const { return { lo_.mathSqrt(), hi_.mathSqrt() }; }

  // Returns inverted square root value (i.e. the Reciprocal of square root)
  // for each component in |x|.
  VecNx mathRsqrt() const { return { lo_.mathRsqrt(), hi_.mathRsqrt() }; }

  // Returns floor value for each component.
  VecNx mathFloor() const { return { lo_.mathFloor(), hi_.mathFloor() }; }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return { Half::min(l.lo_, r.lo_), Half::min(l.hi_, r.hi_) };
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return { Half::max(l.lo_, r.lo_), Half::max(l.hi_, r.hi_) };
  }

  // Ternary operator for vectors.
  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return { Half::ternary(c.lo_, t.lo_, e.lo_),
             Half::ternary(c.hi_, t.hi_, e.hi_) };
  }

  // Saturated addition.
  static VecNx saturatedAdd(const VecNx& l, const VecNx& r) {
    return { Half::saturatedAdd(l.lo_, r.lo_), Half::saturatedAdd(l.hi_, r.hi_) };
  }

  VecNx operator!() const { return { !lo_, !hi_ }; }
  VecNx operator-() const { return { -lo_, -hi_ }; }
  VecNx operator~() const { return { ~lo_, ~hi_ }; }

  VecNx operator<<(unsigned bits) const { return { lo_ << bits, hi_ << bits }; }
  VecNx operator>>(unsigned bits) const { return { lo_ >> bits, hi_ >> bits }; }

  VecNx operator+(const VecNx& o) const { return { lo_ + o.lo_, hi_ + o.hi_ }; }
  VecNx operator-(const VecNx& o) const { return { lo_ - o.lo_, hi_ - o.hi_ }; }
  VecNx operator*(const VecNx& o) const { return { lo_ * o.lo_, hi_ * o.hi_ }; }
  VecNx operator/(const VecNx& o) const { return { lo_ / o.lo_, hi_ / o.hi_ }; }

  VecNx operator&(const VecNx& o) const { return { lo_ & o.lo_, hi_ & o.hi_ }; }
  VecNx operator|(const VecNx& o) const { return { lo_ | o.lo_, hi_ | o.hi_ }; }
  VecNx operator^(const VecNx& o) const { return { lo_ ^ o.lo_, hi_ ^ o.hi_ }; }

  VecNx operator==(const VecNx& o) const { return { lo_ == o.lo_, hi_ == o.hi_ }; }
  VecNx operator!=(const VecNx& o) const { return { lo_ != o.lo_, hi_ != o.hi_ }; }
  VecNx operator< (const VecNx& o) const { return { lo_ <  o.lo_, hi_ <  o.hi_ }; }
  VecNx operator> (const VecNx& o) const { return { lo_ >  o.lo_, hi_ >  o.hi_ }; }
  VecNx operator<=(const VecNx& o) const { return { lo_ <= o.lo_, hi_ <= o.hi_ }; }
  VecNx operator>=(const VecNx& o) const { return { lo_ >= o.lo_, hi_ >= o.hi_ }; }

  T operator[](unsigned k) const {
    ASSERT(k < N);
    return k < N/2 ? lo_[k] : hi_[k-N/2];
  }

  Half lo_;
  Half hi_;
};

// Bottom out the default implementations with scalars when nothing's been specialized.
template<typename T>
struct VecNx<1,T> {
 private:
  typedef TMakeInteger<false, sizeof(T)> Bits;
 public:

  VecNx() = default;
  VecNx(T val) : val_(val) {}

  static VecNx load(const T* ptr) { return *ptr; }
  void store(T* ptr) const { *ptr = val_; }

  bool allTrue() const { return val_ != 0; }
  bool anyTrue() const { return val_ != 0; }

  VecNx mathAbs() const { return stp::mathAbs(val_); }
  VecNx reciprocal() const { return T(1) / val_; }
  VecNx mathSqrt() const { return mathSqrt(val_); }
  VecNx mathRsqrt() const { return VecNx(1) / mathSqrt(); }
  VecNx mathFloor() const { return mathFloor(val_); }

  static VecNx min(const VecNx& l, const VecNx& r) {
    return l.val_ < r.val_ ? l : r;
  }
  static VecNx max(const VecNx& l, const VecNx& r) {
    return l.val_ > r.val_ ? l : r;
  }
  static VecNx ternary(const VecNx& c, const VecNx& t, const VecNx& e) {
    return c.val_ != 0 ? t : e;
  }
  static VecNx saturatedAdd(const VecNx& x, const VecNx& y) {
    static_assert(TIsUnsigned<T>, "!");
    T sum = x.val_ + y.val_;
    return sum < x.val_ ? Limits<T>::Max : sum;
  }

  VecNx operator!() const { return !val_; }
  VecNx operator-() const { return -val_; }
  VecNx operator~() const { return fromBits(~toBits(val_)); }

  VecNx operator<<(int bits) const { return val_ << bits; }
  VecNx operator>>(int bits) const { return val_ >> bits; }

  VecNx operator+(const VecNx& o) const { return val_ + o.val_; }
  VecNx operator-(const VecNx& o) const { return val_ - o.val_; }
  VecNx operator*(const VecNx& o) const { return val_ * o.val_; }
  VecNx operator/(const VecNx& o) const { return val_ / o.val_; }

  VecNx operator&(const VecNx& y) const { return fromBits(toBits(val_) & toBits(y.val_)); }
  VecNx operator|(const VecNx& y) const { return fromBits(toBits(val_) | toBits(y.val_)); }
  VecNx operator^(const VecNx& y) const { return fromBits(toBits(val_) ^ toBits(y.val_)); }

  VecNx operator==(const VecNx& y) const { return conditionValue(val_ == y.val_); }
  VecNx operator!=(const VecNx& y) const { return conditionValue(val_ != y.val_); }
  VecNx operator<=(const VecNx& y) const { return conditionValue(val_ <= y.val_); }
  VecNx operator>=(const VecNx& y) const { return conditionValue(val_ >= y.val_); }
  VecNx operator< (const VecNx& y) const { return conditionValue(val_ <  y.val_); }
  VecNx operator> (const VecNx& y) const { return conditionValue(val_ >  y.val_); }

  T operator[](unsigned k) const {
    ASSERT(k == 0);
    return val_;
  }

  template<typename D>
  explicit operator VecNx<1,D>() const {
    return static_cast<D>(val_);
  }

  T val_;

 private:
  static VecNx conditionValue(bool b) {
    return fromBits(static_cast<Bits>(b ? -1 : 0));
  }

  static Bits toBits(T v) { return bitCast<Bits>(v); }
  static T fromBits(Bits bits) { return bitCast<T>(bits); }
};

template<typename D, typename S, unsigned N>
inline VecNx<N,D> vnxCast(const VecNx<N,S>& x) {
  return { vnxCast<D>(x.lo_), vnxCast<D>(x.hi_) };
}

template<typename D, typename S>
inline VecNx<1,D> vnxCast(const VecNx<1,S>& x) {
  return static_cast<D>(x.val_);
}

template<unsigned N, typename T>
static VecNx<N,T> mathAbs(const VecNx<N,T>& x) {
  return x.mathAbs();
}

// Returns minimum values for all components from |l| and |r|.
template<unsigned N, typename T>
static VecNx<N,T> min(const VecNx<N,T>& l, const VecNx<N,T>& r) {
  return VecNx<N,T>::min(l, r);
}

// Returns maximum values for all components from |l| and |r|.
template<unsigned N, typename T>
static VecNx<N,T> max(const VecNx<N,T>& l, const VecNx<N,T>& r) {
  return VecNx<N,T>::max(l, r);
}

class VnxMath {
  STATIC_ONLY(VnxMath);
 public:
  // Ternary operator for vectors.
  template<unsigned N, typename T>
  static VecNx<N,T> ternary(const VecNx<N,T>& cond, const VecNx<N,T>& then, const VecNx<N,T>& els) {
    return VecNx<N,T>::ternary(cond, then, els);
  }

  template<unsigned N, typename T>
  static VecNx<N,T> saturatedAdd(const VecNx<N,T>& l, const VecNx<N,T>& r) {
    return VecNx<N,T>::saturatedAdd(l, r);
  }

  template<unsigned N, typename T>
  static VecNx<N,T> reciprocal(const VecNx<N,T>& x) { return x.reciprocal(); }

  template<unsigned N, typename T>
  static VecNx<N,T> mathSqrt(const VecNx<N,T>& x) { return x.mathSqrt(); }

  template<unsigned N, typename T>
  static VecNx<N,T> mathRsqrt(const VecNx<N,T>& x) { return x.mathRsqrt(); }

  template<unsigned N, typename T>
  static VecNx<N,T> mathFloor(const VecNx<N,T>& x) { return x.mathFloor(); }

  // A very generic shuffle.  Can reorder, duplicate, contract, expand...
  //  Vec4f v = { R,G,B,A };
  //  Shuffle<2,1,0,3>(v)         ~~> {B,G,R,A}
  //  Shuffle<2,1>(v)             ~~> {B,G}
  //  Shuffle<2,1,2,1,2,1,2,1>(v) ~~> {B,G,B,G,B,G,B,G}
  //  Shuffle<3,3,3,3>(v)         ~~> {A,A,A,A}
  template<int... Ix, unsigned N, typename T>
  static VecNx<sizeof...(Ix),T> shuffle(const VecNx<N,T>& v) {
    return { v[Ix]... };
  }

  // VecNx<N,T> ~~> VecNx<N/2,T> + VecNx<N/2,T>
  template<unsigned N, typename T>
  static void split(const VecNx<N,T>& v, VecNx<N/2,T>* lo, VecNx<N/2,T>* hi) {
    *lo = v.lo_;
    *hi = v.hi_;
  }

  // VecNx<N/2,T> + VecNx<N/2,T> ~~> VecNx<N,T>
  template<unsigned N, typename T>
  static VecNx<N*2,T> join(const VecNx<N,T>& lo, const VecNx<N,T>& hi) {
    return { lo, hi };
  }
};

typedef VecNx< 2, float> Vec2f;
typedef VecNx< 4, float> Vec4f;
typedef VecNx< 8, float> Vec8f;
typedef VecNx<16, float> Vec16f;

typedef VecNx< 2, double> Vec2d;
typedef VecNx< 4, double> Vec4d;
typedef VecNx< 8, double> Vec8d;

typedef VecNx< 4, uint16_t> Vec4h;
typedef VecNx< 8, uint16_t> Vec8h;
typedef VecNx<16, uint16_t> Vec16h;

typedef VecNx< 4, uint8_t> Vec4b;
typedef VecNx< 8, uint8_t> Vec8b;
typedef VecNx<16, uint8_t> Vec16b;

typedef VecNx< 4,  int32_t> Vec4i;
typedef VecNx< 8,  int32_t> Vec8i;

typedef VecNx<4, uint32_t> Vec4u;

} // namespace stp

// Include platform specific specializations if available.
#if !defined(DISABLE_VNX_SIMD)
# if CPU_SIMD(SSE2)
#  include "Base/Simd/VnxSse.h"
# elif CPU_SIMD(NEON)
#  include "Base/Simd/VnxNeon.h"
# endif
#endif

#endif // STP_BASE_SIMD_VNX_H_
