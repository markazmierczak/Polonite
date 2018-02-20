// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Affine.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Io/TextWriter.h"
#include "Base/Math/RawFloat.h"
#include "Base/Simd/Vnx.h"
#include "Geometry/Angle.h"
#include "Geometry/Bounds2.h"
#include "Geometry/Line2.h"
#include "Geometry/Quad2.h"
#include "Geometry/Vector2.h"

namespace stp {

unsigned Affine::GetTypeMaskSlow() const {
  unsigned mask = 0;

  if (d_[EntryTransX] != 0 || d_[EntryTransY] != 0)
    mask |= TypeMaskTranslate;

  unsigned m00 = RawFloat(d_[EntryScaleX]).ToBits();
  unsigned m01 = RawFloat(d_[EntryShearX]).ToBits();
  unsigned m10 = RawFloat(d_[EntryShearY]).ToBits();
  unsigned m11 = RawFloat(d_[EntryScaleY]).ToBits();

  if (m01 | m10) {
    // The skew components may be scale-inducing, unless we are dealing
    // with a pure rotation.  Testing for a pure rotation is expensive,
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

    uint32_t dp0 = 0 == (m00 | m11) ; // true if both are 0
    uint32_t ds1 = m01 & m10; // true if both are 1

    if (dp0 & ds1)
      mask |= TypeMaskRectStaysRect;
  } else {
    // Only test for scale explicitly if not affine, since affine sets the
    // scale bit.
    const uint32_t One = RawFloat(1.f).ToBits();
    if ((m00 ^ One) | (m11 ^ One))
      mask |= TypeMaskScale;

    // Not affine, therefore we already know secondary diagonal is
    // all zeros, so we just need to check that primary diagonal is
    // all non-zero.

    // map non-zero to 1
    m00 = m00 != 0;
    m11 = m11 != 0;

    // Record if the (p)rimary diagonal is all non-zero.
    if (m00 & m11)
      mask |= TypeMaskRectStaysRect;
  }
  type_mask_ = mask;
  return mask;
}

void Affine::FixTransBit() {
  if (GetTransInternal().IsZero())
    type_mask_ &= ~TypeMaskTranslate;
  else
    type_mask_ |= TypeMaskTranslate;
}

void Affine::FixScaleBit() {
  if (d_[EntryScaleX] != 1 || d_[EntryScaleY] != 1 || (type_mask_ & TypeMaskAffine) != 0)
    type_mask_ |= TypeMaskScale;
  else
    type_mask_ &= ~TypeMaskScale;
}

bool Affine::IsSimilarity(float tolerance) const {
  float sx = d_[EntryScaleX];
  float sy = d_[EntryScaleY];

  if (IsScaleTranslate()) {
    if (IsTranslate())
      return true;

    // If no skew, can just compare scale factors.
    return !isNear(sx, 0.f, tolerance) && isNear(sy, 0.f, tolerance);
  }

  float kx = d_[EntryShearX];
  float ky = d_[EntryShearY];

  if (!IsInvertible())
    return false;

  // Upper 2x2 is rotation/reflection + uniform scale if basis vectors
  // are 90 degree rotations of each other.
  return (isNear(sx, sy, tolerance) && isNear(kx, -ky, tolerance))
      || (isNear(sx, -sy, tolerance) && isNear(kx, ky, tolerance));
}

bool Affine::PreservesRightAngles(float tolerance) const {
  if (IsTranslate())
    return true;

  float sx = d_[EntryScaleX];
  float sy = d_[EntryScaleY];
  float kx = d_[EntryShearX];
  float ky = d_[EntryShearY];

  if (!IsInvertible())
    return false;

  // Upper 2x2 is scale + rotation/reflection if basis vectors are orthogonal.
  return isNear(0.f, DotProduct(Vector2(sx, ky), Vector2(kx, sy)), tolerance);
}

Affine Affine::MakeTranslate(float tx, float ty) {
  Affine m(SkipInit);
  m.SetTranslate(tx, ty);
  return m;
}

void Affine::SetTranslate(float dx, float dy) {
  SetIdentity();

  if (dx == 0 && dy == 0)
    return;

  d_[EntryTransX] = dx;
  d_[EntryTransY] = dy;
  type_mask_ = TypeMaskTranslate | TypeMaskRectStaysRect;
}

void Affine::Translate(Vector2 translation) {
  if (translation.IsZero())
    return;

  Vector2 new_trans = MapPoint(translation);
  SetTransInternal(new_trans);
  FixTransBit();
}

void Affine::Translate(float dx, float dy) {
  Translate(Vector2(dx, dy));
}

void Affine::PostTranslate(Vector2 translation) {
  if (translation.IsZero())
    return;

  SetTransInternal(GetTransInternal() + translation);

  if (!(type_mask_ & TypeMaskUnknown))
    FixTransBit();
}

void Affine::PostTranslate(float dx, float dy) {
  PostTranslate(Vector2(dx, dy));
}

Affine Affine::MakeScale(float sx, float sy) {
  Affine m(SkipInit);
  m.SetScale(sx, sy);
  return m;
}

void Affine::SetScale(float sx, float sy, Point2 pivot) {
  SetScale(sx, sy, pivot.x, pivot.y);
}

void Affine::SetScale(float sx, float sy, float px, float py) {
  if (sx == 1 && sy == 1) {
    SetIdentity();
    return;
  }
  SetScaleTranslate(sx, sy, px - sx * px, py - sy * py);
}

void Affine::SetScale(float sx, float sy) {
  if (sx == 1 && sy == 1) {
    SetIdentity();
    return;
  }
  *this = Affine(sx, 0, 0, sy, 0, 0);
  type_mask_ = TypeMaskScale;

  if (sx != 0 && sy != 0)
    type_mask_ |= TypeMaskRectStaysRect;
}

void Affine::SetScale(float scale) {
  SetScale(scale, scale);
}

void Affine::Scale(float sx, float sy) {
  if (sx == 1 && sy == 1)
    return;

  // the assumption is that these multiplies are very cheap, and that
  // a full concat and/or just computing the matrix type is more expensive.

  d_[EntryScaleX] *= sx;
  d_[EntryShearX] *= sy;
  d_[EntryShearY] *= sx;
  d_[EntryScaleY] *= sy;

  // The affine preconditions are in place to keep the mask consistent with
  // what ComputeTypes() would produce (skew always implies kScale).
  // We should investigate whether these flag dependencies are truly needed.
  if (!(type_mask_ & TypeMaskUnknown))
    FixScaleBit();
  if (sx == 0 || sy == 0)
    type_mask_ &= ~TypeMaskRectStaysRect;
}

void Affine::Scale(float s) {
  Scale(s, s);
}

void Affine::FlipX() { Scale(-1, 1); }
void Affine::FlipY() { Scale(1, -1); }

void Affine::PostScale(float sx, float sy) {
  if (sx == 1 && sy == 1)
    return;
  d_[EntryScaleX] *= sx;
  d_[EntryShearX] *= sx;
  d_[EntryShearY] *= sy;
  d_[EntryScaleY] *= sy;
  SetTransInternal(GetTransInternal().GetScaled(sx, sy));
  InvalidateTypes();
}

bool Affine::PostIntDiv(int divx, int divy) {
  if (divx == 0 || divy == 0)
    return false;
  PostScale(1.f / divx, 1.f / divy);
  return true;
}

void Affine::SetScaleTranslate(float sx, float sy, Vector2 translation) {
  SetScaleTranslate(sx, sy, translation.x, translation.y);
}

void Affine::SetScaleTranslate(float sx, float sy, float tx, float ty) {
  *this = Affine(sx, 0, 0, sy, tx, ty);

  int mask = 0;
  if (sx != 1 || sy != 1)
    mask |= TypeMaskScale;
  if (tx != 0 || ty != 0)
    mask |= TypeMaskTranslate;
  if (sx != 0 && sy != 0)
    mask |= TypeMaskRectStaysRect;
  type_mask_ = mask;
}

void Affine::SetRotate(double radians, Point2 pivot) {
  auto fun = SinCos(radians);
  SetSinCos(fun.sin, fun.cos, pivot);
}

void Affine::SetRotate(double radians, float px, float py) {
  SetRotate(radians, Point2(px, py));
}

void Affine::SetRotate(double radians) {
  auto fun = SinCos(radians);
  SetSinCos(fun.sin, fun.cos);
}

void Affine::SetSinCos(float sin_value, float cos_value, Point2 pivot) {
  const float one_minus_cos_v = 1 - cos_value;

  float tx = DotProduct(Vector2(one_minus_cos_v, sin_value), pivot);
  float ty = DotProduct(Vector2(-sin_value, one_minus_cos_v), pivot);

  *this = Affine(cos_value, sin_value, -sin_value, cos_value, tx, ty);
}

void Affine::SetSinCos(float sin_value, float cos_value, float pivot_x, float pivot_y) {
  SetSinCos(sin_value, cos_value, Point2(pivot_x, pivot_y));
}

void Affine::SetSinCos(float sin_value, float cos_value) {
  *this = Affine(cos_value, sin_value, -sin_value, cos_value, 0, 0);
}

bool Affine::SetBoundsToBounds(const Bounds2& src, const Bounds2& dst, ScaleToFit scale_to_fit) {
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

void Affine::Rotate(double radians) {
  if (radians == 0)
    return;

  float sin_angle = Sin(radians);
  float cos_angle = Cos(radians);

  if (IsTranslate()) {
    // Counter-clockwise rotation matrix.
    *this = Affine(
        cos_angle, sin_angle,
        -sin_angle, cos_angle,
        d_[EntryTransX], d_[EntryTransY]);
  } else {
    Affine r(Affine::SkipInit);
    r.SetRotate(radians);
    Concat(r);
  }
}

void Affine::PostRotate(double radians) {
  if (radians == 0)
    return;

  Affine m(Affine::SkipInit);
  m.SetRotate(radians);
  PostConcat(m);
}

void Affine::SetSkew(double ax, double ay, Point2 pivot) {
  float kx = Tan(ax);
  float ky = Tan(ay);
  float tx = -kx * pivot.y;
  float ty = -ky * pivot.x;
  *this = Affine(1, ky, kx, 1, tx, ty);
}

void Affine::SetSkew(double ax, double ay) {
  float kx = Tan(ax);
  float ky = Tan(ay);
  SetShear(kx, ky);
}

void Affine::SetShear(float kx, float ky) {
  *this = Affine(1, ky, kx, 1, 0, 0);
}

void Affine::Shear(float kx, float ky) {
  if (kx == 0 && ky == 0)
    return;

  if (IsScaleTranslate()) {
      d_[EntryShearX] = d_[EntryScaleX] * kx;
      d_[EntryShearY] = d_[EntryScaleY] * ky;
  } else {
    *this = Affine(
        d_[EntryScaleX] + d_[EntryShearX] * ky, d_[EntryShearY] + d_[EntryScaleY] * ky,
        d_[EntryShearX] + d_[EntryScaleX] * kx, d_[EntryScaleY] + d_[EntryShearY] * kx,
        d_[EntryTransX], d_[EntryTransY]);
  }
  InvalidateTypes();
}

void Affine::Skew(double radians_x, double radians_y) {
  Shear(Tan(radians_x), Tan(radians_y));
}

void Affine::SkewX(double radians) {
  Skew(radians, 0);
}

void Affine::SkewY(double radians) {
  Skew(0, radians);
}

void Affine::PostSkew(double ax, double ay) {
  Affine m(Affine::SkipInit);
  m.SetSkew(ax, ay);
  PostConcat(m);
}

static inline float MulAddMul(float a, float b, float c, float d) {
  return static_cast<float>(static_cast<double>(a) * b + static_cast<double>(c) * d);
}

void Affine::SetConcat(const Affine& lhs, const Affine& rhs) {
  unsigned transforms = (lhs.GetTypeMask() | rhs.GetTypeMask()) & TypeMaskAll;
  if (TransformsAre(transforms, TypeMaskTranslate | TypeMaskScale)) {
    if (TransformsAre(transforms, TypeMaskTranslate)) {
      if (transforms == 0)
        SetIdentity();
      else
        SetTranslate(lhs.GetTransInternal() + rhs.GetTransInternal());
    } else {
      SetScaleTranslate(
          lhs.d_[EntryScaleX] * rhs.d_[EntryScaleX],
          lhs.d_[EntryScaleY] * rhs.d_[EntryScaleY],
          lhs.d_[EntryScaleX] * rhs.d_[EntryTransX] + lhs.d_[EntryTransX],
          lhs.d_[EntryScaleY] * rhs.d_[EntryTransY] + lhs.d_[EntryTransY]);
    }
  } else {
    Affine tmp(SkipInit);
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

    *this = tmp;
    InvalidateTypes();
  }
}

void Affine::Concat(const Affine& other) {
  // Check for identity first, so we don't do a needless copy of ourselves
  // to ourselves inside SetConcat().
  if (!other.IsIdentity())
    SetConcat(*this, other);
}

void Affine::PostConcat(const Affine& other) {
  // Check for identity first, so we don't do a needless copy of ourselves
  // to ourselves inside SetConcat().
  if (!other.IsIdentity())
    SetConcat(other, *this);
}

namespace {

static_assert(sizeof(Point2) == 2 * sizeof(float), "!");

inline Vec4f Vec4fLoadPoints(const Point2* points) {
  // reinterpret_cast to AliasedType of first member is allowed.
  return Vec4f::Load(reinterpret_cast<const float*>(points));
}

inline void Vec4fStorePoints(const Vec4f& vec, Point2* points) {
  vec.Store(reinterpret_cast<float*>(points));
}

void MapPointsOptIdentity(const Affine& m, Point2* dst, const Point2* src, int count) {
  ASSERT(m.IsIdentity());
  if (dst != src && count)
    copyObjectsNonOverlapping(dst, src, count);
}

void MapPointsOptTranslate(const Affine& m, Point2* dst, const Point2* src, int count) {
  ASSERT(m.IsTranslate());
  float tx = m.get(Affine::EntryTransX);
  float ty = m.get(Affine::EntryTransY);
  if (count & 1) {
    *dst = *src + Vector2(tx, ty);
    src += 1;
    dst += 1;
  }
  Vec4f trans4(tx, ty, tx, ty);
  count >>= 1;
  if (count & 1) {
    Vec4fStorePoints(Vec4fLoadPoints(src) + trans4, dst);
    src += 2;
    dst += 2;
  }
  count >>= 1;
  for (int i = 0; i < count; ++i) {
    Vec4fStorePoints(Vec4fLoadPoints(src+0) + trans4, dst+0);
    Vec4fStorePoints(Vec4fLoadPoints(src+2) + trans4, dst+2);
    src += 4;
    dst += 4;
  }
}

void MapPointsOptScale(const Affine& m, Point2* dst, const Point2* src, int count) {
  ASSERT(m.IsScaleTranslate());
  float tx = m.get(Affine::EntryTransX);
  float ty = m.get(Affine::EntryTransY);
  float sx = m.get(Affine::EntryScaleX);
  float sy = m.get(Affine::EntryScaleY);
  if (count & 1) {
    *dst = Point2(src->x * sx + tx, src->y * sy + ty);
    src += 1;
    dst += 1;
  }
  Vec4f trans4(tx, ty, tx, ty);
  Vec4f scale4(sx, sy, sx, sy);
  count >>= 1;
  if (count & 1) {
    Vec4fStorePoints(Vec4fLoadPoints(src) * scale4 + trans4, dst);
    src += 2;
    dst += 2;
  }
  count >>= 1;
  for (int i = 0; i < count; ++i) {
    Vec4fStorePoints(Vec4fLoadPoints(src+0) * scale4 + trans4, dst+0);
    Vec4fStorePoints(Vec4fLoadPoints(src+2) * scale4 + trans4, dst+2);
    src += 4;
    dst += 4;
  }
}

void MapPointsOptAffine(const Affine& m, Point2* dst, const Point2* src, int count) {
  float tx = m.get(Affine::EntryTransX);
  float ty = m.get(Affine::EntryTransY);
  float sx = m.get(Affine::EntryScaleX);
  float sy = m.get(Affine::EntryScaleY);
  float kx = m.get(Affine::EntryShearX);
  float ky = m.get(Affine::EntryShearY);
  if (count & 1) {
    *dst = Point2(
        src->x * sx + src->y * kx + tx,
        src->x * ky + src->y * sy + ty);
    src += 1;
    dst += 1;
  }
  Vec4f trans4(tx, ty, tx, ty);
  Vec4f scale4(sx, sy, sx, sy);
  Vec4f share4(kx, ky, kx, ky); // Applied to swizzle of src4
  count >>= 1;
  for (int i = 0; i < count; ++i) {
    Vec4f src4 = Vec4fLoadPoints(src);
    Vec4f swz4 = VnxMath::Shuffle<1,0,3,2>(src4);  // y0 x0, y1 x1
    Vec4fStorePoints(src4 * scale4 + swz4 * share4 + trans4, dst);
    src += 2;
    dst += 2;
  }
}

} // namespace

const Affine::MapPointsProc Affine::MapPointsProcs[] = {
  MapPointsOptIdentity,
  MapPointsOptTranslate,
  MapPointsOptScale,
  MapPointsOptScale,
  MapPointsOptAffine,
  MapPointsOptAffine,
  MapPointsOptAffine,
  MapPointsOptAffine,
};

Point2 Affine::MapPoint(Point2 p) const {
  if (IsScaleTranslate()) {
    if (IsTranslate())
      return p + GetTransInternal();
    return p.GetScaled(d_[EntryScaleX], d_[EntryScaleY]) + GetTransInternal();
  }
  return Point2(
      d_[EntryScaleX] * p.x + d_[EntryShearX] * p.y + d_[EntryTransX],
      d_[EntryShearY] * p.x + d_[EntryScaleY] * p.y + d_[EntryTransY]);
}

void Affine::MapPoints(Point2* dst, const Point2* src, int count) const {
  ASSERT((dst && src && count > 0) || 0 == count);
  // No partial overlap.
  ASSERT(src == dst || &dst[count] <= &src[0] || &src[count] <= &dst[0]);
  GetMapPointsProc()(*this, dst, src, count);
}

Bounds2 Affine::MapBounds(const Bounds2& bounds) const {
  if (Preserves2DAxisAlignment()) {
    if (IsTranslate()) {
      if (IsIdentity())
        return bounds;
      return bounds + GetTransInternal();
    }
    Point2 points[2] = { bounds.min, bounds.max };
    MapPoints(points, points, 2);
    Bounds2 result = { points[0], points[1] };
    result.Sort();
    return result;;
  }
  return MapBoundsAsQuad(bounds).GetBounds();
}

Quad2 Affine::MapBoundsAsQuad(const Bounds2& b) const {
  Point2 points[4] = {
    { b.min.x, b.min.y},
    { b.max.x, b.min.y},
    { b.max.x, b.max.y},
    { b.min.x, b.max.y},
  };
  Quad2 result;
  MapPoints(result.p, points, 4);
  return result;
}

double Affine::GetDeterminant() const {
  return static_cast<double>(d_[EntryScaleX]) * d_[EntryScaleY] -
      static_cast<double>(d_[EntryShearX]) * d_[EntryShearY];
}

bool Affine::IsInvertible() const {
  return GetDeterminant() != 0;
}

bool Affine::GetInverted(Affine& out) const {
  int transforms = GetTransforms();
  if (IsTranslate()) {
    out.SetTranslate(-GetTransInternal());
  } else if (IsScaleTranslate()) {
    if (Abs(d_[EntryScaleX]) <= FLT_EPSILON || Abs(d_[EntryScaleY]) <= FLT_EPSILON)
      return false;

    float inv_sx = 1.f / d_[EntryScaleX];
    float inv_sy = 1.f / d_[EntryScaleY];
    Vector2 scaled_trans = (-GetTransInternal()).GetScaled(inv_sx, inv_sy);
    out = Affine(
        inv_sx, 0,
        0, inv_sy,
        scaled_trans.x, scaled_trans.y);
    out.type_mask_ = transforms | TypeMaskRectStaysRect;
  } else {
    float determinant = GetDeterminant();
    if (Abs(determinant) <= FLT_EPSILON) {
      // Singular matrix.
      return false;
    }
    float inverse_determinant = 1 / determinant;
    float tx = d_[EntryShearX] * d_[EntryTransY] - d_[EntryScaleY] * d_[EntryTransX];
    float ty = d_[EntryShearY] * d_[EntryTransX] - d_[EntryScaleX] * d_[EntryTransY];
    out = Affine(
        d_[EntryScaleY] * inverse_determinant, -d_[EntryShearY] * inverse_determinant,
        -d_[EntryShearX] * inverse_determinant, d_[EntryScaleX] * inverse_determinant,
        tx * inverse_determinant, ty * inverse_determinant);
  }
  return true;
}

bool Affine::Decompose(DecomposedAffine& out) const {
  if (IsTranslate()) {
    out.SetTranslate(GetTransInternal());
  } else {
    Affine m = *this;
    // Compute scaling factors.
    float sx = DecomposeScaleMagX();
    float sy = DecomposeScaleMagY();
    // Compute cross product of transformed unit vectors. If negative,
    // one axis was flipped.
    if (m.d_[EntryScaleX] * m.d_[EntryScaleY] -
        m.d_[EntryShearX] * m.d_[EntryShearY] < 0) {
      // Flip axis with minimum unit vector dot product
      if (m.d_[EntryScaleX] < m.d_[EntryScaleY])
        sx = -sx;
      else
        sy = -sy;
    }
    if (Abs(sx) <= FLT_EPSILON || Abs(sy) <= FLT_EPSILON)
      return false;
    // Remove scale from matrix
    m.Scale(1 / sx, 1 / sy);

    double angle = Atan2(
        static_cast<double>(m.d_[EntryShearY]),
        static_cast<double>(m.d_[EntryScaleX]));

    // Remove rotation from result matrix.
    m.Rotate(-angle);

    out.delta = m.GetTransInternal();
    out.scale_x = sx;
    out.scale_y = sy;
    out.angle_radians = angle;
    for (int i = 0; i < 4; ++i)
      out.remainder[i] = m.d_[i];
  }
  return true;
}

static inline float GetScale(float s0, float s1) {
  return Sqrt(static_cast<double>(s0) * s0 + static_cast<double>(s1) * s1);
}

float Affine::DecomposeScaleMagX() const {
  if (IsTranslate())
    return 1;
  return GetScale(d_[EntryScaleX], d_[EntryShearY]);
}

float Affine::DecomposeScaleMagY() const {
  if (IsTranslate())
    return 1;
  return GetScale(d_[EntryShearX], d_[EntryScaleY]);
}

void Affine::Recompose(const DecomposedAffine& decomposed) {
  *this = Affine(
      decomposed.remainder[EntryScaleX], decomposed.remainder[EntryShearY],
      decomposed.remainder[EntryShearX], decomposed.remainder[EntryScaleY],
      decomposed.delta.x, decomposed.delta.y);

  Rotate(decomposed.angle_radians);
  Scale(decomposed.scale_x, decomposed.scale_y);
}

void Affine::Store(float data[EntryCount]) const {
  copyObjectsNonOverlapping(data, d_, EntryCount);
}

void Affine::Load(float data[EntryCount]) {
  copyObjectsNonOverlapping(d_, data, EntryCount);
  InvalidateTypes();
}

bool Affine::operator==(const Affine& other) const {
  for (int i = 0; i < EntryCount; ++i) {
    if (d_[i] != other.d_[i])
      return false;
  }
  return true;
}

void Affine::FormatImpl(TextWriter& out) const {
  out.Write('[');
  for (int i = 0; i < EntryCount; ++i) {
    if (i != 0)
      out.Write(' ');
    out << d_[i];
  }
  out.Write(']');
}

bool isNear(const Affine& lhs, const Affine& rhs, float tolerance) {
  for (int i = 0; i < Affine::EntryCount; ++i) {
    auto idx = static_cast<Affine::EntryType>(i);
    if (!isNear(lhs.get(idx), rhs.get(idx), tolerance))
      return false;
  }
  return true;
}

bool Trylerp(Affine& out, const Affine& x, const Affine& y, double t) {
  if (t == 0) {
    out = x;
    return true;
  }
  if (t == 1) {
    out = y;
    return true;
  }
  DecomposedAffine x_decomp(DecomposedAffine::SkipInit);
  DecomposedAffine y_decomp(DecomposedAffine::SkipInit);
  if (!x.Decompose(x_decomp) || !y.Decompose(y_decomp))
    return false;

  DecomposedAffine out_decomp = lerp(x_decomp, y_decomp, t);
  out.Recompose(out_decomp);
  return true;
}

// https://www.w3.org/TR/css3-transforms/#interpolation-of-decomposed-2d-matrix-values
DecomposedAffine lerp(const DecomposedAffine& a, const DecomposedAffine& b, double t) {
  double a_angle = a.angle_radians;
  double b_angle = b.angle_radians;
  float a_scale_x = a.scale_x;
  float a_scale_y = a.scale_y;

  // If x-axis of one is flipped, and y-axis of the other,
  // convert to an unflipped rotation.
  if ((a_scale_x < 0 && b.scale_y < 0) || (a_scale_y < 0 && b.scale_x < 0)) {
    a_scale_x = -a_scale_x;
    a_scale_y = -a_scale_y;
    a_angle -= CopySign(Angle::StraightInRadians, a_angle);
  }

  a_angle = Angle::NormalizeRadians(a_angle);
  b_angle = Angle::NormalizeRadians(b_angle);

  // Don’t rotate the long way around.
  if (!a_angle)
    a_angle = Angle::FullInRadians;
  if (!b_angle)
    b_angle = Angle::FullInRadians;

  DecomposedAffine out(DecomposedAffine::SkipInit);
  out.delta = lerp(a.delta, b.delta, t);
  out.scale_x = lerp(a_scale_x, b.scale_x, t);
  out.scale_y = lerp(a_scale_y, b.scale_y, t);
  out.angle_radians = lerp(a_angle, b_angle, t);
  for (int i = 0; i < 4; ++i)
    out.remainder[i] = lerp(a.remainder[i], b.remainder[i], t);
  return out;
}

} // namespace stp
