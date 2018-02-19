// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Xform2.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Io/TextWriter.h"
#include "Base/Math/Abs.h"
#include "Base/Math/RawFloat.h"
#include "Base/Simd/Vnx.h"
#include "Geometry/Affine.h"
#include "Geometry/Angle.h"
#include "Geometry/Bounds2.h"
#include "Geometry/Quad2.h"

namespace stp {

static inline bool IsDegenerate2x2(float scale_x, float shear_x, float shear_y, float scale_y) {
  float perp_dot = scale_x * scale_y + shear_x * shear_y;
  return Abs(perp_dot) <= Limits<float>::Epsilon;
}

unsigned Xform2::GetTypeMaskSlow() const {
  if (d_[EntryPersp0] != 0 || d_[EntryPersp1] != 0 || d_[EntryLast] != 1) {
    // Once it is determined that that this is a perspective transform,
    // all other flags are moot as far as optimizations are concerned.
    type_mask_ = TypeMaskAll;
    return type_mask_;
  }

  unsigned mask = 0;

  if (d_[EntryTransX] != 0 || d_[EntryTransY] != 0)
    mask |= TypeMaskTranslate;

  unsigned m00 = RawFloat(d_[EntryScaleX]).ToBits();
  unsigned m01 = RawFloat(d_[EntryShearX]).ToBits();
  unsigned m10 = RawFloat(d_[EntryShearY]).ToBits();
  unsigned m11 = RawFloat(d_[EntryScaleY]).ToBits();

  if (m01 | m10) {
    // The skew components may be scale-inducing, unless we are dealing
    // with a pure rotation. Testing for a pure rotation is expensive,
    // so we opt for being conservative by always setting the scale bit.
    // along with affine.
    // By doing this, we are also ensuring that matrices have the same
    // type masks as their inverses.
    mask |= TypeMaskAffine | TypeMaskScale;

    // For RectStaysRect, in the affine case, we only need check that
    // the primary diagonal is all zeros and that the secondary diagonal
    // is all non-zero.

    // map non-zero to 1
    m01 = m01 != 0;
    m10 = m10 != 0;

    int dp0 = 0 == (m00 | m11) ; // true if both are 0
    int ds1 = m01 & m10; // true if both are 1

    mask |= (dp0 & ds1) * TypeMaskRectStaysRect;
  } else {
    // Only test for scale explicitly if not affine, since affine sets the scale bit.
    const uint32_t One = RawFloat(1.f).ToBits();

    if ((m00 ^ One) | (m11 ^ One))
      mask |= TypeMaskScale;

    // Not affine, therefore we already know secondary diagonal is
    // all zeros, so we just need to check that primary diagonal is
    // all non-zero.

    // map non-zero to 1
    m00 = m00 != 0;
    m11 = m11 != 0;

    // record if the (p)rimary diagonal is all non-zero
    mask |= (m00 & m11) * TypeMaskRectStaysRect;
  }
  type_mask_ = mask;
  return mask;
}

bool Xform2::IsSimilarity(float tolerance) const {
  unsigned transforms = GetTransforms();
  if (transforms <= TypeMaskTranslate)
    return true;
  if (transforms & TypeMaskPerspective)
    return false;

  float mx = d_[EntryScaleX];
  float my = d_[EntryScaleY];
  if (!(transforms & TypeMaskAffine)) {
    return !IsNear(mx, 0.f, NearlyZeroForGraphics<float>) &&
        IsNear(Abs(mx), Abs(my), NearlyZeroForGraphics<float>);
  }

  float sx = d_[EntryShearX];
  float sy = d_[EntryShearY];

  if (IsDegenerate2x2(mx, sx, sy, my))
    return false;

  // upper 2x2 is rotation/reflection + uniform scale if basis vectors
  // are 90 degree rotations of each other
  return (IsNear(mx, my, tolerance) && IsNear(sx, -sy, tolerance))
      || (IsNear(mx, -my, tolerance) && IsNear(sx, sy, tolerance));
}

bool Xform2::PreservesRightAngles(float tolerance) const {
  unsigned transforms = GetTransforms();

  if (transforms <= TypeMaskTranslate)
    return true;
  if (transforms & TypeMaskPerspective)
    return false;

  ASSERT(transforms & (TypeMaskAffine | TypeMaskScale));

  float mx = d_[EntryScaleX];
  float my = d_[EntryScaleY];
  float sx = d_[EntryShearX];
  float sy = d_[EntryShearY];

  if (IsDegenerate2x2(mx, sx, sy, my))
    return false;

  float dot = DotProduct(Vector2(mx, sy), Vector2(sx, my));
  return IsNear(dot, tolerance * tolerance, NearlyZeroForGraphics<float>);
}

bool isFinite(const Xform2& xform) {
  float accumulator = 0;
  for (int i = 0; i < Xform2::EntryCount; ++i)
    accumulator *= xform.get(i);
  return accumulator == 0;
}

void Xform2::SetTranslate(float dx, float dy) {
  SetIdentity();

  if (dx == 0 && dy == 0)
    return;

  d_[EntryTransX] = dx;
  d_[EntryTransY] = dy;

  type_mask_ = TypeMaskTranslate | TypeMaskRectStaysRect;
}

void Xform2::Translate(float dx, float dy) {
  if (dx == 0 && dy == 0)
    return;

  if (!HasPerspective()) {
    if (IsTranslate()) {
      d_[EntryTransX] += dx;
      d_[EntryTransY] += dy;
    } else {
      d_[EntryTransX] += d_[EntryScaleX] * dx + d_[EntryShearX] * dy;
      d_[EntryTransX] += d_[EntryShearY] * dx + d_[EntryScaleY] * dy;
    }
    FixTransBit();
  } else {
    Xform2 m(SkipInit);
    m.SetTranslate(dx, dy);
    Concat(m);
  }
}

void Xform2::PostTranslate(float dx, float dy) {
  if (dx == 0 && dy == 0)
    return;

  if (HasPerspective()) {
    Xform2 m(SkipInit);
    m.SetTranslate(dx, dy);
    PostConcat(m);
  } else {
    d_[EntryTransX] += dx;
    d_[EntryTransY] += dy;
    FixTransBit();
  }
}

void Xform2::FixTransBit() {
  if (GetTransInternal().IsZero())
    type_mask_ &= ~TypeMaskTranslate;
  else
    type_mask_ |= TypeMaskTranslate;
}

void Xform2::SetScale(float sx, float sy) {
  SetIdentity();

  if (sx == 1 && sy == 1)
    return;

  d_[EntryScaleX] = sx;
  d_[EntryScaleY] = sy;
  type_mask_ = TypeMaskScale | TypeMaskRectStaysRect;
}

void Xform2::Scale(float sx, float sy) {
  if (sx == 1 && sy == 1)
    return;

  d_[EntryScaleX] *= sx;
  d_[EntryShearY] *= sx;
  d_[EntryPersp0] *= sx;

  d_[EntryShearX] *= sy;
  d_[EntryScaleY] *= sy;
  d_[EntryPersp1] *= sy;

  if (d_[EntryScaleX] == 1 && d_[EntryScaleY] == 1
      && !(type_mask_ & (TypeMaskPerspective | TypeMaskAffine))) {
    type_mask_ &= ~TypeMaskScale;
  } else {
    type_mask_ |= TypeMaskScale;
  }
}

void Xform2::PostScale(float sx, float sy) {
  if (sx == 1 && sy == 1)
    return;

  Xform2 m(SkipInit);
  m.SetScale(sx, sy);
  PostConcat(m);
}

void Xform2::SetScale(float sx, float sy, float px, float py) {
  if (sx == 1 && sy == 1)
    SetIdentity();
  else
    SetScaleTranslate(sx, sy, px - sx * px, py - sy * py);
}

void Xform2::Scale(float sx, float sy, float px, float py) {
  if (sx == 1 && sy == 1)
    return;

  Xform2 m(SkipInit);
  m.SetScale(sx, sy, px, py);
  PostConcat(m);
}

void Xform2::PostScale(float sx, float sy, float px, float py) {
  if (sx == 1 && sy == 1)
    return;

  Xform2 m(SkipInit);
  m.SetScale(sx, sy, px, py);
  PostConcat(m);
}

bool Xform2::PostIntDiv(int divx, int divy) {
  if (divx == 0 || divy == 0)
    return false;

  const float inv_x = 1.f / divx;
  const float inv_y = 1.f / divy;

 d_[EntryScaleX] *= inv_x;
 d_[EntryShearX] *= inv_x;
 d_[EntryTransX] *= inv_x;

 d_[EntryScaleY] *= inv_y;
 d_[EntryShearY] *= inv_y;
 d_[EntryTransY] *= inv_y;

  type_mask_ = TypeMaskUnknown;
  return true;
}

void Xform2::SetScaleTranslate(float sx, float sy, float tx, float ty) {
  d_[EntryScaleX] = sx;
  d_[EntryShearX] = 0;
  d_[EntryTransX] = tx;

  d_[EntryShearY] = 0;
  d_[EntryScaleY] = sy;
  d_[EntryTransY] = ty;

  d_[EntryPersp0] = 0;
  d_[EntryPersp1] = 0;
  d_[EntryLast] = 1;

  type_mask_ = TypeMaskRectStaysRect;
  if (sx != 1 || sy != 1)
    type_mask_ |= TypeMaskScale;
  if (tx || ty)
    type_mask_ |= TypeMaskTranslate;
}

bool Xform2::SetBoundsToBounds(const Bounds2& src, const Bounds2& dst, ScaleToFit scale_to_fit) {
  if (src.isEmpty()) {
    SetIdentity();
    return false;
  }

  float sx = dst.GetWidth()  / src.GetWidth();
  float sy = dst.GetHeight() / src.GetHeight();

  bool x_larger = false;
  if (scale_to_fit != ScaleToFit::Fill) {
    if (sx > sy) {
      x_larger = true;
      sx = sy;
    } else {
      sy = sx;
    }
  }

  float tx = dst.min.x - src.min.x * sx;
  float ty = dst.min.y - src.min.y * sy;

  if (scale_to_fit == ScaleToFit::Center || scale_to_fit == ScaleToFit::End) {
    float diff;
    if (x_larger)
      diff = dst.GetWidth()  - src.GetWidth() * sy;
    else
      diff = dst.GetHeight() - src.GetHeight() * sy;

    if (scale_to_fit == ScaleToFit::Center)
      diff *= 0.5f;

    if (x_larger)
      tx += diff;
    else
      ty += diff;
  }
  SetScaleTranslate(sx, sy, tx, ty);
  return true;
}

void Xform2::SetRotate(double radians) {
  auto fun = SinCos(radians);
  SetSinCos(fun.sin, fun.cos);
}

void Xform2::SetRotate(double radians, float px, float py) {
  auto fun = SinCos(radians);
  SetSinCos(fun.sin, fun.cos, px, py);
}

void Xform2::Rotate(double radians) {
  Xform2 rot(Xform2::SkipInit);
  rot.SetRotate(radians);
  Concat(rot);
}

void Xform2::PostRotate(double radians) {
  Xform2 rot(Xform2::SkipInit);
  rot.SetRotate(radians);
  PostConcat(rot);
}

void Xform2::SetSinCos(float sin_value, float cos_value) {
  d_[EntryScaleX] = cos_value;
  d_[EntryShearX] = -sin_value;
  d_[EntryTransX] = 0;

  d_[EntryShearY] = sin_value;
  d_[EntryScaleY] = cos_value;
  d_[EntryTransY] = 0;

  d_[EntryPersp0] = d_[EntryPersp1] = 0;
  d_[EntryLast] = 1;

  type_mask_ = TypeMaskUnknown;
}

void Xform2::SetSinCos(float sin_value, float cos_value, float px, float py) {
  const float OneMinusCosV = 1 - cos_value;

  d_[EntryScaleX] = cos_value;
  d_[EntryShearX] = -sin_value;
  d_[EntryTransX] = sin_value * py + OneMinusCosV * px;

  d_[EntryShearY] = sin_value;
  d_[EntryScaleY] = cos_value;
  d_[EntryTransY] = -sin_value * px + OneMinusCosV * py;

  d_[EntryPersp0] = d_[EntryPersp1] = 0;
  d_[EntryLast] = 1;

  type_mask_ = TypeMaskUnknown;
}

void Xform2::SetShear(float kx, float ky) {
  d_[EntryScaleX] = 1;
  d_[EntryShearX] = kx;
  d_[EntryTransX] = 0;

  d_[EntryShearY] = ky;
  d_[EntryScaleY] = 1;
  d_[EntryTransY] = 0;

  d_[EntryPersp0] = d_[EntryPersp1] = 0;
  d_[EntryLast] = 1;

  type_mask_ = TypeMaskUnknown;
}

void Xform2::Shear(float kx, float ky) {
  Xform2 m(SkipInit);
  m.SetShear(kx, ky);
  Concat(m);
}

void Xform2::SetSkew(double ax, double ay) {
  SetShear(Tan(ax), Tan(ay));
}

void Xform2::Skew(double angle_x, double angle_y) {
  Shear(Tan(angle_x), Tan(angle_y));
}

void Xform2::SkewX(double angle) {
  Skew(angle, 0);
}

void Xform2::SkewY(double angle) {
  Skew(0, angle);
}

void Xform2::PostSkew(double ax, double ay) {
  Xform2 m(Xform2::SkipInit);
  m.SetSkew(ax, ay);
  PostConcat(m);
}

void Xform2::SetAffine(const Affine& affine) {
  if (affine.IsScaleTranslate()) {
    if (affine.IsTranslate()) {
      SetTranslate(
          affine.get(Affine::EntryTransX), affine.get(Affine::EntryTransY));
    } else {
      SetScaleTranslate(
          affine.get(Affine::EntryScaleX), affine.get(Affine::EntryScaleY),
          affine.get(Affine::EntryTransX), affine.get(Affine::EntryTransY));
    }
  } else {
    *this = Xform2(
        affine.get(Affine::EntryScaleX), affine.get(Affine::EntryShearY),
        affine.get(Affine::EntryShearX), affine.get(Affine::EntryScaleY),
        affine.get(Affine::EntryTransX), affine.get(Affine::EntryTransY));
  }
}

static inline float MulAddMul(float a, float b, float c, float d) {
  return static_cast<float>(static_cast<double>(a) * b + static_cast<double>(c) * d);
}

static inline float RowCol3(const float row[], const float col[]) {
    return row[0] * col[0] + row[1] * col[3] + row[2] * col[6];
}

void Xform2::SetConcat(const Xform2& lhs, const Xform2& rhs) {
  unsigned lhs_type = lhs.GetTransforms();
  unsigned rhs_type = rhs.GetTransforms();

  if (lhs_type == 0) {
    *this = rhs;
  } else if (rhs_type == 0) {
    *this = lhs;
  } else if (((lhs_type | rhs_type) & ~(TypeMaskTranslate | TypeMaskScale)) == 0) {
    SetScaleTranslate(
        lhs.d_[EntryScaleX] * rhs.d_[EntryScaleX],
        lhs.d_[EntryScaleY] * rhs.d_[EntryScaleY],
        lhs.d_[EntryScaleX] * rhs.d_[EntryTransX] + lhs.d_[EntryTransX],
        lhs.d_[EntryScaleY] * rhs.d_[EntryTransY] + lhs.d_[EntryTransY]);
  } else {
    Xform2 tmp(SkipInit);

    if ((lhs_type | rhs_type) & TypeMaskPerspective) {
      tmp.d_[EntryScaleX] = RowCol3(&lhs.d_[0], &rhs.d_[0]);
      tmp.d_[EntryShearX] = RowCol3(&lhs.d_[0], &rhs.d_[1]);
      tmp.d_[EntryTransX] = RowCol3(&lhs.d_[0], &rhs.d_[2]);
      tmp.d_[EntryShearY] = RowCol3(&lhs.d_[3], &rhs.d_[0]);
      tmp.d_[EntryScaleY] = RowCol3(&lhs.d_[3], &rhs.d_[1]);
      tmp.d_[EntryTransY] = RowCol3(&lhs.d_[3], &rhs.d_[2]);
      tmp.d_[EntryPersp0] = RowCol3(&lhs.d_[6], &rhs.d_[0]);
      tmp.d_[EntryPersp1] = RowCol3(&lhs.d_[6], &rhs.d_[1]);
      tmp.d_[EntryLast  ] = RowCol3(&lhs.d_[6], &rhs.d_[2]);
      tmp.type_mask_ = TypeMaskUnknown;
    } else {
      tmp.d_[EntryScaleX] = MulAddMul(
          lhs.d_[EntryScaleX], rhs.d_[EntryScaleX], lhs.d_[EntryShearX], rhs.d_[EntryShearY]);

      tmp.d_[EntryShearX] = MulAddMul(
          lhs.d_[EntryScaleX], rhs.d_[EntryShearX], lhs.d_[EntryShearX], rhs.d_[EntryScaleY]);

      tmp.d_[EntryTransX] = MulAddMul(
          lhs.d_[EntryScaleX], rhs.d_[EntryTransX], lhs.d_[EntryShearX], rhs.d_[EntryTransY]) + lhs.d_[EntryTransX];

      tmp.d_[EntryShearY] = MulAddMul(
          lhs.d_[EntryShearY], rhs.d_[EntryScaleX], lhs.d_[EntryScaleY], rhs.d_[EntryShearY]);

      tmp.d_[EntryScaleY] = MulAddMul(
          lhs.d_[EntryShearY], rhs.d_[EntryShearX], lhs.d_[EntryScaleY], rhs.d_[EntryScaleY]);

      tmp.d_[EntryTransY] = MulAddMul(
          lhs.d_[EntryShearY], rhs.d_[EntryTransX], lhs.d_[EntryScaleY], rhs.d_[EntryTransY]) + lhs.d_[EntryTransY];

      tmp.d_[EntryPersp0] = 0;
      tmp.d_[EntryPersp1] = 0;
      tmp.d_[EntryLast  ] = 1;
      tmp.type_mask_ = TypeMaskUnknown;
    }
    *this = tmp;
  }
}

void Xform2::Concat(const Xform2& other) {
  if (!other.IsIdentity())
    SetConcat(*this, other);
}

void Xform2::PostConcat(const Xform2& other) {
  if (!other.IsIdentity())
    SetConcat(other, *this);
}

static inline double FCross(float a, float b, float c, float d) {
  return a * b - c * d;
}

static inline double DCross(double a, double b, double c, double d) {
  return a * b - c * d;
}

double Xform2::GetDeterminant() const {
  double det;

  if (HasPerspective()) {
    det = d_[EntryScaleX] *
        DCross(d_[EntryScaleY], d_[EntryLast], d_[EntryTransY], d_[EntryPersp1]) + d_[EntryShearX]  *
        DCross(d_[EntryTransY], d_[EntryPersp0], d_[EntryShearY],  d_[EntryLast]) + d_[EntryTransX] *
        DCross(d_[EntryShearY],  d_[EntryPersp1], d_[EntryScaleY], d_[EntryPersp0]);
  } else {
    det = DCross(d_[EntryScaleX], d_[EntryScaleY], d_[EntryShearX], d_[EntryShearY]);
  }
  return det;
}

static inline float FCrossDScale(float a, float b, float c, float d, double scale) {
  return FCross(a, b, c, d) * scale;
}

static inline float DCrossDScale(double a, double b, double c, double d, double scale) {
  return DCross(a, b, c, d) * scale;
}

void Xform2::ComplexInverse(float dst[9], const float src[9], double inv_det, bool is_persp) {
  ASSERT(src != dst);

  if (is_persp) {
    dst[EntryScaleX] = FCrossDScale(src[EntryScaleY], src[EntryLast  ], src[EntryTransY], src[EntryPersp1], inv_det);
    dst[EntryShearX] = FCrossDScale(src[EntryTransX], src[EntryPersp1], src[EntryShearX], src[EntryLast  ], inv_det);
    dst[EntryTransX] = FCrossDScale(src[EntryShearX], src[EntryTransY], src[EntryTransX], src[EntryScaleY], inv_det);

    dst[EntryShearY] = FCrossDScale(src[EntryTransY], src[EntryPersp0], src[EntryShearY], src[EntryLast  ], inv_det);
    dst[EntryScaleY] = FCrossDScale(src[EntryScaleX], src[EntryLast  ], src[EntryTransX], src[EntryPersp0], inv_det);
    dst[EntryTransY] = FCrossDScale(src[EntryTransX], src[EntryShearY], src[EntryScaleX], src[EntryTransY], inv_det);

    dst[EntryPersp0] = FCrossDScale(src[EntryShearY], src[EntryPersp1], src[EntryScaleY], src[EntryPersp0], inv_det);
    dst[EntryPersp1] = FCrossDScale(src[EntryShearX], src[EntryPersp0], src[EntryScaleX], src[EntryPersp1], inv_det);
    dst[EntryLast  ] = FCrossDScale(src[EntryScaleX], src[EntryScaleY], src[EntryShearX], src[EntryShearY], inv_det);
  } else {
    dst[EntryScaleX] = src[EntryScaleY] * inv_det;
    dst[EntryShearX] = -src[EntryShearX] * inv_det;
    dst[EntryTransX] = DCrossDScale(src[EntryShearX], src[EntryTransY], src[EntryScaleY], src[EntryTransX], inv_det);

    dst[EntryShearY] = -src[EntryShearY] * inv_det;
    dst[EntryScaleY] = src[EntryScaleX] * inv_det;
    dst[EntryTransY] = DCrossDScale(src[EntryShearY], src[EntryTransX], src[EntryScaleX], src[EntryTransY], inv_det);

    dst[EntryPersp0] = 0;
    dst[EntryPersp1] = 0;
    dst[EntryLast  ] = 1;
  }
}

bool Xform2::GetInverted(Xform2& out) const {
  if (IsScaleTranslate()) {
    if (IsTranslate()) {
      if (IsIdentity())
        out.SetIdentity();
      else
        out.SetTranslate(-GetTransInternal());
      return true;
    }
    if (d_[EntryScaleX] == 0 || d_[EntryScaleY] == 0)
      return false;

    float inv_x = 1 / d_[EntryScaleX];
    float inv_y = 1 / d_[EntryScaleY];

    // Must be careful when writing to inv, since it may be the
    // same memory as this.

    Xform2 out(SkipInit);
    out.d_[EntryShearX] = out.d_[EntryShearY] = 0;
    out.d_[EntryPersp0] = out.d_[EntryPersp1] = 0;

    out.d_[EntryScaleX] = inv_x;
    out.d_[EntryScaleY] = inv_y;
    out.d_[EntryLast  ] = 1;
    out.d_[EntryTransX] = -d_[EntryTransX] * inv_x;
    out.d_[EntryTransY] = -d_[EntryTransY] * inv_y;

    out.type_mask_ = type_mask_;
    return true;
  }

  double det = GetDeterminant();

  constexpr double NearlyZero = NearlyZeroForGraphics<double>;
  constexpr double MinDet = NearlyZero * NearlyZero * NearlyZero;

  if (Abs(det) <= MinDet)
    return false;

  double inv_det = 1 / det;

  ComplexInverse(out.d_, d_, inv_det, HasPerspective());
  if (!isFinite(out))
    return false;

  out.type_mask_ = type_mask_;
  return true;
}

bool Xform2::IsInvertible() const {
  if (IsScaleTranslate()) {
    if (IsTranslate())
      return true;
    return d_[EntryScaleX] != 0 && d_[EntryScaleY] != 0;
  }

  double det = GetDeterminant();

  constexpr double NearlyZero = NearlyZeroForGraphics<double>;
  constexpr double MinDet = NearlyZero * NearlyZero * NearlyZero;
  return Abs(det) > MinDet;
}

Affine Xform2::GetFlattenedAsAffine() const {
  return Affine(
      d_[EntryScaleX], d_[EntryShearY],
      d_[EntryShearX], d_[EntryScaleY],
      d_[EntryTransX], d_[EntryTransY]);
}

Point2 Xform2::MapPoint(Point2 p) const {
  MapPoints(&p, &p, 1);
  return p;
}

void Xform2::MapPointsIdent(const Xform2& m, Point2 dst[], const Point2 src[], int count) {
  ASSERT(m.IsIdentity());

  if (dst != src && count > 0)
    copyObjectsNonOverlapping(dst, src, count);
}

void Xform2::MapPointsTrans(const Xform2& m, Point2 dst[], const Point2 src[], int count) {
  ASSERT(m.IsTranslate());

  if (count > 0) {
    float tx = m.d_[EntryTransX];
    float ty = m.d_[EntryTransY];

    if (count & 1) {
      *dst = *src + Vector2(tx, ty);
      src += 1;
      dst += 1;
    }
    count >>= 1;

    Vec4f trans4(tx, ty, tx, ty);
    if (count & 1) {
      (Vec4f::Load(src->AsFloats()) + trans4).Store(dst->AsFloats());
      src += 2;
      dst += 2;
    }
    count >>= 1;

    for (int i = 0; i < count; ++i) {
      (Vec4f::Load((src + 0)->AsFloats()) + trans4).Store((dst + 0)->AsFloats());
      (Vec4f::Load((src + 2)->AsFloats()) + trans4).Store((dst + 2)->AsFloats());
      src += 4;
      dst += 4;
    }
  }
}

void Xform2::MapPointsScale(const Xform2& m, Point2 dst[], const Point2 src[], int count) {
  ASSERT(m.IsScaleTranslate());

  if (count > 0) {
    float tx = m.d_[EntryTransX];
    float ty = m.d_[EntryTransY];
    float sx = m.d_[EntryScaleX];
    float sy = m.d_[EntryScaleY];

    if (count & 1) {
      *dst = *src + Vector2(tx, ty);
      src += 1;
      dst += 1;
    }
    count >>= 1;

    Vec4f trans4(tx, ty, tx, ty);
    Vec4f scale4(sx, sy, sx, sy);
    if (count & 1) {
      (Vec4f::Load(src->AsFloats()) * scale4 + trans4).Store(dst->AsFloats());
      src += 2;
      dst += 2;
    }
    count >>= 1;

    for (int i = 0; i < count; ++i) {
      (Vec4f::Load((src + 0)->AsFloats()) * scale4 + trans4).Store((dst + 0)->AsFloats());
      (Vec4f::Load((src + 2)->AsFloats()) * scale4 + trans4).Store((dst + 2)->AsFloats());
      src += 4;
      dst += 4;
    }
  }
}

void Xform2::MapPointsAffin(const Xform2& m, Point2 dst[], const Point2 src[], int count) {
  ASSERT(!m.IsScaleTranslate() && !m.HasPerspective());

  if (count > 0) {
    float tx = m.d_[EntryTransX];
    float ty = m.d_[EntryTransY];
    float sx = m.d_[EntryScaleX];
    float sy = m.d_[EntryScaleY];
    float kx = m.d_[EntryShearX];
    float ky = m.d_[EntryShearY];

    if (count & 1) {
      *dst = Point2(
          src->x * sx + src->y * kx + tx,
          src->x * ky + src->y * sy + ty);
      src += 1;
      dst += 1;
    }
    count >>= 1;

    Vec4f trans4(tx, ty, tx, ty);
    Vec4f scale4(sx, sy, sx, sy);
    Vec4f  skew4(kx, ky, kx, ky); // applied to swizzle of src4
    for (int i = 0; i < count; ++i) {
      Vec4f src4 = Vec4f::Load(src->AsFloats());
      Vec4f swz4 = VnxMath::Shuffle<1,0,3,2>(src4); // y0 x0, y1 x1
      (src4 * scale4 + swz4 * skew4 + trans4).Store(dst->AsFloats());
      src += 2;
      dst += 2;
    }
  }
}

static inline float FDot(float a, float b, float c, float d) {
  return a * b + c * d;
}

void Xform2::MapPointsPersp(const Xform2& m, Point2 dst[], const Point2 src[], int count) {
  ASSERT(m.HasPerspective());

  if (count > 0) {
    do {
      float sx = src->x;
      float sy = src->y;
      src += 1;

      float x = FDot(sx, m.d_[EntryScaleX], sy, m.d_[EntryShearX]) + m.d_[EntryTransX];
      float y = FDot(sx, m.d_[EntryShearY], sy, m.d_[EntryScaleY]) + m.d_[EntryTransY];
      float z = FDot(sx, m.d_[EntryPersp0], sy, m.d_[EntryPersp1]) + m.d_[EntryLast];

      if (z != 0)
        z = 1 / z;

      *dst = Point2(x * z, y * z);
      dst += 1;
    } while (--count);
  }
}

const Xform2::MapPointsFunction Xform2::MapPointsFunctions_[16] = {
  &MapPointsIdent,
  &MapPointsTrans,
  &MapPointsScale, &MapPointsScale,
  &MapPointsAffin, &MapPointsAffin, &MapPointsAffin, &MapPointsAffin,
  &MapPointsPersp, &MapPointsPersp, &MapPointsPersp, &MapPointsPersp,
  &MapPointsPersp, &MapPointsPersp, &MapPointsPersp, &MapPointsPersp,
};

void Xform2::MapPoints(Point2 dst[], const Point2 src[], int count) const {
  ASSERT((dst && src && count > 0) || count == 0);
  ASSERT(src == dst || dst + count <= src || src + count <= dst, "partial overlap not supported");

  auto function = GetMapPointsFunction();
  function(*this, dst, src, count);
}

void Xform2::MapXYIdent(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(m.IsIdentity());

  dst[0] += src[0];
  dst[1] += src[1];
}

void Xform2::MapXYTrans(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(m.IsTranslate());

  dst[0] += src[0] + m.d_[EntryTransX];
  dst[1] += src[1] + m.d_[EntryTransY];
}

void Xform2::MapXYScale(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(m.IsScale());

  dst[0] = src[0] * m.d_[EntryScaleX];
  dst[1] = src[1] * m.d_[EntryScaleY];
}

void Xform2::MapXYScaTr(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(m.IsScaleTranslate());

  dst[0] = src[0] * m.d_[EntryScaleX] + m.d_[EntryTransX];
  dst[1] = src[1] * m.d_[EntryScaleY] + m.d_[EntryTransY];
}

void Xform2::MapXYAffin(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(!m.IsScaleTranslate() && !m.HasPerspective());

  dst[0] = src[0] * m.d_[EntryScaleX] + (src[1] * m.d_[EntryShearX] + m.d_[EntryTransX]);
  dst[1] = src[0] * m.d_[EntryShearY] + (src[1] * m.d_[EntryScaleY] + m.d_[EntryTransY]);
}

void Xform2::MapXYPersp(const Xform2& m, float dst[2], const float src[2]) {
  ASSERT(m.HasPerspective());

  float sx = src[0];
  float sy = src[1];

  float x = FDot(sx, m.d_[EntryScaleX], sy, m.d_[EntryShearX]) + m.d_[EntryTransX];
  float y = FDot(sx, m.d_[EntryShearY], sy, m.d_[EntryScaleY]) + m.d_[EntryTransY];
  float z = FDot(sx, m.d_[EntryPersp0], sy, m.d_[EntryPersp1]) + m.d_[EntryLast];

  if (z != 0)
    z = 1 / z;

  dst[0] = x * z;
  dst[1] = y * z;
}

const Xform2::MapXYFunction Xform2::MapXYFunctions_[16] = {
  &MapXYIdent,
  &MapXYTrans,
  &MapXYScale,
  &MapXYScaTr,
  &MapXYAffin, &MapXYAffin, &MapXYAffin, &MapXYAffin,
  &MapXYPersp, &MapXYPersp, &MapXYPersp, &MapXYPersp,
  &MapXYPersp, &MapXYPersp, &MapXYPersp, &MapXYPersp,
};

Quad2 Xform2::MapQuad(const Quad2& quad) const {
  Quad2 result;
  MapPoints(result.p, quad.p, 4);
  return result;
}

Vector2 Xform2::MapVector(Vector2 v) const {
  MapVectors(&v, &v, 1);
  return v;
}

void Xform2::MapVectors(Vector2 dst[], const Vector2 src[], int count) const {
  if (HasPerspective()) {
    const float zero[2] = { 0.f, 0.f };
    float origin[2];

    auto function = GetMapXYFunction();
    function(*this, origin, zero);

    for (int i = count - 1; i >= 0; --i) {
      float tmp[2];
      function(*this, tmp, (src + i)->AsFloats());
      dst[i] = Vector2(tmp[0] - origin[0], tmp[1] - origin[1]);
    }
  } else {
    Xform2 tmp = *this;
    tmp.d_[EntryTransX] = 0;
    tmp.d_[EntryTransY] = 0;

    tmp.type_mask_ &= ~TypeMaskTranslate;

    static_assert(sizeof(Point2) == sizeof(Vector2), "layout compatible");
    tmp.MapPoints(reinterpret_cast<Point2*>(dst), reinterpret_cast<const Point2*>(src), count);
  }
}

bool IsNear(const Xform2& lhs, const Xform2& rhs, float tolerance) {
  for (int i = 0; i < Xform2::EntryCount; ++i) {
    if (!IsNear(lhs.get(i), rhs.get(i), tolerance))
      return false;
  }
  return true;
}

bool Xform2::operator==(const Xform2& other) const {
  for (int i = 0; i < EntryCount; ++i) {
    if (d_[i] != other.d_[i])
      return false;
  }
  return true;
}

void Xform2::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.Write('[');
  for (int y = 0; y < 3; ++y) {
    if (y != 0)
      out.Write(' ');
    for (int x = 0; x < 3; ++x)
      out.WriteFloat(Get(y * 3 + x));
  }
  out.Write(']');
}

} // namespace stp
