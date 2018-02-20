// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Geometry/Xform3.h"

#include "Base/Containers/ArrayOps.h"
#include "Geometry/Affine.h"
#include "Geometry/Bounds2.h"
#include "Geometry/Quad2.h"
#include "Geometry/Quaternion.h"
#include "Geometry/Xform2.h"

namespace stp {

Xform3::Xform3(
    float scale_x, float skew_y,
    float skew_x, float scale_y,
    float trans_x, float trans_y) {
  SetAffine(
      scale_x, skew_y,
      skew_x, scale_y,
      trans_x, trans_y);
}

unsigned Xform3::GetTypeSlow() const {
  unsigned old_mask = type_mask_;
  unsigned mask;

  if (old_mask & (TypeMaskPerspective << TypeMaskDirtyShift)) {
    mask = 0;

    if (0 != GetEntry(EntryPersp0) ||
        0 != GetEntry(EntryPersp1) ||
        0 != GetEntry(EntryPersp2) ||
        1 != Get(3, 3))
      return TypeMaskAll;

    if (0 != GetEntry(EntryTransX) || 0 != GetEntry(EntryTransY) || 0 != GetEntry(EntryTransZ))
      mask |= TypeMaskTranslate;

    if (1 != GetEntry(EntryScaleX) || 1 != GetEntry(EntryScaleY) || 1 != GetEntry(EntryScaleZ))
      mask |= TypeMaskScale;

    if (0 != d_[1][0] || 0 != d_[0][1] || 0 != d_[0][2] ||
        0 != d_[2][0] || 0 != d_[1][2] || 0 != d_[2][1]) {
      mask |= TypeMaskAffine;
    }
  } else {
    mask = old_mask & TypeMaskAll;

    if (old_mask & (TypeMaskTranslate << TypeMaskDirtyShift)) {
      if (0 != GetEntry(EntryTransX) || 0 != GetEntry(EntryTransY) || 0 != GetEntry(EntryTransZ)) {
        mask |= TypeMaskTranslate;
      } else {
        mask &= ~TypeMaskTranslate;
      }
    }

    if (old_mask & (TypeMaskScale << TypeMaskDirtyShift)) {
      if (1 != GetEntry(EntryScaleX) || 1 != GetEntry(EntryScaleY) || 1 != GetEntry(EntryScaleZ)) {
        mask |= TypeMaskScale;
      } else {
        mask &= ~TypeMaskScale;
      }
    }

    if (old_mask & (TypeMaskAffine << TypeMaskDirtyShift)) {
      if (0 != d_[1][0] || 0 != d_[0][1] || 0 != d_[0][2] ||
          0 != d_[2][0] || 0 != d_[1][2] || 0 != d_[2][1]) {
        mask |= TypeMaskAffine;
      } else {
        mask &= ~TypeMaskAffine;
      }
    }
  }
  type_mask_ = mask;
  ASSERT((type_mask_ & TypeMaskUnknown) == 0);
  return mask;
}

bool Xform3::operator==(const Xform3& other) const {
  if (TriviallyIsIdentity() && other.TriviallyIsIdentity())
    return true;

  const float* a = &d_[0][0];
  const float* b = &other.d_[0][0];

  for (int i = 0; i < 16; ++i) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

static inline bool ApproximatelyZero(float x, float tolerance) {
  return Abs(x) <= tolerance;
}

static inline bool ApproximatelyOne(float x, float tolerance) {
  return Abs(x - 1) <= tolerance;
}

bool Xform3::isNearTranslate(float tolerance) const {
  ASSERT(tolerance >= 0);

  if (IsTranslate())
    return true;

  return ApproximatelyOne(GetEntry(EntryScaleX), tolerance) &&
         ApproximatelyZero(GetEntry(EntryShearY), tolerance) &&
         ApproximatelyZero(Get(2, 0), tolerance) &&
         GetEntry(EntryPersp0) == 0 &&
         ApproximatelyZero(GetEntry(EntryShearX), tolerance) &&
         ApproximatelyOne(GetEntry(EntryScaleY), tolerance) &&
         ApproximatelyZero(Get(2, 1), tolerance) &&
         GetEntry(EntryPersp1) == 0 &&
         ApproximatelyZero(Get(0, 2), tolerance) &&
         ApproximatelyZero(Get(1, 2), tolerance) &&
         ApproximatelyOne(GetEntry(EntryScaleZ), tolerance) &&
         GetEntry(EntryPersp2) == 0 &&
         Get(3, 3) == 1;
}

bool Xform3::IsIntegerTranslate(float tolerance) const {
  return IsTranslate() ? HasIntegerTranslate(tolerance) : false;
}

bool Xform3::IsScaleIntegerTranslate(float tolerance) const {
  return IsScaleTranslate() ? HasIntegerTranslate(tolerance) : false;
}

bool Xform3::HasIntegerTranslate(float tolerance) const {
  if (IsIdentity())
    return true;

  bool no_fractional_translation;
  if (tolerance == 0) {
    no_fractional_translation =
        static_cast<int>(GetEntry(EntryTransX)) == GetEntry(EntryTransX) &&
        static_cast<int>(GetEntry(EntryTransY)) == GetEntry(EntryTransY) &&
        static_cast<int>(GetEntry(EntryTransZ)) == GetEntry(EntryTransZ);
  } else {
    no_fractional_translation =
        isNear(Round(GetEntry(EntryTransX)), GetEntry(EntryTransX), tolerance) &&
        isNear(Round(GetEntry(EntryTransY)), GetEntry(EntryTransY), tolerance) &&
        isNear(Round(GetEntry(EntryTransZ)), GetEntry(EntryTransZ), tolerance);
  }
  return no_fractional_translation;
}

bool Xform3::IsScale2D() const {
  return IsScale() && GetEntry(EntryScaleZ) == 1;
}

void Xform3::SetTranslate(float dx, float dy, float dz) {
  SetIdentity();

  if (dx == 0 && dy == 0 && dz == 0)
    return;

  d_[3][0] = dx;
  d_[3][1] = dy;
  d_[3][2] = dz;

  SetTypeMask(TypeMaskTranslate);
}

void Xform3::Translate(float dx, float dy, float dz) {
  if (dx == 0 && dy == 0 && dz == 0)
    return;

  if (!TriviallyHasPerspective()) {
    if (TriviallyIsScaleTranslate()) {
      if (TriviallyIsIdentity()) { // Identity
        SetTranslate(dx, dy, dz);
        return;
      }
      if (TriviallyIsTranslate()) { // Translate
        d_[3][0] += dx;
        d_[3][1] += dy;
        d_[3][2] += dz;
      } else { // Scale & Translate
        d_[3][0] += d_[0][0] * dx;
        d_[3][1] += d_[1][1] * dy;
        d_[3][2] += d_[2][2] * dz;
      }
    } else { // Affine
      for (int i = 0; i < 3; ++i) {
        d_[3][i] = d_[0][i] * dx + d_[1][i] * dy + d_[2][i] * dz + d_[3][i];
      }
    }
    DirtyTypeMask(TypeMaskTranslate);
    return;
  }

  // Perspective
  for (int i = 0; i < 4; ++i) {
    d_[3][i] = d_[0][i] * dx + d_[1][i] * dy + d_[2][i] * dz + d_[3][i];
  }
  // No need to DirtyTypeMask for perspective - the translation can't drop perspective.
}

void Xform3::PostTranslate(float dx, float dy, float dz) {
  if (dx == 0 && dy == 0 && dz == 0)
    return;

  if (HasPerspective()) {
    for (int i = 0; i < 4; ++i) {
      d_[i][0] += d_[i][3] * dx;
      d_[i][1] += d_[i][3] * dy;
      d_[i][2] += d_[i][3] * dz;
    }
  } else {
    d_[3][0] += dx;
    d_[3][1] += dy;
    d_[3][2] += dz;
    DirtyTypeMask(TypeMaskTranslate);
  }
}

void Xform3::SetTranslate2D(float dx, float dy) {
  SetTranslate(dx, dy, 0);
}

void Xform3::Translate2D(float dx, float dy) {
  Translate(dx, dy, 0);
}

void Xform3::TranslateXAxis(float dx) {
  Translate(dx, 0, 0);
}

void Xform3::TranslateYAxis(float dy) {
  Translate(0, dy, 0);
}

void Xform3::TranslateZAxis(float dz) {
  Translate(0, 0, dz);
}

void Xform3::PostTranslate2D(float dx, float dy) {
  PostTranslate(dx, dy, 0);
}

void Xform3::SetScale(float sx, float sy, float sz) {
  SetIdentity();

  if (sx == 1 && sy == 1 && sz == 1)
    return;

  d_[0][0] = sx;
  d_[1][1] = sy;
  d_[2][2] = sz;

  SetTypeMask(TypeMaskScale);
}

void Xform3::SetScale2D(float sx, float sy) {
  SetScale(sx, sy, 1);
}

void Xform3::Scale(float sx, float sy, float sz) {
  if (1 == sx && 1 == sy && 1 == sz)
    return;

  if (!TriviallyHasPerspective()) {
    if (TriviallyIsIdentity()) {
      SetScale(sx, sy, sz);
      return;
    }
    if (TriviallyIsScaleTranslate()) {
      d_[0][0] *= sx;
      d_[1][1] *= sy;
      d_[2][2] *= sz;
      DirtyTypeMask(TypeMaskScale);
    } else {
      for (int i = 0; i < 3; i++) {
        d_[0][i] *= sx;
        d_[1][i] *= sy;
        d_[2][i] *= sz;
      }
      DirtyTypeMask(TypeMaskScale | TypeMaskAffine);
    }
    return;
  }

  for (int i = 0; i < 4; i++) {
    d_[0][i] *= sx;
    d_[1][i] *= sy;
    d_[2][i] *= sz;
  }
  DirtyTypeMask(); // Scaling may drop perspective.
}

void Xform3::ScaleXAxis(float sx) {
  Scale(sx, 1, 1);
}

void Xform3::ScaleYAxis(float sy) {
  Scale(1, sy, 1);
}

void Xform3::ScaleZAxis(float sz) {
  Scale(1, 1, sz);
}

void Xform3::PostScale(float sx, float sy, float sz) {
  if (1 == sx && 1 == sy && 1 == sz)
    return;

  if (TriviallyIsScaleTranslate()) {
    if (TriviallyIsIdentity()) {
      SetScale(sx, sy, sz);
      return;
    }
    d_[0][0] *= sx;
    d_[1][1] *= sy;
    d_[2][2] *= sz;
    d_[3][0] *= sx;
    d_[3][1] *= sy;
    d_[3][2] *= sz;
  } else {
    for (int i = 0; i < 4; i++) {
      d_[i][0] *= sx;
      d_[i][1] *= sy;
      d_[i][2] *= sz;
    }
  }
  DirtyTypeMask((type_mask_ & TypeMaskAll) | TypeMaskScale);
}

void Xform3::PostScale2D(float sx, float sy) {
  PostScale(sx, sy, 0);
}

void Xform3::SetScaleTranslate(float sx, float sy, float sz, float tx, float ty, float tz) {
  SetIdentity();

  d_[0][0] = sx;
  d_[1][1] = sy;
  d_[2][2] = sz;

  d_[3][0] = tx;
  d_[3][1] = ty;
  d_[3][2] = tz;

  DirtyTypeMask(TypeMaskScale| TypeMaskTranslate);
}

void Xform3::SetScaleTranslate2D(float sx, float sy, float tx, float ty) {
  SetScaleTranslate(sx, sy, 1, tx, ty, 0);
}

bool Xform3::SetBoundsToBounds(const Bounds2& src, const Bounds2& dst, ScaleToFit scale_to_fit) {
  if (src.isEmpty()) {
    SetIdentity();
    return false;
  }

  if (dst.isEmpty()) {
    SetScale2D(0, 0);
  } else {
    float sx = dst.GetWidth() / src.GetWidth();
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
        diff = dst.GetWidth() - src.GetWidth() * sy;
      else
        diff = dst.GetHeight() - src.GetHeight() * sy;

      if (scale_to_fit == ScaleToFit::Center)
        diff *= 0.5f;

      if (x_larger)
        tx += diff;
      else
        ty += diff;
    }
    SetScaleTranslate2D(sx, sy, tx, ty);
  }
  return true;
}

void Xform3::SetOrthoProjection(
    const Bounds2& bounds,
    bool flip_y,
    float near_plane, float far_plane) {
  ASSERT(!bounds.isEmpty() && near_plane != far_plane);
  float clip = far_plane - near_plane;
  if (flip_y) {
    SetScaleTranslate(
        2 / bounds.GetWidth(), -2 / bounds.GetHeight(), -2 / clip,
        -(bounds.min.x + bounds.max.x) / bounds.GetWidth(),
        (bounds.min.y + bounds.max.y) / bounds.GetHeight(),
        -(near_plane + far_plane) / clip);
  } else {
    SetScaleTranslate(
        2 / bounds.GetWidth(), 2 / bounds.GetHeight(), -2 / clip,
        -(bounds.min.x + bounds.max.x) / bounds.GetWidth(),
        -(bounds.min.y + bounds.max.y) / bounds.GetHeight(),
        -(near_plane + far_plane) / clip);
  }
}

void Xform3::SetOrthoProjectionFlat(const Bounds2& bounds, bool flip_y) {
  ASSERT(!bounds.isEmpty());
  if (flip_y) {
    SetScaleTranslate(
        2 / bounds.GetWidth(), -2 / bounds.GetHeight(), 0,
        -(bounds.min.x + bounds.max.x) / bounds.GetWidth(),
        (bounds.min.y + bounds.max.y) / bounds.GetHeight(),
        0);
  } else {
    SetScaleTranslate(
        2 / bounds.GetWidth(), 2 / bounds.GetHeight(), 0,
        -(bounds.min.x + bounds.max.x) / bounds.GetWidth(),
        -(bounds.min.y + bounds.max.y) / bounds.GetHeight(),
        0);
  }
}

void Xform3::SetRotate(const Quaternion& quaternion) {
  double x = quaternion.x;
  double y = quaternion.y;
  double z = quaternion.z;
  double w = quaternion.w;

  Set3x3(
      1 - 2 * (y * y + z * z),
      2 * (x * y - z * w),
      2 * (x * z + y * w),
      2 * (x * y + z * w),
      1 - 2 * (x * x + z * z),
      2 * (y * z - x * w),
      2 * (x * z - y * w),
      2 * (y * z + x * w),
      1 - 2 * (x * x + y * y));
}

void Xform3::SetRotate2D(double radians) {
  if (radians == 0) {
    SetIdentity();
    return;
  }
  double sin_theta, cos_theta;
  SinCos(radians).Unpack(sin_theta, cos_theta);
  Set3x3(
      cos_theta, -sin_theta, 0,
      sin_theta, cos_theta, 0,
      0, 0, 1);
}

void Xform3::SetRotateAbout(float x, float y, float z, double radians) {
  double len2 = (double)x * x + (double)y * y + (double)z * z;
  if (1 != len2) {
    if (0 == len2) {
      SetIdentity();
      return;
    }
    double scale = 1 / Sqrt(len2);
    x *= scale;
    y *= scale;
    z *= scale;
  }
  SetRotateAboutUnit(x, y, z, radians);
}

void Xform3::SetRotateAboutUnit(float x, float y, float z, double radians) {
  ASSERT(isNear(Vector3(x, y, z).GetLengthSquared(), 1.0, 1E-5));

  if (radians == 0) {
    SetIdentity();
    return;
  }

  double s, c;
  SinCos(radians).Unpack(s, c);
  double C = 1 - c;
  double xs = x * s;
  double ys = y * s;
  double zs = z * s;
  double xC = x * C;
  double yC = y * C;
  double zC = z * C;
  double xyC = x * yC;
  double yzC = y * zC;
  double zxC = z * xC;

  Set3x3(
      x * xC + c, xyC - zs  , zxC + ys,
      xyC + zs  , y * yC + c, yzC - xs,
      zxC - ys  , yzC + xs  , z * zC + c);
}

void Xform3::Rotate(const Quaternion& quaternion) {
  if (quaternion.IsIdentity())
    return;

  if (IsIdentity()) {
    SetRotate(quaternion);
  } else {
    Xform3 rot(SkipInit);
    rot.SetRotate(quaternion);
    Concat(rot);
  }
}

void Xform3::RotateAboutXAxis(double radians) {
  if (radians == 0)
    return;

  double sin_theta, cos_theta;
  SinCos(radians).Unpack(sin_theta, cos_theta);

  Xform3 rot(SkipInit);
  rot.Set3x3(
      1, 0, 0,
      0, cos_theta, -sin_theta,
      0, sin_theta, cos_theta);

  if (IsIdentity())
    *this = rot;
  else
    Concat(rot);
}

void Xform3::RotateAboutYAxis(double radians) {
  if (radians == 0)
    return;

  double sin_theta, cos_theta;
  SinCos(radians).Unpack(sin_theta, cos_theta);

  Xform3 rot(SkipInit);
  rot.Set3x3(
      cos_theta, 0, sin_theta,
      0, 1, 0,
      -sin_theta, 0, cos_theta);

  if (IsIdentity())
    *this = rot;
  else
    Concat(rot);
}

void Xform3::RotateAboutZAxis(double radians) {
  if (radians == 0)
    return;

  Xform3 rot(Xform3::SkipInit);
  rot.SetRotate2D(radians);

  if (IsIdentity())
    *this = rot;
  else
    Concat(rot);
}

bool Xform3::RotateAbout(const Vector3& in_axis, double radians) {
  Vector3 axis = in_axis;
  if (!axis.TryNormalize())
    return false;
  RotateAboutUnit(axis, radians);
  return true;
}

bool Xform3::RotateAbout(float x, float y, float z, double radians) {
  return RotateAbout(Vector3(x, y, z), radians);
}

void Xform3::RotateAboutUnit(const Vector3& axis, double radians) {
  if (radians == 0)
    return;

  if (IsIdentity()) {
    SetRotateAboutUnit(axis, radians);
  } else {
    Xform3 rot(SkipInit);
    rot.SetRotateAboutUnit(axis, radians);
    Concat(rot);
  }
}

void Xform3::RotateAboutUnit(float x, float y, float z, double angle) {
  RotateAboutUnit(Vector3(x, y, z), angle);
}

void Xform3::SetShear(float kx, float ky) {
  Set3x3(
      1, kx, 0,
      ky, 1, 0,
      0,  0, 1);
}

void Xform3::Shear(float kx, float ky) {
  if (kx == 0 && ky == 0)
    return;
  Xform3 shear(SkipInit);
  shear.SetShear(kx, ky);
  Concat(shear);
}

void Xform3::Skew(double angle_x, double angle_y) {
  SkewRadians(Angle::DegreesToRadians(angle_x), Angle::DegreesToRadians(angle_y));
}

void Xform3::SkewRadians(double angle_x, double angle_y) {
  Shear(Tan(angle_x), Tan(angle_y));
}

void Xform3::SkewX(double ax) {
  Skew(ax, 0);
}

void Xform3::SkewY(double ay) {
  Skew(0, ay);
}

void Xform3::SkewXRadians(double ax) {
  SkewRadians(ax, 0);
}

void Xform3::SkewYRadians(double ay) {
  SkewRadians(0, ay);
}

void Xform3::SetAffine(const Affine& affine) {
  SetAffine(
      affine.get(Affine::EntryScaleX), affine.get(Affine::EntryShearY),
      affine.get(Affine::EntryShearX), affine.get(Affine::EntryScaleY),
      affine.get(Affine::EntryTransX), affine.get(Affine::EntryTransY));
}

void Xform3::SetXform2d(const Xform2& x) {
  *this = Xform3(
      x.get(Xform2::EntryScaleX), x.get(Xform2::EntryShearX), 0, x.get(Xform2::EntryTransX),
      x.get(Xform2::EntryShearY), x.get(Xform2::EntryScaleY), 0, x.get(Xform2::EntryTransY),
      0, 0, 0, 0,
      x.get(Xform2::EntryPersp0), x.get(Xform2::EntryPersp1), 0, x.get(Xform2::EntryLast));

  if (x.IsScaleTranslate()) {
    if (x.IsIdentity())
      SetTypeMask(TypeMaskIdentity);
    else
      SetDirtyTypeMask(TypeMaskScale | TypeMaskTranslate);
  }
}

void Xform3::Set3x3(
    float col1row1, float col2row1, float col3row1,
    float col1row2, float col2row2, float col3row2,
    float col1row3, float col2row3, float col3row3) {
  d_[0][0] = col1row1;
  d_[0][1] = col1row2;
  d_[0][2] = col1row3;
  d_[0][3] = 0;

  d_[1][0] = col2row1;
  d_[1][1] = col2row2;
  d_[1][2] = col2row3;
  d_[1][3] = 0;

  d_[2][0] = col3row1;
  d_[2][1] = col3row2;
  d_[2][2] = col3row3;
  d_[2][3] = 0;

  d_[3][0] = 0;
  d_[3][1] = 0;
  d_[3][2] = 0;
  d_[3][3] = 1;

  SetDirtyTypeMask(TypeMaskAffine | TypeMaskScale);
}

void Xform3::ApplyPerspectiveDepth(float depth) {
  if (depth == 0)
    return;
  if (IsIdentity()) {
    Set(3, 2, -1 / depth);
  } else {
    Xform3 m(InitWithIdentity);
    m.set(3, 2, -1 / depth);
    Concat(m);
  }
}

void Xform3::SetFrustum(
    float left, float right, float top, float bottom,
    float near_plane, float far_plane) {
  float width = right - left;
  float inv_height = top - bottom;
  float clip = far_plane - near_plane;
  ASSERT(width != 0 && inv_height != 0 && clip != 0);

  SetIdentity();
  Set(0, 0, 2.f * near_plane / width);
  Set(1, 1, 2.f * near_plane / inv_height);
  Set(0, 2, (right + left) / width);
  Set(1, 2, (top + bottom) / inv_height);
  Set(2, 2, -(far_plane + near_plane) / clip);
  Set(3, 2, -1.f);
  Set(2, 3, -2.f * far_plane * near_plane / clip);
}

void Xform3::SetPerspective(
    double fov_radians, float aspect_ratio,
    float near_plane, float far_plane) {
  float ymax = near_plane * Tan(fov_radians);
  float xmax = ymax * aspect_ratio;
  SetFrustum(-xmax, xmax, -ymax, ymax, near_plane, far_plane);
}

void Xform3::SetLookAt(const Vector3& eye, const Vector3& origin, const Vector3& up) {
  ASSERT(up.IsNormalized());
  float dotND = DotProduct(up, eye - origin);
  float m00 = dotND - eye.x * up.x;
  float m01 = -eye.x * up.y;
  float m02 = -eye.x * up.z;
  float m10 = -eye.y * up.x;
  float m11 = dotND - eye.y * up.y;
  float m12 = -eye.y * up.z;
  float m20 = -eye.z * up.x;
  float m21 = -eye.z * up.y;
  float m22 = dotND - eye.z * up.z;

  *this = Xform3(
      m00, m01, m02, -DotProduct(Vector3(m00, m01, m02), eye),
      m10, m11, m12, -DotProduct(Vector3(m10, m11, m12), eye),
      m20, m21, m22, -DotProduct(Vector3(m20, m21, m22), eye),
      -up.x, -up.y, -up.z, DotProduct(up, eye));
}

void Xform3::SetConcat(const Xform3& a, const Xform3& b) {
  unsigned a_mask = a.GetType();
  unsigned b_mask = b.GetType();

  if (a_mask == TypeMaskIdentity) {
      *this = b;
      return;
  }
  if (b_mask == TypeMaskIdentity) {
      *this = a;
      return;
  }

  bool use_storage = (this == &a || this == &b);
  float storage[EntryCount];
  float* result = use_storage ? storage : &d_[0][0];

  if (((a_mask | b_mask) & (TypeMaskTranslate | TypeMaskScale)) == 0) {
    // Both matrices are at most scale+translate
    result[0] = a.d_[0][0] * b.d_[0][0];
    result[1] = result[2] = result[3] = result[4] = 0;
    result[5] = a.d_[1][1] * b.d_[1][1];
    result[6] = result[7] = result[8] = result[9] = 0;
    result[10] = a.d_[2][2] * b.d_[2][2];
    result[11] = 0;
    result[12] = a.d_[0][0] * b.d_[3][0] + a.d_[3][0];
    result[13] = a.d_[1][1] * b.d_[3][1] + a.d_[3][1];
    result[14] = a.d_[2][2] * b.d_[3][2] + a.d_[3][2];
    result[15] = 1;
    SetDirtyTypeMask(TypeMaskTranslate | TypeMaskScale);
  } else {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++) {
        double value = 0;
        for (int k = 0; k < 4; k++) {
          value += static_cast<double>(a.d_[k][i]) * b.d_[j][k];
        }
        *result++ = value;
      }
    }
    DirtyTypeMask();
  }

  if (use_storage)
    copyObjectsNonOverlapping(&d_[0][0], storage, EntryCount);
}

