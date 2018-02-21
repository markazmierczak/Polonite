// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_AFFINE_H_
#define STP_BASE_GEOMETRY_AFFINE_H_

#include "Base/Debug/Assert.h"
#include "Geometry/Angle.h"
#include "Geometry/Limits.h"
#include "Geometry/Vector2.h"

namespace stp {

struct Bounds2;
struct DecomposedAffine;
struct Quad2;

// https://www.w3.org/TR/css3-transforms
// https://www.w3.org/TR/SVG/coords.html
class BASE_EXPORT Affine {
 public:
  enum SkipInitTag { SkipInit };
  enum InitWithIdentityTag { InitWithIdentity };

  enum EntryType {
    EntryScaleX = 0,
    EntryShearY = 1,
    EntryShearX = 2,
    EntryScaleY = 3,
    EntryTransX = 4,
    EntryTransY = 5,
  };
  static constexpr int EntryCount = 6;

  // The type of transform in the matrix.
  // Use this to identify the complexity of the matrix.
  enum TransformMask {
    TypeMaskTranslate = 0x01,
    TypeMaskScale = 0x02,
    TypeMaskAffine = 0x04,
    TypeMaskAll = 0xF,

    // The following flags are private and never returned by GetTransforms().
    TypeMaskRectStaysRect = 0x100,
    TypeMaskUnknown = 0x80000000u,
  };

  enum class ScaleToFit {
    Fill,
    Start,
    Center,
    End,
  };

  explicit Affine(SkipInitTag) : type_mask_(TypeMaskUnknown) {}

  constexpr explicit Affine(InitWithIdentityTag);

  constexpr Affine(
      float scale_x, float shear_y,
      float shear_x, float scale_y,
      float trans_x, float trans_y);

  static constexpr Affine Identity() { return Affine(InitWithIdentity); }

  unsigned GetTransforms() const { return GetTypeMask() & TypeMaskAll; }

  bool IsIdentity() const { return GetTransforms() == 0; }
  bool IsTranslate() const { return TransformsAre(GetTransforms(), TypeMaskTranslate); }
  bool IsScaleTranslate() const;

  bool Preserves2DAxisAlignment() const { return GetTypeMask() & TypeMaskRectStaysRect; }

  bool IsSimilarity(float tolerance = NearlyZeroForGraphics<float>) const;

  bool PreservesRightAngles(float tolerance = NearlyZeroForGraphics<float>) const;

  void SetIdentity() { *this = Identity(); }

  static Affine MakeTranslate(Vector2 translation);
  static Affine MakeTranslate(float tx, float ty);

  void SetTranslate(Vector2 translation);
  void SetTranslate(float tx, float ty);

  // M' = M * T(tx, ty)
  void Translate(Vector2 translation);
  void Translate(float tx, float ty);

  // M' = T(translation) * M
  void PostTranslate(Vector2 translation);
  void PostTranslate(float tx, float ty);

  static Affine MakeScale(float sx, float sy);

  void SetScale(float sx, float sy);
  void SetScale(float scale);

  // M' = M * S(sx, sy).
  void Scale(float sx, float sy);
  void Scale(float s);

  void FlipX();
  void FlipY();

  // M' = S(sx, sy) * M
  void PostScale(float sx, float sy);

  // M' = S(1/divx, 1/divy, 0, 0) * M
  bool PostIntDiv(int divx, int divy) WARN_UNUSED_RESULT;
  bool PostIntDiv(IntVector2 d) WARN_UNUSED_RESULT { return PostIntDiv(d.x, d.y); }

  void SetScale(float sx, float sy, Point2 pivot);
  void SetScale(float sx, float sy, float px, float py);

  void SetScaleTranslate(float sx, float sy, Vector2 translation);
  void SetScaleTranslate(float sx, float sy, float tx, float ty);

  bool SetBoundsToBounds(
      const Bounds2& src, const Bounds2& dst,
      ScaleToFit scale_to_fit = ScaleToFit::Fill);

  void SetRotate(double radians, Point2 pivot);
  void SetRotate(double radians, float px, float py);

  void SetRotate(double radians);

  // M' = M * R(angle) | counter-clockwise
  void Rotate(double radians);

  // M' = R(angle) * M | counter-clockwise
  void PostRotate(double radians);

  void SetmathSinCos(float sin_value, float cos_value, Point2 pivot);
  void SetmathSinCos(float sin_value, float cos_value, float px, float py);

  void SetmathSinCos(float sin_value, float cos_value);

  void SetShear(float kx, float ky);

  // M' = M * K(kx, ky)
  void Shear(float kx, float ky);

  void SetSkew(double ax, double ay, Point2 pivot);
  void SetSkew(double ax, double ay);

  // M' = M * K(ax, ay)
  void Skew(double ax, double ay);

  // M' = K(ax, ay) * M
  void PostSkew(double ax, double ay);

  void SkewX(double radians);
  void SkewY(double radians);

  // Set the matrix to the concatenation of the two specified matrices.
  // Either of the two matrices may also be the target matrix.
  // *this = lhs * rhs;
  void SetConcat(const Affine& lhs, const Affine& rhs);

  // M' = M * other
  void Concat(const Affine& other);

  // M' = other * M
  void PostConcat(const Affine& other);

  // Apply this matrix to the array of points specified by src, and write
  // the transformed points into the array of points specified by dst.
  // dst[] = M * src[]
  // |count| The number of points in src to read, and then transform into dst.
  void MapPoints(Point2 dst[], const Point2 src[], int count) const;

  Point2 MapPoint(Point2 p) const WARN_UNUSED_RESULT;
  Bounds2 MapBounds(const Bounds2& bounds) const WARN_UNUSED_RESULT;
  Quad2 MapBoundsAsQuad(const Bounds2& bounds) const WARN_UNUSED_RESULT;

