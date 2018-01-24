// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_XFORM3_H_
#define STP_BASE_GEOMETRY_XFORM3_H_

#include "Base/Debug/Assert.h"
#include "Geometry/Angle.h"
#include "Geometry/Limits.h"
#include "Geometry/Quaternion.h"
#include "Geometry/Vector3.h"

#include <string.h>

namespace stp {

struct Bounds2;
struct DecomposedXform3;
struct Quad2;
struct Quaternion;
struct Vector3;
class Affine;
class Xform2;

// 4x4 transformation matrix.
class BASE_EXPORT Xform3 {
 public:
  enum SkipInitTag { SkipInit };
  enum InitWithIdentityTag { InitWithIdentity };

  enum EntryType {
    EntryScaleX = 0,
    EntryShearY = 1,
    EntryPersp0 = 3,
    EntryShearX = 4,
    EntryScaleY = 5,
    EntryPersp1 = 7,
    EntryScaleZ = 10,
    EntryPersp2 = 11,
    EntryTransX = 12,
    EntryTransY = 13,
    EntryTransZ = 14,
  };

  // Type mask consists of two nibbles.
  // Lower nibble tracks actual transformations in the matrix.
  // Higher nibble marks bits that need update (are dirty).
  enum TypeMask : unsigned {
    TypeMaskDirtyShift = 4,

    TypeMaskTranslate    = 0x01,  //!< set if the matrix has translation
    TypeMaskScale        = 0x02,  //!< set if the matrix has any scale != 1
    TypeMaskAffine       = 0x04,  //!< set if the matrix skews or rotates
    TypeMaskPerspective  = 0x08,  //!< set if the matrix is in perspective
    TypeMaskAll          = 0x0F,

    TypeMaskIdentity     = 0,
    TypeMaskUnknown      = TypeMaskAll << TypeMaskDirtyShift,
  };

  static constexpr int RowCount = 4;
  static constexpr int ColCount = 4;
  static constexpr int EntryCount = RowCount * ColCount;

  enum class ScaleToFit {
    Fill,
    Start,
    Center,
    End
  };

  // Skips initializing this matrix to avoid overhead, when we know it will be
  // initialized before use.
  explicit Xform3(SkipInitTag) {}

  constexpr explicit Xform3(InitWithIdentityTag);

  // Constructs a transform from explicit 16 matrix elements. Elements
  // should be given in row-major order.
  constexpr Xform3(
      float col1row1, float col2row1, float col3row1, float col4row1,
      float col1row2, float col2row2, float col3row2, float col4row2,
      float col1row3, float col2row3, float col3row3, float col4row3,
      float col1row4, float col2row4, float col3row4, float col4row4);

  // Constructs a transformation from explicit 2D elements. All other matrix
  // elements remain the same as the corresponding elements of an identity matrix.
  // NOTE: The order is on column-major order instead of row-major to be compliant with
  //       Web standards. https://www.w3.org/TR/css-transforms-1/#MatrixDefined
  Xform3(
      float scale_x, float skew_y,
      float skew_x, float scale_y,
      float trans_x, float trans_y);

  unsigned GetType() const;

  static constexpr Xform3 Identity() { return Xform3(InitWithIdentity); }

  // Returns true if this is the identity matrix.
  bool IsIdentity() const { return GetType() == TypeMaskIdentity; }

  // Returns true if the matrix is either identity or translation.
  bool IsTranslate() const { return (GetType() & ~TypeMaskTranslate) == 0; }

  // Returns true if the matrix is either the identity or a 2D translation.
  bool IsTranslate2D() const { return IsTranslate() && Get(2, 3) == 0; }

  // Returns true if the matrix is either identity or pure integer translation,
  // allowing for an amount of inaccuracy as specified by the parameter.
  bool IsIntegerTranslate(float tolerance = 1E-8) const;

  // Returns true if the matrix is either identity or translation,
  // allowing for an amount of inaccuracy as specified by the parameter.
  bool IsNearTranslate(float tolerance) const;

  // Returns true if the matrix had only scaling components.
  bool IsScale() const { return (GetType() & ~TypeMaskScale) == 0; }
  bool IsScale2D() const;

  // Returns true if the matrix has only scaling and translation.
  bool IsScaleTranslate() const;
  bool IsScaleTranslate2D() const;