void Xform3::Concat(const Xform3& other) {
  SetConcat(*this, other);
}

void Xform3::PostConcat(const Xform3& other) {
  SetConcat(other, *this);
}

bool Xform3::Preserves2DAxisAlignment(float epsilon) const {
  if (TriviallyIsTranslate())
    return true;

  if (0 != GetEntry(EntryPersp0) || 0 != GetEntry(EntryPersp1))
    return false;

  // A matrix with two non-zeroish values in any of the upper right
  // rows or columns will skew.  If only one value in each row or
  // column is non-zeroish, we get a scale plus perhaps a 90-degree rotation.
  int col0 = 0;
  int col1 = 0;
  int row0 = 0;
  int row1 = 0;

  // Must test against epsilon, not 0, because we can get values
  // around 6e-17 in the matrix that "should" be 0.

  if (Abs(d_[0][0]) > epsilon) {
    col0++;
    row0++;
  }
  if (Abs(d_[0][1]) > epsilon) {
    col1++;
    row0++;
  }
  if (Abs(d_[1][0]) > epsilon) {
    col0++;
    row1++;
  }
  if (Abs(d_[1][1]) > epsilon) {
    col1++;
    row1++;
  }
  return !(col0 > 1 || col1 > 1 || row0 > 1 || row1 > 1);
}

