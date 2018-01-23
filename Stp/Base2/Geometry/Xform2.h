// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_XFORM2_H_
#define STP_BASE_GEOMETRY_XFORM2_H_

#include "Base/Debug/Assert.h"
#include "Base/Geometry/Angle.h"
#include "Base/Geometry/Limits.h"
#include "Base/Geometry/Vector2.h"

namespace stp {

struct Bounds2;
struct Quad2;
class Affine;

class BASE_EXPORT Xform2 {
 public:
  enum SkipInitTag { SkipInit };
  enum InitWithIdentityTag { InitWithIdentity };

  enum EntryType {
    EntryScaleX,
    EntryShearX,
    EntryTransX,
    EntryShearY,
    EntryScaleY,
    EntryTransY,
    EntryPersp0,
    EntryPersp1,
    EntryLast,
  };

  enum TypeMask : unsigned {
    TypeMaskTranslate     = 0x01,
    TypeMaskScale         = 0x02,
    TypeMaskAffine        = 0x04,
    TypeMaskPerspective   = 0x08,
    TypeMaskAll           = 0x0F,

    TypeMaskRectStaysRect = 0x100,
    TypeMaskUnknown       = 0x80000000u,
  };

  static constexpr int RowCount = 3;
  static constexpr int ColCount = 3;
  static constexpr int EntryCount = RowCount * ColCount;

  enum class ScaleToFit {
    Fill,
    Start,
    Center,
    End
  };

  explicit Xform2(SkipInitTag) {}

  constexpr explicit Xform2(InitWithIdentityTag);

  constexpr Xform2(
      float scale_x, float shear_y,
      float shear_x, float scale_y,
      float trans_x, float trans_y);

  constexpr Xform2(
      float scale_x, float shear_x, float trans_x,
      float shear_y, float scale_y, float trans_y,
      float persp_0, float persp_1, float m22);

  explicit Xform2(const Affine& affine) { SetAffine(affine); }

  static constexpr Xform2 Identity() { return Xform2(InitWithIdentity); }

  unsigned GetTransforms() const { return GetTypeMask() & TypeMaskAll; }

  bool IsIdentity() const { return GetTransforms() == 0; }

  bool IsTranslate() const { return (GetTransforms() & ~TypeMaskTranslate) == 0; }

  bool IsScale() const { return (GetTransforms() & ~TypeMaskScale) == 0; }

  bool IsScaleTranslate() const;

  bool HasPerspective() const { return (GetTransforms() & TypeMaskPerspective) != 0; }

  bool PreservesAxisAlignment() const { return (GetTypeMask() & TypeMaskRectStaysRect) != 0; }

  bool IsSimilarity(float tolerance = NearlyZeroForGraphics<float>) const;

  bool PreservesRightAngles(float tolerance = NearlyZeroForGraphics<float>) const;

  void SetIdentity() { *this = Identity(); }

  void SetTranslate(Vector2 d) { SetTranslate(d.x, d.y); }
  void SetTranslate(float dx, float dy);

  // M' = M * T(dx, dy)
  void Translate(Vector2 d) { Translate(d.x, d.y); }
  void Translate(float dx, float dy);

  // M' = T(delta) * M
  void PostTranslate(Vector2 d) { PostTranslate(d.x, d.y); }
  void PostTranslate(float dx, float dy);

  void SetScale(float sx, float sy);
  void SetScale(float scale) { SetScale(scale, scale); }

  void SetScale(float sx, float sy, Point2 pivot) { SetScale(sx, sy, pivot.x, pivot.y); }
  void SetScale(float sx, float sy, float px, float py);

  // M' = M * S(sx, sy).
  void Scale(float sx, float sy);
  void Scale(float scale) { Scale(scale, scale); }

  void Scale(float sx, float sy, Point2 pivot) { SetScale(sx, sy, pivot.x, pivot.y); }
  void Scale(float sx, float sy, float px, float py);

