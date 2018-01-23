// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_CUBICBEZIER_H_
#define STP_BASE_GEOMETRY_CUBICBEZIER_H_

#include "Base/Export.h"

namespace stp {

class BASE_EXPORT CubicBezier {
 public:
  CubicBezier(double x1, double y1, double x2, double y2)
      : x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}

  // Returns an approximation of y at the given x.
  double Solve(double x) const;

  // Returns an approximation of dy/dx at the given x.
  double GetSlope(double x) const;

  struct Range {
    double min;
    double max;

    void Unpack(double& out_min, double& out_max) const {
      out_min = min; out_max = max;
    }
  };
  Range GetRange() const;

  double x1() const { return x1_; }
  double x2() const { return x2_; }
  double y1() const { return y1_; }
  double y2() const { return y2_; }

  bool operator==(const CubicBezier& o) const;
  bool operator!=(const CubicBezier& o) const { return !operator==(o); }

 private:
  double x1_;
  double y1_;
  double x2_;
  double y2_;
};

inline bool CubicBezier::operator==(const CubicBezier& o) const {
  return x1_ == o.x1_ && y1_ == o.y1_ && x2_ == o.x2_ && y2_ == o.y2_;
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_CUBICBEZIER_H_