bool Xform3::IsBackFaceVisible() const {
  // Compute whether a layer with a forward-facing normal of (0, 0, 1, 0)
  // would have its back face visible after applying the transform.
  if (IsScaleTranslate())
    return false;

  // This is done by transforming the normal and seeing if the resulting z
  // value is positive or negative. However, note that transforming a normal
  // actually requires using the inverse-transpose of the original transform.
  //
  // We can avoid inverting and transposing the matrix since we know we want
  // to transform only the specific normal vector (0, 0, 1, 0). In this case,
  // we only need the 3rd row, 3rd column of the inverse-transpose. We can
  // calculate only the 3rd row 3rd column element of the inverse, skipping
  // everything else.
  //
  // For more information, refer to:
  //   http://en.wikipedia.org/wiki/Invertible_matrix#Analytic_solution
  //

  double determinant = GetDeterminant();

  // If matrix was not invertible, then just assume back face is not visible.
  if (determinant == 0)
    return false;

  // Compute the cofactor of the 3rd row, 3rd column.
  double cofactor_part_1 = Get(0, 0) * Get(1, 1) * Get(3, 3);
  double cofactor_part_2 = Get(0, 1) * Get(1, 3) * Get(3, 0);
  double cofactor_part_3 = Get(0, 3) * Get(1, 0) * Get(3, 1);
  double cofactor_part_4 = Get(0, 0) * Get(1, 3) * Get(3, 1);
  double cofactor_part_5 = Get(0, 1) * Get(1, 0) * Get(3, 3);
  double cofactor_part_6 = Get(0, 3) * Get(1, 1) * Get(3, 0);

  double cofactor33 =
      cofactor_part_1 +
      cofactor_part_2 +
      cofactor_part_3 -
      cofactor_part_4 -
      cofactor_part_5 -
      cofactor_part_6;

  // Technically the transformed z component is cofactor33 / determinant.  But
  // we can avoid the costly division because we only care about the resulting
  // +/- sign; we can check this equivalently by multiplication.
  return cofactor33 * determinant < -FLT_EPSILON;
}