  // Returns true if the matrix has only scaling and integer translation.
  bool IsScaleIntegerTranslate(float tolerance) const;

  // Returns true if the matrix has any perspective component that would
  // change the w-component of a homogeneous point.
  bool HasPerspective() const { return (GetType() & TypeMaskPerspective) != 0; }

  // Resets this transform to the identity transform.
  void SetIdentity() { *this = Identity(); }

  // Set the matrix to translate by (delta).
  void SetTranslate(Vector3 d) { SetTranslate(d.x, d.y, d.z); }
  void SetTranslate(float dx, float dy, float dz);
  void SetTranslate2D(Vector2 d) { SetTranslate2D(d.x, d.y); }
  void SetTranslate2D(float dx, float dy);

  // Preconcats the matrix with the specified translation.
  // M' = M * T(dx, dy, dz)
  void Translate(Vector3 d) { Translate(d.x, d.y, d.z); }
  void Translate(float dx, float dy, float dz);
  void Translate2D(Vector2 d) { Translate2D(d.x, d.y); }
  void Translate2D(float dx, float dy);

  void TranslateXAxis(float dx);
  void TranslateYAxis(float dy);
  void TranslateZAxis(float dz);

  // Postconcats the matrix with the specified translation.
  // M' = T(delta) * M
  void PostTranslate(Vector3 d) { PostTranslate(d.x, d.y, d.z); }
  void PostTranslate(float dx, float dy, float dz);
  void PostTranslate2D(Vector2 d) { PostTranslate2D(d.x, d.y); }
  void PostTranslate2D(float dx, float dy);

  // Set the matrix to scale by sx, sy, sz.
  void SetScale(float sx, float sy, float sz);
  void SetScale2D(float sx, float sy);

  // Applies the current transformation on a scaling and assigns the result to |this|.
  void Scale(float sx, float sy, float sz);
  void Scale2D(float sx, float sy) { Scale(sx, sy, 1); }
  void Scale2D(float scale) { Scale(scale, scale, 1); }

  void ScaleXAxis(float sx);
  void ScaleYAxis(float sy);
  void ScaleZAxis(float sz);

  // Postconcats the matrix with the specified scale.
  // M' = S(sx, sy, sz) * M
  void PostScale(float sx, float sy, float sz);
  void PostScale2D(float sx, float sy);

  void SetScaleTranslate(float sx, float sy, float sz, float tx, float ty, float tz);
  void SetScaleTranslate2D(float sx, float sy, float tx, float ty);

  bool SetBoundsToBounds(
      const Bounds2& src, const Bounds2& dst,
      ScaleToFit scale_to_fit = ScaleToFit::Fill);

  void SetOrthoProjection(
      const Bounds2& bounds,
      bool flip_y = false,
      float near_plane = -1.f, float far_plane = 1.f);

  void SetOrthoProjectionFlat(const Bounds2& bounds, bool flip_y = false);

  void SetRotate2D(double radians);

  void SetRotate(const Quaternion& quaternion);

  void SetRotateAbout(Vector3 direction, double radians);
  void SetRotateAbout(float x, float y, float z, double radians);

  // Rotate about the vector [x,y,z]. Assuming direction is unit-length.
  void SetRotateAboutUnit(Vector3 direction, double radians);
  void SetRotateAboutUnit(float x, float y, float z, double radians);

  void Rotate(const Quaternion& quaternion);

  bool RotateAbout(const Vector3& axis, double radians);
  bool RotateAbout(float x, float y, float z, double radians);

  // The |axis| vector must be unit-length.
  void RotateAboutUnit(const Vector3& axis, double radians);
  void RotateAboutUnit(float x, float y, float z, double radians);

  void RotateAboutXAxis(double radians);
  void RotateAboutYAxis(double radians);
  void RotateAboutZAxis(double radians);

  // Applies the current transformation on a 2D rotation and assigns the result to |this|.
  void Rotate2D(double radians) { RotateAboutZAxis(radians); }

  void SetShear(float kx, float ky);

  void Shear(float kx, float ky);

  // Preconcats the matrix with the specified skew.
  // M' = M * K(ax, ay)
  void Skew(double ax, double ay);
  void SkewX(double ax);
  void SkewY(double ay);
  void SkewRadians(double ax, double ay);
  void SkewXRadians(double ax);
  void SkewYRadians(double ay);