  // M' = S(sx, sy) * M
  void PostScale(float sx, float sy);
  void PostScale(float sx, float sy, float px, float py);

  // M' = S(1/divx, 1/divy, 0, 0) * M
  bool PostIntDiv(int divx, int divy) WARN_UNUSED_RESULT;
  bool PostIntDiv(IntVector2 d) WARN_UNUSED_RESULT { return PostIntDiv(d.x, d.y); }

  void SetScaleTranslate(float sx, float sy, Vector2 t);
  void SetScaleTranslate(float sx, float sy, float tx, float ty);

  bool SetBoundsToBounds(
      const Bounds2& src, const Bounds2& dst,
      ScaleToFit scale_to_fit = ScaleToFit::Fill);

  void SetRotate(double radians);

  void SetRotate(double radians, Point2 pivot);
  void SetRotate(double radians, float px, float py);

  // M' = M * R(angle) | counterclockwise
  void Rotate(double radians);

  // M' = R(angle) * M
  void PostRotate(double radians);

  void SetSinCos(float sin_value, float cos_value);

  void SetSinCos(float sin_value, float cos_value, Point2 pivot);
  void SetSinCos(float sin_value, float cos_value, float px, float py);

  void SetShear(float kx, float ky);

  // M' = M * K(kx, ky)
  void Shear(float kx, float ky);

  void SetSkew(double ax, double ay);

  // M' = M * K(ax, ay)
  void Skew(double angle_x, double angle_y);

  // M' = K(ax, ay) * M
  void PostSkew(double ax, double ay);

  void SkewX(double radians);
  void SkewY(double radians);

  void SetAffine(const Affine& affine);

  // *this = lhs * rhs;
  void SetConcat(const Xform2& lhs, const Xform2& rhs);

  // M' = M * other
  void Concat(const Xform2& other);

  // M' = other * M
  void PostConcat(const Xform2& other);

  double GetDeterminant() const;

  bool GetInverted(Xform2& out) const WARN_UNUSED_RESULT;
  bool IsInvertible() const;

  Affine GetFlattenedAsAffine() const;

  // Apply this matrix to the array of points specified by src, and write
  // the transformed points into the array of points specified by dst.
  // dst[] = M * src[]
  // |count| The number of points in src to read, and then transform into dst.
  void MapPoints(Point2 dst[], const Point2 src[], int count) const;

  Point2 MapPoint(Point2 p) const WARN_UNUSED_RESULT;
  Quad2 MapQuad(const Quad2& quad) const WARN_UNUSED_RESULT;
  Vector2 MapVector(Vector2 v) const WARN_UNUSED_RESULT;
  void MapVectors(Vector2 dst[], const Vector2 src[], int count) const;

  typedef void (*MapPointsFunction)(const Xform2& xform, Point2 dst[], const Point2 src[], int count);
  MapPointsFunction GetMapPointsFunction() const { return MapPointsFunctions_[GetTransforms()]; }

  typedef void (*MapXYFunction)(const Xform2& xform, float dst[2], const float src[2]);
  MapXYFunction GetMapXYFunction() const { return MapXYFunctions_[GetTransforms()]; }

  float Get(int entry) const;
  void Set(int entry, float value);

  void operator*=(const Xform2& rhs) { Concat(rhs); }
  Xform2 operator*(const Xform2& rhs) const;

  float operator[](int entry) const { return Get(entry); }