void Xform3::Transpose() {
  swap(d_[0][1], d_[1][0]);
  swap(d_[0][2], d_[2][0]);
  swap(d_[0][3], d_[3][0]);
  swap(d_[1][2], d_[2][1]);
  swap(d_[1][3], d_[3][1]);
  swap(d_[2][3], d_[3][2]);

  if (!TriviallyIsIdentity())
    DirtyTypeMask();
}

void Xform3::FlattenTo2D() {
  Set(2, 0, 0.0);
  Set(2, 1, 0.0);
  Set(0, 2, 0.0);
  Set(1, 2, 0.0);
  Set(2, 2, 1.0);
  Set(3, 2, 0.0);
  Set(2, 3, 0.0);
}

Xform2 Xform3::GetFlattenedTo2DAsXform2d() const {
  return Xform2(
      GetEntry(EntryScaleX), GetEntry(EntryShearX), GetEntry(EntryTransX),
      GetEntry(EntryShearY), GetEntry(EntryScaleY), GetEntry(EntryTransY),
      GetEntry(EntryPersp0), GetEntry(EntryPersp1), Get(3, 3));
}

Affine Xform3::GetFlattenedTo2DAsAffine() const {
  return Affine(GetEntry(EntryScaleX), GetEntry(EntryShearY),
                GetEntry(EntryShearX), GetEntry(EntryScaleY),
                GetEntry(EntryTransX), GetEntry(EntryTransY));
}