  typedef void (*MapPointsProc)(const Affine& m, Point2 dst[], const Point2 src[], int count);
  MapPointsProc GetMapPointsProc() const { return MapPointsProcs[GetTransforms()]; }

  double GetDeterminant() const;

  // Returns true and an inverse of this (in |out_transform|) if the matrix is
  // non-singular.
  // Returns false (and does not touch out_transform) otherwise.
  bool GetInverted(Affine& out) const WARN_UNUSED_RESULT;
  bool IsInvertible() const;

  // Converts matrix to decomposed representation.
  // If conversion fails, returns false and leaves |decomposed| untouched.
  bool decompose(DecomposedAffine& out) const WARN_UNUSED_RESULT;

  // Decomposes magnitude of scale X and Y from transformation.
  // The sign is always positive and might be different than original.
  // Use decompose() if you need that.
  float DecomposeScaleMagX() const;
  float DecomposeScaleMagY() const;

  void Recompose(const DecomposedAffine& decomposed);

  float Get(int entry) const;
  void Set(int entry, float value);

  // Copy entries for this matrix into buffer, in the same order as the
  // EntryType enum.
  void Store(float data[EntryCount]) const;

  // Set this matrix to the entries from the buffer, in the same order as the
  // EntryType enum.
  void Load(float data[EntryCount]);

  Affine operator*(const Affine& rhs) const;
  float operator[](int entry) const { return Get(entry); }

  bool operator==(const Affine& other) const;
  bool operator!=(const Affine& other) const { return !operator==(other); }

  friend TextWriter& operator<<(TextWriter& out, const Affine& x) {
    x.formatImpl(out); return out;
  }
  friend void format(TextWriter& out, const Affine& x, const StringSpan& opts) {
    x.formatImpl(out);
  }

 private:
  static const MapPointsProc MapPointsProcs[];

  static bool TransformsAre(unsigned mask, unsigned compare);

  void InvalidateTypes() { type_mask_ = TypeMaskUnknown; }

  unsigned GetTypeMask() const;
  unsigned GetTypeMaskSlow() const;

  void FixTransBit();
  void FixScaleBit();

  void SetTransInternal(const Vector2& v);
  Vector2 GetTransInternal() const { return Vector2(d_[EntryTransX], d_[EntryTransY]); }

  void formatImpl(TextWriter& out) const;

  float d_[6]; // row-major
  mutable int type_mask_;
};

struct BASE_EXPORT DecomposedAffine {
  enum SkipInitialization { SkipInit };
  enum IdentityInitialization { InitWithIdentity };

  DecomposedAffine(SkipInitialization) {}
  DecomposedAffine(IdentityInitialization) { SetIdentity(); }

  void SetIdentity() { SetTranslate(Vector2()); }

  void SetTranslate(Vector2 d);

  Vector2 delta;
  float scale_x;
  float scale_y;
  double angle_radians;
  float remainder[4];
};

inline Affine Affine::operator*(const Affine& rhs) const {
  Affine result(SkipInit);
  result.SetConcat(*this, rhs);
  return result;
}

// Note: this call is expensive since we need to decompose the transform. If
// you're going to be calling this rapidly (e.g., in an animation) you should
// decompose once using Affine::Decompose and reuse your DecomposedAffine.
BASE_EXPORT bool Trylerp(Affine& out, const Affine& x, const Affine& y, double t);

BASE_EXPORT DecomposedAffine lerp(const DecomposedAffine& x, const DecomposedAffine& y, double t);

BASE_EXPORT bool isNear(const Affine& lhs, const Affine& rhs, float tolerance);

constexpr Affine::Affine(
    float scale_x, float shear_y,
    float shear_x, float scale_y,
    float trans_x, float trans_y)
    : d_ { scale_x, shear_y, shear_x, scale_y, trans_x, trans_y },
      type_mask_(TypeMaskUnknown) {
}

constexpr Affine::Affine(InitWithIdentityTag)
    : d_ { 1, 0, 0, 1, 0, 0 },
      type_mask_(TypeMaskRectStaysRect) {
}

inline bool Affine::IsScaleTranslate() const {
  return TransformsAre(GetTransforms(), TypeMaskTranslate | TypeMaskScale);
}

inline Affine Affine::MakeTranslate(Vector2 translation) {
  return MakeTranslate(translation.x, translation.y);
}

inline void Affine::SetTranslate(Vector2 translation) {
  SetTranslate(translation.x, translation.y);
}

inline float Affine::Get(int entry) const {
  ASSERT(0 <= entry && entry < EntryCount);
  return d_[entry];
}

inline void Affine::Set(int entry, float value) {
  ASSERT(0 <= entry && entry < EntryCount);
  d_[entry] = value;
  InvalidateTypes();
}

inline bool Affine::TransformsAre(unsigned mask, unsigned compare) {
  ASSERT(mask == (mask & TypeMaskAll));
  return (mask & ~compare) == 0;
}

inline unsigned Affine::GetTypeMask() const {
  unsigned mask;
  if (type_mask_ & TypeMaskUnknown)
    mask = GetTypeMaskSlow();
  else
    mask = type_mask_;
  ASSUME((type_mask_ & TypeMaskUnknown) == 0);
  return mask;
}

inline void DecomposedAffine::SetTranslate(Vector2 d) {
  delta = d;
  scale_x = 1;
  scale_y = 1;
  angle_radians = 0;
  remainder[0] = 1;
  remainder[1] = 0;
  remainder[2] = 0;
  remainder[3] = 1;
}

inline void Affine::SetTransInternal(const Vector2& v) {
  d_[EntryTransX] = v.x;
  d_[EntryTransY] = v.y;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_AFFINE_H_