  bool operator==(const Xform2& o) const;
  bool operator!=(const Xform2& other) const { return !operator==(other); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

 private:
  float d_[EntryCount]; // row-major
  mutable unsigned type_mask_;

  unsigned GetTypeMask() const;
  unsigned GetTypeMaskSlow() const;

  void FixTransBit();

  bool TriviallyIsIdentity() const {
    return !(type_mask_ & TypeMaskUnknown) ? (type_mask_ & TypeMaskAll) == 0 : false;
  }

  Vector2 GetTransInternal() const { return Vector2(d_[EntryTransX], d_[EntryTransY]); }

  static void ComplexInverse(float dst[9], const float src[9], double inv_det, bool is_persp);

  static void MapPointsIdent(const Xform2& m, Point2 dst[], const Point2 src[], int count);
  static void MapPointsTrans(const Xform2& m, Point2 dst[], const Point2 src[], int count);
  static void MapPointsScale(const Xform2& m, Point2 dst[], const Point2 src[], int count);
  static void MapPointsAffin(const Xform2& m, Point2 dst[], const Point2 src[], int count);
  static void MapPointsPersp(const Xform2& m, Point2 dst[], const Point2 src[], int count);

  static void MapXYIdent(const Xform2& m, float dst[2], const float src[2]);
  static void MapXYTrans(const Xform2& m, float dst[2], const float src[2]);
  static void MapXYScale(const Xform2& m, float dst[2], const float src[2]);
  static void MapXYScaTr(const Xform2& m, float dst[2], const float src[2]);
  static void MapXYAffin(const Xform2& m, float dst[2], const float src[2]);
  static void MapXYPersp(const Xform2& m, float dst[2], const float src[2]);

  static const MapPointsFunction MapPointsFunctions_[16];
  static const MapXYFunction MapXYFunctions_[16];
};

BASE_EXPORT bool IsNear(const Xform2& lhs, const Xform2& rhs, float tolerance);

BASE_EXPORT bool IsFinite(const Xform2& xform);

constexpr Xform2::Xform2(InitWithIdentityTag)
    : d_ { 1, 0, 0, 0, 1, 0, 0, 0, 1 },
      type_mask_(TypeMaskRectStaysRect) {
}

constexpr Xform2::Xform2(
    float scale_x, float shear_y,
    float shear_x, float scale_y,
    float trans_x, float trans_y)
    : d_ { scale_x, shear_x, trans_x , shear_y, scale_y, trans_y ,  0, 0, 1  },
      type_mask_(TypeMaskUnknown) {
}

constexpr Xform2::Xform2(
    float scale_x, float shear_x, float trans_x,
    float shear_y, float scale_y, float trans_y,
    float persp_0, float persp_1, float m22)
    : d_ {
        scale_x, shear_x, trans_x,
        shear_y, scale_y, trans_y,
        persp_0, persp_1, m22
      },
      type_mask_(TypeMaskUnknown) {
}

inline float Xform2::Get(int entry) const {
  ASSERT(0 <= entry && entry < EntryCount);
  return d_[entry];
}

inline void Xform2::Set(int entry, float value) {
  ASSERT(0 <= entry && entry < EntryCount);
  d_[entry] = value;
  type_mask_ = TypeMaskUnknown;
}

inline unsigned Xform2::GetTypeMask() const {
  unsigned mask;
  if (type_mask_ & TypeMaskUnknown)
    mask = GetTypeMaskSlow();
  else
    mask = type_mask_;
  ASSUME((type_mask_ & TypeMaskUnknown) == 0);
  return mask;
}

inline bool Xform2::IsScaleTranslate() const {
  return (GetTransforms() & ~(TypeMaskTranslate | TypeMaskScale)) == 0;
}

inline void Xform2::SetScaleTranslate(float sx, float sy, Vector2 t) {
  SetScaleTranslate(sx, sy, t.x, t.y);
}

inline void Xform2::SetRotate(double radians, Point2 pivot) {
  SetRotate(radians, pivot.x, pivot.y);
}

inline void Xform2::SetSinCos(float sin_value, float cos_value, Point2 pivot) {
  SetSinCos(sin_value, cos_value, pivot.x, pivot.y);
}

inline Xform2 Xform2::operator*(const Xform2& rhs) const {
  Xform2 out(SkipInit);
  out.SetConcat(*this, rhs);
  return out;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_XFORM2_H_