bool Xform3::IsFlat() const {
  if (IsScaleTranslate2D())
    return true;

  return Get(2, 0) == 0.0 && Get(2, 1) == 0.0 &&
         Get(0, 2) == 0.0 && Get(1, 2) == 0.0 &&
         Get(2, 2) == 1.0 && Get(3, 2) == 0.0 &&
         Get(2, 3) == 0.0;
}

Point3 Xform3::MapPoint(Point3 point) const {
  const float src[4] = { point.x, point.y, point.z, 1 };
  float dst[4];
  MapMatrix4x1(dst, src);
  if (dst[3] != 1 && dst[3] != 0.f) {
    float w_inverse = 1 / dst[3];
    return Point3(dst[0] * w_inverse, dst[1] * w_inverse, dst[2] * w_inverse);
  }
  return Point3(dst[0], dst[1], dst[2]);
}

void Xform3::MapPoints(Point3 dst[], const Point3 src[], int count) const {
  for (int i = 0; i < count; ++i)
    dst[i] = MapPoint(src[i]);
}

void Xform3::MapMatrix4x1(float dst[4], const float src[4]) const {
  float storage[4];
  float* result = (src == dst) ? storage : dst;

  for (int i = 0; i < 4; i++) {
    float value = 0;
    for (int j = 0; j < 4; j++) {
      value += d_[j][i] * src[j];
    }
    result[i] = value;
  }

  if (storage == result)
    copyObjectsNonOverlapping(dst, storage, 4);
}