  void SetAffine(
      float scale_x, float skew_y,
      float skew_x, float scale_y,
      float trans_x, float trans_y);

  void SetAffine(const Affine& affine);

  void SetXform2d(const Xform2& xform);

  // This sets the top-left of the matrix and clears the translation and
  // perspective components (with [3][3] set to 1).
  void Set3x3(
      float col1row1, float col2row1, float col3row1,
      float col1row2, float col2row2, float col3row2,
      float col1row3, float col2row3, float col3row3);

  // Applies the current transformation on a perspective transform and assigns
  // the result to |this|.
  // The depth, given as the parameter to the function, represents the distance
  // of the z=0 plane from the viewer. Lower values give a more flattened
  // pyramid and therefore a more pronounced perspective effect.
  // The value is given in pixels, so a value of 1000 gives a moderate amount of
  // foreshortening and a value of 200 gives an extreme amount.
  void ApplyPerspectiveDepth(float depth);

  // Set perspective frustum projection,
  // |left| |bottom| - |near_plane| and |right| |top| - |near_plane| specify the points
  // on the near clipping plane that are mapped to the lower-left and upper-right corners
  // of the window, assuming that the eye is located at (0,0,0). - |far_plane| specifies
  // the location of the far clipping plane.
  void SetFrustum(
      float left, float right, float top, float bottom,
      float near_plane, float far_plane);

  void SetPerspective(
      double fov_radians, float aspect_ratio,
      float near_plane, float far_plane);

  void SetLookAt(const Vector3& eye, const Vector3& origin, const Vector3& up);

  void SetConcat(const Xform3& lhs, const Xform3& rhs);

  // Preconcats the matrix with the specified matrix.
  // M' = M * other
  void Concat(const Xform3& other);

  // Postconcats the matrix with the specified matrix.
  // M' = other * M
  void PostConcat(const Xform3& other);

  // Returns true if axis-aligned 2D rects will remain axis-aligned after being
  // transformed by this matrix.
  //
  // A 3D rotation through 90 degrees into a perpendicular plane collapses a square
  // to a line, but is still considered to be axis-aligned.
  //
  // By default, tolerates very slight error due to float imprecisions;
  // a 90-degree rotation can still end up with 10^-17 of "non-axis-aligned" result.
  bool Preserves2DAxisAlignment(float epsilon = NearlyZeroForGraphics<float>) const;

  // Returns true if a layer with a forward-facing normal of (0, 0, 1) would
  // have its back side facing frontwards after applying the transform.
  bool IsBackFaceVisible() const;

  // Returns true and an inverse of this (in |inverted|) if the matrix is
  // non-singular.
  // Returns false (and does not touch |inverted|) otherwise.
  bool GetInverted(Xform3& out) const WARN_UNUSED_RESULT;

  // Returns true if this transform is non-singular.
  bool IsInvertible() const;

  // Transposes this transform in place.
  void Transpose();

  // Set 3rd row and 3rd colum to (0, 0, 1, 0). Note that this flattening
  // operation is not quite the same as an orthographic projection and is
  // technically not a linear operation.
  //
  // One useful interpretation of doing this operation:
  //  - For x and y values, the new transform behaves effectively like an
  //    orthographic projection was added to the matrix sequence.
  //  - For z values, the new transform overrides any effect that the transform
  //    had on z, and instead it preserves the z value for any points that are
  //    transformed.
  //  - Because of linearity of transforms, this flattened transform also
  //    preserves the effect that any subsequent (multiplied from the right)
  //    transforms would have on z values.
  //
  void FlattenTo2D();
  Xform2 GetFlattenedTo2DAsXform2d() const;
  Affine GetFlattenedTo2DAsAffine() const;

  // Returns true if the 3rd row and 3rd column are both (0, 0, 1, 0).
  bool IsFlat() const;

  void MapPoints(Point3 dst[], const Point3 src[], int count) const;

  Point3 MapPoint(Point3 p) const WARN_UNUSED_RESULT;

  void MapMatrix4x1(float dst[4], const float src[4]) const;

  double GetDeterminant() const;

  // Converts matrix to decomposed representation.
  // If conversion fails, returns false and leaves |decomposed| untouched.
  bool Decompose(DecomposedXform3& decomposed) const WARN_UNUSED_RESULT;

  void Recompose(const DecomposedXform3& decomposed);

