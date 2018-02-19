// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonOptions.h"

#include "Base/Error/BasicExceptions.h"

namespace stp {

JsonOptions JsonOptions::Parse(StringSpan string) {
  JsonOptions options;
  for (; !string.isEmpty(); string.removePrefix(1)) {
    switch (string[0]) {
      case 'R':
        options.add(ReferenceInput);
        break;
      case 'C':
        static_assert(AllowTrailingCommas == EmitTrailingCommas, "!");
        options.add(AllowTrailingCommas);
        break;
      case 'K':
        options.add(UniqueKeys);
        break;
      case 'N':
        options.add(EnableInfNaN);
        break;
      case 'P':
        options.add(PrettyFormatting);
        break;
      case 'U':
        options.add(EscapeUnicode);
        break;
      case 'L':
        options.add(DisallowLossOfPrecision);
        break;
      case 'I':
        options.add(TryIntegerForFloat);
        break;
      case 'E':
        options.add(BreakOnError);
        break;

      default:
        throw FormatException("Json");
    }
  }
  return options;
}

} // namespace stp