double Xform3::GetDeterminant() const {
  if (IsScaleTranslate()) {
    if (IsIdentity())
      return 1;
    return d_[0][0] * d_[1][1] * d_[2][2] * d_[3][3];
  }

  double a00 = d_[0][0];
  double a01 = d_[0][1];
  double a02 = d_[0][2];
  double a03 = d_[0][3];
  double a10 = d_[1][0];
  double a11 = d_[1][1];
  double a12 = d_[1][2];
  double a13 = d_[1][3];
  double a20 = d_[2][0];
  double a21 = d_[2][1];
  double a22 = d_[2][2];
  double a23 = d_[2][3];
  double a30 = d_[3][0];
  double a31 = d_[3][1];
  double a32 = d_[3][2];
  double a33 = d_[3][3];

  double b00 = a00 * a11 - a01 * a10;
  double b01 = a00 * a12 - a02 * a10;
  double b02 = a00 * a13 - a03 * a10;
  double b03 = a01 * a12 - a02 * a11;
  double b04 = a01 * a13 - a03 * a11;
  double b05 = a02 * a13 - a03 * a12;
  double b06 = a20 * a31 - a21 * a30;
  double b07 = a20 * a32 - a22 * a30;
  double b08 = a20 * a33 - a23 * a30;
  double b09 = a21 * a32 - a22 * a31;
  double b10 = a21 * a33 - a23 * a31;
  double b11 = a22 * a33 - a23 * a32;

  return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
}

bool Xform3::GetInverted(Xform3& out) const {
  if (IsTranslate()) {
    if (IsIdentity())
      out.SetIdentity();
    else
      out.SetTranslate(-GetEntry(EntryTransX), -GetEntry(EntryTransY), -GetEntry(EntryTransZ));
    return true;
  }

  if (IsScaleTranslate()) {
    if (0 == d_[0][0] * d_[1][1] * d_[2][2])
      return false;

    double inv_x_scale = 1 / d_[0][0];
    double inv_y_scale = 1 / d_[1][1];
    double inv_z_scale = 1 / d_[2][2];

    out.d_[0][0] = inv_x_scale;
    out.d_[0][1] = 0;
    out.d_[0][2] = 0;
    out.d_[0][3] = 0;

    out.d_[1][0] = 0;
    out.d_[1][1] = inv_y_scale;
    out.d_[1][2] = 0;
    out.d_[1][3] = 0;

    out.d_[2][0] = 0;
    out.d_[2][1] = 0;
    out.d_[2][2] = inv_z_scale;
    out.d_[2][3] = 0;

    out.d_[3][0] = -d_[3][0] * inv_x_scale;
    out.d_[3][1] = -d_[3][1] * inv_y_scale;
    out.d_[3][2] = -d_[3][2] * inv_z_scale;
    out.d_[3][3] = 1;

    out.type_mask_ = type_mask_;
    return isFinite(out);
  }

  double a00 = d_[0][0];
  double a01 = d_[0][1];
  double a02 = d_[0][2];
  double a03 = d_[0][3];
  double a10 = d_[1][0];
  double a11 = d_[1][1];
  double a12 = d_[1][2];
  double a13 = d_[1][3];
  double a20 = d_[2][0];
  double a21 = d_[2][1];
  double a22 = d_[2][2];
  double a23 = d_[2][3];
  double a30 = d_[3][0];
  double a31 = d_[3][1];
  double a32 = d_[3][2];
  double a33 = d_[3][3];

  if (!HasPerspective()) {
    // If we know the matrix has no perspective, then the perspective
    // component is (0, 0, 0, 1). We can use this information to save a lot
    // of arithmetic that would otherwise be spent to compute the inverse
    // of a general matrix.

    ASSERT(a03 == 0);
    ASSERT(a13 == 0);
    ASSERT(a23 == 0);
    ASSERT(a33 == 1);

    double b00 = a00 * a11 - a01 * a10;
    double b01 = a00 * a12 - a02 * a10;
    double b03 = a01 * a12 - a02 * a11;
    double b06 = a20 * a31 - a21 * a30;
    double b07 = a20 * a32 - a22 * a30;
    double b08 = a20;
    double b09 = a21 * a32 - a22 * a31;
    double b10 = a21;
    double b11 = a22;

    // Calculate the determinant
    double det = b00 * b11 - b01 * b10 + b03 * b08;

    double invdet = 1 / det;
    // If det is zero, we want to return false. However, we also want to return false
    // if 1/det overflows to infinity (i.e. det is denormalized). Both of these are
    // handled by checking that 1/det is finite.
    if (!isFinite(invdet))
      return false;

    b00 *= invdet;
    b01 *= invdet;
    b03 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;

    out.d_[0][0] = a11 * b11 - a12 * b10;
    out.d_[0][1] = a02 * b10 - a01 * b11;
    out.d_[0][2] = b03;
    out.d_[0][3] = 0;
    out.d_[1][0] = a12 * b08 - a10 * b11;
    out.d_[1][1] = a00 * b11 - a02 * b08;
    out.d_[1][2] = -b01;
    out.d_[1][3] = 0;
    out.d_[2][0] = a10 * b10 - a11 * b08;
    out.d_[2][1] = a01 * b08 - a00 * b10;
    out.d_[2][2] = b00;
    out.d_[2][3] = 0;
    out.d_[3][0] = a11 * b07 - a10 * b09 - a12 * b06;
    out.d_[3][1] = a00 * b09 - a01 * b07 + a02 * b06;
    out.d_[3][2] = a31 * b01 - a30 * b03 - a32 * b00;
    out.d_[3][3] = 1;

    out.type_mask_ = type_mask_;
    return isFinite(out);
  }

  double b00 = a00 * a11 - a01 * a10;
  double b01 = a00 * a12 - a02 * a10;
  double b02 = a00 * a13 - a03 * a10;
  double b03 = a01 * a12 - a02 * a11;
  double b04 = a01 * a13 - a03 * a11;
  double b05 = a02 * a13 - a03 * a12;
  double b06 = a20 * a31 - a21 * a30;
  double b07 = a20 * a32 - a22 * a30;
  double b08 = a20 * a33 - a23 * a30;
  double b09 = a21 * a32 - a22 * a31;
  double b10 = a21 * a33 - a23 * a31;
  double b11 = a22 * a33 - a23 * a32;

  // Calculate the determinant
  double det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

  double invdet = 1 / det;
  // If det is zero, we want to return false. However, we also want to return false
  // if 1/det overflows to infinity (i.e. det is denormalized). Both of these are
  // handled by checking that 1/det is finite.
  if (!isFinite(invdet))
    return false;

  b00 *= invdet;
  b01 *= invdet;
  b02 *= invdet;
  b03 *= invdet;
  b04 *= invdet;
  b05 *= invdet;
  b06 *= invdet;
  b07 *= invdet;
  b08 *= invdet;
  b09 *= invdet;
  b10 *= invdet;
  b11 *= invdet;

  out.d_[0][0] = a11 * b11 - a12 * b10 + a13 * b09;
  out.d_[0][1] = a02 * b10 - a01 * b11 - a03 * b09;
  out.d_[0][2] = a31 * b05 - a32 * b04 + a33 * b03;
  out.d_[0][3] = a22 * b04 - a21 * b05 - a23 * b03;
  out.d_[1][0] = a12 * b08 - a10 * b11 - a13 * b07;
  out.d_[1][1] = a00 * b11 - a02 * b08 + a03 * b07;
  out.d_[1][2] = a32 * b02 - a30 * b05 - a33 * b01;
  out.d_[1][3] = a20 * b05 - a22 * b02 + a23 * b01;
  out.d_[2][0] = a10 * b10 - a11 * b08 + a13 * b06;
  out.d_[2][1] = a01 * b08 - a00 * b10 - a03 * b06;
  out.d_[2][2] = a30 * b04 - a31 * b02 + a33 * b00;
  out.d_[2][3] = a21 * b02 - a20 * b04 - a23 * b00;
  out.d_[3][0] = a11 * b07 - a10 * b09 - a12 * b06;
  out.d_[3][1] = a00 * b09 - a01 * b07 + a02 * b06;
  out.d_[3][2] = a31 * b01 - a30 * b03 - a32 * b00;
  out.d_[3][3] = a20 * b03 - a21 * b01 + a22 * b00;

  out.type_mask_ = type_mask_;

  return isFinite(out);
}