  static Xform3 FromDecomposed(const DecomposedXform3& decomposed);

  float trans_x() const { return GetEntry(EntryTransX); }
  float trans_y() const { return GetEntry(EntryTransY); }
  float trans_z() const { return GetEntry(EntryTransZ); }

  float GetEntry(EntryType type) const;

  void Set(int row, int col, float val);
  float Get(int row, int col) const;

  bool TriviallyIsIdentity() const { return type_mask_ == TypeMaskIdentity; }
  bool TriviallyIsTranslate() const { return TriviallyIsType(TypeMaskTranslate); }
  bool TriviallyIsScaleTranslate() const;
  bool TriviallyHasPerspective() const;

  Xform3 operator*(const Xform3& other) const;
  Xform3& operator*=(const Xform3& other) { Concat(other); return *this; }

  bool operator==(const Xform3& rhs) const;
  bool operator!=(const Xform3& rhs) const { return !operator==(rhs); }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

 private:
  float d_[ColCount][RowCount];
  mutable unsigned type_mask_;

  unsigned GetTypeSlow() const;

  bool TriviallyIsType(unsigned mask) const {
    mask |= mask << TypeMaskDirtyShift;
    return (type_mask_ & ~mask) == 0;
  }

  void DirtyTypeMask(unsigned mask = TypeMaskAll) {
    ASSERT(mask != 0 && !(mask & ~TypeMaskAll));
    type_mask_ |= mask << TypeMaskDirtyShift;
  }

  void SetTypeMask(unsigned mask) {
    ASSERT(0 == (~(TypeMaskAll | TypeMaskUnknown) & mask));
    type_mask_ = mask;
  }

  void SetDirtyTypeMask(unsigned dirty_mask) {
    ASSERT(dirty_mask != TypeMaskIdentity && !(dirty_mask & ~TypeMaskAll));
    type_mask_ = dirty_mask << TypeMaskDirtyShift;
  }

  bool HasIntegerTranslate(float tolerance) const;

  Vector3 GetTransInternal() const {
    return Vector3(GetEntry(EntryTransX), GetEntry(EntryTransY), GetEntry(EntryTransZ));
  }
};

// Decomposes |this| and |from|, interpolates the decomposed values, and
// sets |this| to the reconstituted result. Returns false if either matrix
// can't be decomposed. Uses routines described in this spec:
// http://www.w3.org/TR/css3-3d-transforms/.
//
// Note: this call is expensive since we need to decompose the transform. If
// you're going to be calling this rapidly (e.g., in an animation) you should
// decompose once using DecomposeTransforms and reuse your
// DecomposedTransform.
BASE_EXPORT bool TryLerp(Xform3& out, const Xform3& x, const Xform3& y, double t);

BASE_EXPORT bool IsNear(const Xform3& lhs, const Xform3& rhs, float tolerance = 0.1f);

BASE_EXPORT bool IsFinite(const Xform3& xform);

// Contains the components of a factored transform. These components may be
// blended and recomposed.
struct BASE_EXPORT DecomposedXform3 {
  enum SkipInitialization { SkipInit };
  enum IdentInitialization { InitWithIdentity };

  explicit DecomposedXform3(SkipInitialization) : quaternion(Quaternion::SkipInit) {}

  explicit DecomposedXform3(IdentInitialization);

  void SetIdentity();

  void SetScaleTranslate(
      float sx, float sy, float sz,
      float tx, float ty, float tz);

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  Vector3 translate;
  float scale[3];
  float shear[3];
  float perspective[4];
  Quaternion quaternion;
};

// Interpolates the decomposed components |to| with |from| using the
// routines described in http://www.w3.org/TR/css3-3d-transform/.
// |progress| is in the range [0, 1] (0 leaves |out| unchanged, and 1
// assigns |from| to |out|).
BASE_EXPORT DecomposedXform3 Lerp(
    const DecomposedXform3& from, const DecomposedXform3& to,
    double progress);

constexpr Xform3::Xform3(InitWithIdentityTag)
    : d_ { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } },
      type_mask_(TypeMaskIdentity) {
}

