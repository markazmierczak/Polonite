// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_ANGLE_H_
#define STP_BASE_GEOMETRY_ANGLE_H_

#include "Base/Export.h"
#include "Base/Math/Math.h"
#include "Base/Math/MathConstants.h"

namespace stp {

struct Vector3;

struct BASE_EXPORT Angle {
  static constexpr double Pi = MathConstants<double>::Pi;

  static constexpr double FullInDegrees = 360.0;
  static constexpr double StraightInDegrees = 180.0;
  static constexpr double RightInDegrees = 90.0;

  static constexpr double FullInRadians = Pi * 2;
  static constexpr double StraightInRadians = Pi;
  static constexpr double RightInRadians = Pi / 2;

  static constexpr double RadiansToDegrees(double rad) { return rad * (180.0 / Pi); }
  static constexpr double DegreesToRadians(double deg) { return deg * (Pi / 180.0); }

  static constexpr double GradientsToDegrees(double g) { return g * (360.0 / 400.0); }
  static constexpr double DegreesToGradients(double d) { return d * (400.0 / 360.0); }

  static constexpr double TurnsToDegrees(double t) { return t * 360.0; }
  static constexpr double DegreesToTurns(double d) { return d * (1 / 360.0); }

  static constexpr double TurnsToRadians(double t) { return t * (2 * Pi); }
  static constexpr double RadiansToTurns(double d) { return d * (1 / (2 * Pi)); }

  // Normalizes angle value to be in [0,360] range.
  static double NormalizeDegrees(double degrees);

  // Normalizes angle value to be in [0,2*PI] range.
  static double NormalizeRadians(double radians);

  static double BetweenInRadians(const Vector3& base, const Vector3& other);

  // Returns the clockwise angle between |base| and |other| where |normal| is the
  // normal of the virtual surface to measure clockwise according to.
  static double ClockwiseBetweenInRadians(
      const Vector3& base, const Vector3& other, const Vector3& normal);

  enum Unit : char {
    RadiansUnit = 'r',
    DegreesUnit = 'd',
    TurnsUnit = 't',
  };

  constexpr Angle(double value, Unit unit) : value(value), unit(unit) {}

  double InRadians() const;
  double InDegrees() const;
  double InTurns() const;
  double In(Unit unit) const;

  double value;
  Unit unit;
};

inline double Angle::NormalizeDegrees(double degrees) {
  degrees = IeeeRemainder(degrees, FullInDegrees);
  if (degrees < 0)
    degrees += FullInDegrees;

  return degrees;
}

inline double Angle::NormalizeRadians(double radians) {
  radians = IeeeRemainder(radians, FullInRadians);
  if (radians < 0)
    radians += FullInRadians;

  return radians;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_ANGLE_H_