bool Xform3::IsInvertible() const {
  if (IsScaleTranslate()) {
    if (IsTranslate())
      return true;
    return d_[0][0] * d_[1][1] * d_[2][2] != 0;
  }
  return isFinite(1 / GetDeterminant());
}

bool isFinite(const Xform3& xform) {
  float accumulator = 0;
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col)
      accumulator *= xform.get(row, col);
  }
  return accumulator == 0;
}

// Returns false if the matrix cannot be normalized.
static bool TryNormalize(Xform3& m) {
  if (m.get(3, 3) == 0) {
    // Cannot normalize.
    return false;
  }

  float scale = 1 / m.get(3, 3);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++)
      m.set(i, j, m.get(i, j) * scale);
  }
  return true;
}

// Decomposition needs double-precision enabled dot and cross products.
static inline float DotProduct2(const Vector3& lhs, const Vector3& rhs) {
  return static_cast<double>(lhs.x) * rhs.x +
      static_cast<double>(lhs.y) * rhs.y +
      static_cast<double>(lhs.z) * rhs.z;
}

static inline Vector3 CrossProduct2(const Vector3& lhs, const Vector3& rhs) {
  float x = static_cast<double>(lhs.y) * rhs.z - static_cast<double>(lhs.z) * rhs.y;
  float y = static_cast<double>(lhs.z) * rhs.x - static_cast<double>(lhs.x) * rhs.z;
  float z = static_cast<double>(lhs.x) * rhs.y - static_cast<double>(lhs.y) * rhs.x;
  return Vector3(x, y, z);
}

// Taken from http://www.w3.org/TR/css3-transforms/.
bool Xform3::Decompose(DecomposedXform3& out) const {
  if (IsScaleTranslate()) {
    out.SetScaleTranslate(
        GetEntry(EntryScaleX), GetEntry(EntryScaleY), GetEntry(EntryScaleZ),
        GetEntry(EntryTransX), GetEntry(EntryTransY), GetEntry(EntryTransZ));
    return true;
  }

  // We'll operate on a copy of the matrix.
  Xform3 matrix = *this;

  // If we cannot normalize the matrix, then bail early as we cannot decompose.
  if (!TryNormalize(matrix))
    return false;

  Xform3 perspective_matrix = matrix;

  for (int i = 0; i < 3; ++i)
    perspective_matrix.set(3, i, 0);
  perspective_matrix.set(3, 3, 1);

  // If the perspective matrix is not invertible, we are also unable to
  // decompose, so we'll bail early.
  if (Abs(perspective_matrix.GetDeterminant()) < 1e-8)
    return false;

  if (matrix.GetEntry(EntryPersp0) != 0 ||
      matrix.GetEntry(EntryPersp1) != 0 ||
      matrix.GetEntry(EntryPersp2) != 0) {
    float rhs[4] = {
      matrix.get(3, 0),
      matrix.get(3, 1),
      matrix.get(3, 2),
      matrix.get(3, 3)
    };

    // Solve the equation by inverting perspective_matrix and multiplying
    // rhs by the inverse.
    Xform3 inverse_perspective_matrix(Xform3::SkipInit);
    if (!perspective_matrix.GetInverted(inverse_perspective_matrix))
      return false;

    inverse_perspective_matrix.Transpose();
    inverse_perspective_matrix.MapMatrix4x1(rhs, rhs);

    for (int i = 0; i < 4; ++i)
      out.perspective[i] = rhs[i];

  } else {
    // No perspective.
    for (int i = 0; i < 3; ++i)
      out.perspective[i] = 0;
    out.perspective[3] = 1;
  }

  out.translate = matrix.GetTransInternal();

  Vector3 rows[3];
  for (int i = 0; i < 3; i++)
    rows[i] = Vector3(matrix.get(0, i), matrix.get(1, i), matrix.get(2, i));

  // Compute X scale factor and normalize first row.
  out.scale[0] = rows[0].GetLength();
  if (out.scale[0] != 0)
    rows[0] *= 1.f / out.scale[0];

  // Compute XY shear factor and make 2nd row orthogonal to 1st.
  out.shear[0] = DotProduct2(rows[0], rows[1]);
  rows[1] -= rows[0] * out.shear[0];

  // Now, compute Y scale and normalize 2nd row.
  out.scale[1] = rows[1].GetLength();
  if (out.scale[1] != 0)
    rows[1] *= 1.f / out.scale[1];

  out.shear[0] /= out.scale[1];

  // Compute XZ and YZ shears, orthogonalize 3rd row.
  out.shear[1] = DotProduct2(rows[0], rows[2]);
  rows[2] -= rows[0] * out.shear[1];
  out.shear[2] = DotProduct2(rows[1], rows[2]);
  rows[2] -= rows[1] * out.shear[2];

  // Next, get Z scale and normalize 3rd row.
  out.scale[2] = rows[2].GetLength();
  if (out.scale[2] != 0)
    rows[2] *= 1.f / out.scale[2];

  out.shear[1] /= out.scale[2];
  out.shear[2] /= out.scale[2];

  // At this point, the matrix (in rows) is orthonormal.
  // Check for a coordinate system flip.  If the determinant
  // is -1, then negate the matrix and the scaling factors.
  Vector3 pdum3 = CrossProduct2(rows[1], rows[2]);
  if (DotProduct2(rows[0], pdum3) < 0) {
    for (int i = 0; i < 3; ++i) {
      out.scale[i] = -out.scale[i];
      rows[i] = -rows[i];
    }
  }

  double row00 = rows[0].x;
  double row11 = rows[1].y;
  double row22 = rows[2].z;

  double qx = 0.5 * Sqrt(max(1.0 + row00 - row11 - row22, 0.0));
  double qy = 0.5 * Sqrt(max(1.0 - row00 + row11 - row22, 0.0));
  double qz = 0.5 * Sqrt(max(1.0 - row00 - row11 + row22, 0.0));
  double qw = 0.5 * Sqrt(max(1.0 + row00 + row11 + row22, 0.0));

  if (rows[2].y > rows[1].z)
    qx = -qx;
  if (rows[0].z > rows[2].x)
    qy = -qy;
  if (rows[1].x > rows[0].y)
    qz = -qz;

  out.quaternion = Quaternion(qw, qx, qy, qz);
  return true;
}