constexpr Xform3::Xform3(
    float col1row1, float col2row1, float col3row1, float col4row1,
    float col1row2, float col2row2, float col3row2, float col4row2,
    float col1row3, float col2row3, float col3row3, float col4row3,
    float col1row4, float col2row4, float col3row4, float col4row4)
    : d_{{col1row1, col1row2, col1row3, col1row4},
         {col2row1, col2row2, col2row3, col2row4},
         {col3row1, col3row2, col3row3, col3row4},
         {col4row1, col4row2, col4row3, col4row4}},
      type_mask_(TypeMaskUnknown) {
}

inline unsigned Xform3::GetType() const {
  return (type_mask_ & TypeMaskUnknown) == 0 ? type_mask_ : GetTypeSlow();
}

inline bool Xform3::IsScaleTranslate() const {
  return (GetType() & ~(TypeMaskScale | TypeMaskTranslate)) == 0;
}

inline bool Xform3::IsScaleTranslate2D() const {
  return IsScaleTranslate() && Get(2, 3) == 0 && Get(2, 2) == 1;
}

inline void Xform3::SetRotateAbout(Vector3 direction, double radians) {
  SetRotateAbout(direction.x, direction.y, direction.z, radians);
}

inline void Xform3::SetRotateAboutUnit(Vector3 direction, double radians) {
  SetRotateAboutUnit(direction.x, direction.y, direction.z, radians);
}

inline void Xform3::SetAffine(
    float scale_x, float skew_y,
    float skew_x, float scale_y,
    float trans_x, float trans_y) {
  d_[0][0] = scale_x;
  d_[1][0] = skew_x;
  d_[2][0] = 0;
  d_[3][0] = trans_x;

  d_[0][1] = skew_y;
  d_[1][1] = scale_y;
  d_[2][1] = 0;
  d_[3][1] = trans_y;

  d_[0][2] = 0;
  d_[1][2] = 0;
  d_[2][2] = 1;
  d_[3][2] = 0;

  d_[0][3] = 0;
  d_[1][3] = 0;
  d_[2][3] = 0;
  d_[3][3] = 1;

  SetDirtyTypeMask(TypeMaskAffine | TypeMaskScale | TypeMaskTranslate);
}

inline Xform3 Xform3::FromDecomposed(const DecomposedXform3& decomp) {
  Xform3 xform(Xform3::SkipInit);
  xform.Recompose(decomp);
  return xform;
}

inline Xform3 Xform3::operator*(const Xform3& other) const {
  Xform3 out(SkipInit);
  out.SetConcat(*this, other);
  return out;
}

inline float Xform3::GetEntry(EntryType type) const {
  auto t = static_cast<unsigned>(type);
  return Get(t & 3, t >> 2);
}

inline void Xform3::Set(int row, int col, float val) {
  ASSERT(static_cast<unsigned>(row) < RowCount);
  ASSERT(static_cast<unsigned>(col) < ColCount);

  d_[col][row] = val;

  if (row == 3)
    DirtyTypeMask();
  else
    DirtyTypeMask(TypeMaskAffine | TypeMaskScale | TypeMaskTranslate);
}

inline float Xform3::Get(int row, int col) const {
  ASSERT(static_cast<unsigned>(row) < RowCount);
  ASSERT(static_cast<unsigned>(col) < ColCount);
  return d_[col][row];
}

inline bool Xform3::TriviallyIsScaleTranslate() const {
  return TriviallyIsType(TypeMaskTranslate | TypeMaskScale);
}

inline bool Xform3::TriviallyHasPerspective() const {
  constexpr unsigned mask = TypeMaskPerspective | (TypeMaskPerspective << TypeMaskDirtyShift);
  return (type_mask_ & mask) != 0;
}

inline DecomposedXform3::DecomposedXform3(IdentInitialization)
    : scale { 1, 1, 1 },
      shear { 0, 0, 0 },
      perspective { 0, 0, 0, 1 },
      quaternion(Quaternion::InitWithIdentity) {
}

inline void DecomposedXform3::SetIdentity() {
  translate = Vector3();
  scale[0] = scale[1] = scale[2] = 1;
  shear[0] = shear[1] = shear[2] = 0;
  perspective[0] = perspective[1] = perspective[2] = 0;
  perspective[3] = 1;
  quaternion.SetIdentity();
}

inline void DecomposedXform3::SetScaleTranslate(
    float sx, float sy, float sz,
    float tx, float ty, float tz) {
  SetIdentity();
  scale[0] = sx;
  scale[1] = sy;
  scale[2] = sz;
  translate = Vector3(tx, ty, tz);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_XFORM3_H_
