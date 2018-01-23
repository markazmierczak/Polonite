// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_MATHCONSTANTS_H_
#define STP_BASE_MATH_MATHCONSTANTS_H_

namespace stp {

template<typename T>
struct MathConstants {
  static constexpr T Pi = 3.141592653589793238462643383279502884+00;
  static constexpr T E = 2.718281828459045235360287471352662497e+00;

  static constexpr T Sqrt2 = 1.414213562373095048801688724209698078e+00;

  static constexpr T Log10E = 4.342944819032518276511289189166050822e-01;

  static constexpr T Ln2 = 6.931471805599453094172321214581765680e-01;
  static constexpr T Ln10 = 2.302585092994045684017991454684364207e+00;
};

} // namespace stp

#endif // STP_BASE_MATH_MATHCONSTANTS_H_