static void ApplyShear(Xform3* xform, const float* decomp) {
  Xform3 temp(Xform3::InitWithIdentity);
  Xform3 skew_xform(Xform3::InitWithIdentity);
  if (decomp[0] || decomp[1] || decomp[2]) {
    temp.set(1, 2, decomp[2]);
    skew_xform.Concat(temp);
  }

  if (decomp[1]) {
    temp.set(1, 2, 0);
    temp.set(0, 2, decomp[1]);
    skew_xform.Concat(temp);
  }

  if (decomp[0]) {
    temp.set(0, 2, 0);
    temp.set(0, 1, decomp[0]);
    skew_xform.Concat(temp);
  }
  xform->Concat(skew_xform);
}

void Xform3::Recompose(const DecomposedXform3& decomp) {
  SetIdentity();
  for (int i = 0; i < 4; ++i)
    Set(3, i, decomp.perspective[i]);
  Translate(decomp.translate);
  Rotate(decomp.quaternion);
  ApplyShear(this, decomp.shear);
  Scale(decomp.scale[0], decomp.scale[1], decomp.scale[2]);
}

bool isNear(const Xform3& lhs, const Xform3& rhs, float tolerance) {
  const float ComponentTolerance = tolerance;

  // We may have a larger discrepancy in the scroll components due to snapping
  // (floating point error might round the other way).
  const float TranslationTolerance = tolerance * 10;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      const float delta = Abs(lhs.get(row, col) - rhs.get(row, col));
      const float tolerance = (col == 3 && row < 3) ? TranslationTolerance : ComponentTolerance;
      if (delta > tolerance)
        return false;
    }
  }
  return true;
}

bool Trylerp(Xform3& out, const Xform3& x, const Xform3& y, double t) {
  if (t == 0) {
    out = x;
    return true;
  }
  if (t == 1) {
    out = y;
    return true;
  }
  DecomposedXform3 x_decomp(DecomposedXform3::SkipInit);
  DecomposedXform3 y_decomp(DecomposedXform3::SkipInit);
  if (!x.Decompose(x_decomp) || !y.Decompose(y_decomp))
    return false;

  DecomposedXform3 out_decomp = lerp(x_decomp, y_decomp, t);
  out.Recompose(out_decomp);
  return true;
}

void Xform3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  for (int row = 0; row < RowCount; ++row) {
    out.Write(row == 0 ? '[' : ' ');
    for (int col = 0; col < ColCount; ++col) {
      if (col != 0)
        out.Write(' ');
      out.WriteFloat(Get(row, col));
    }
    if (row != Xform3::RowCount - 1)
      out.WriteLine();
  }
  out.Write(']');
}

namespace {

template<int n>
void Combine(float* out, const float* a, const float* b, double scale_a, double scale_b) {
  for (int i = 0; i < n; ++i)
    out[i] = static_cast<float>(a[i] * scale_a + b[i] * scale_b);
}

} // namespace

DecomposedXform3 lerp(
    const DecomposedXform3& from, const DecomposedXform3& to,
    double progress) {
  // NOTE: Be aware |out| can be equal to |&from| or |&to|, but since we modify each component
  //       separately we don't bother.
  double scalea = 1.0 - progress;
  double scaleb = progress;

  DecomposedXform3 out(DecomposedXform3::SkipInit);
  out.translate = lerp(from.translate, to.translate, progress);
  Combine<3>(out.scale, from.scale, to.scale, scalea, scaleb);
  Combine<3>(out.shear, from.shear, to.shear, scalea, scaleb);
  Combine<4>(out.perspective, from.perspective, to.perspective, scalea, scaleb);
  out.quaternion = Slerp(from.quaternion, to.quaternion, progress);
  return out;
}

static void StreamFloats(TextWriter& out, StringSpan name, const float* fv, int size) {
  out.WriteAscii(name);
  out.WriteAscii(": ");

  for (int i = 0; i < size; ++i) {
    out.Write(' ');
    out.WriteFloat(fv[i]);
  }
  out.WriteLine();
}

void DecomposedXform3::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out.WriteAscii("translate: ");
  format(out, translate);
  out.WriteLine();

  StreamFloats(out, "scale", scale, 3);
  StreamFloats(out, "shear", shear, 3);
  StreamFloats(out, "perspective", perspective, 4);

  out.WriteAscii("quaternion: ");
  format(out, quaternion);
  out.WriteLine();
}

} // namespace stp
