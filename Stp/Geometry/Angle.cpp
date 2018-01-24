// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Geometry/Angle.h"

#include "Base/Error/BasicExceptions.h"
#include "Base/Io/TextWriter.h"

namespace stp {

void Angle::ToFormat(TextWriter& out, const StringSpan& opts) const {
  auto output_unit = DegreesUnit;
  if (!opts.IsEmpty()) {
    bool ok = opts.size() == 1;
    switch (opts[0]) {
      case 'd': case 'D':
        output_unit = DegreesUnit;
        break;
      case 'r': case 'R':
        output_unit = RadiansUnit;
        break;
      case 't': case 'T':
        output_unit = TurnsUnit;
        break;
      default:
        ok = false;
        break;
    }
    if (!ok)
      throw FormatException("Angle");
  }

  out.WriteFloat(In(output_unit));
  switch (output_unit) {
    case RadiansUnit:
      out.WriteAscii("rad");
      break;
    case DegreesUnit:
      out.Write(U'°');
      break;
    case TurnsUnit:
      out.WriteAscii("turns");
      break;
  }
}

double Angle::In(Unit requested) const {
  if (requested == unit)
    return value;

  switch (requested) {
    case RadiansUnit:
      switch (unit) {
        case RadiansUnit:
          break;
        case DegreesUnit:
          return DegreesToRadians(value);
        case TurnsUnit:
          return TurnsToRadians(value);
      }
      break;
    case DegreesUnit:
      switch (unit) {
        case RadiansUnit:
          return RadiansToDegrees(value);
        case DegreesUnit:
          break;
        case TurnsUnit:
          return TurnsToDegrees(value);
      }
      break;
    case TurnsUnit:
      switch (unit) {
        case RadiansUnit:
          return RadiansToTurns(value);
        case DegreesUnit:
          return DegreesToTurns(value);
        case TurnsUnit:
          break;
      }
      break;
  }
  UNREACHABLE(return 0);
}

} // namespace stp
