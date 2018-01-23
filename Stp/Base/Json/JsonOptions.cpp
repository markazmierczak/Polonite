// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Json/JsonOptions.h"

#include "Base/Error/BasicExceptions.h"

namespace stp {

JsonOptions JsonOptions::Parse(StringSpan string) {
  JsonOptions options;
  for (; !string.IsEmpty(); string.RemovePrefix(1)) {
    switch (string[0]) {
      case 'R':
        options.Add(ReferenceInput);
        break;
      case 'C':
        static_assert(AllowTrailingCommas == EmitTrailingCommas, "!");
        options.Add(AllowTrailingCommas);
        break;
      case 'K':
        options.Add(UniqueKeys);
        break;
      case 'N':
        options.Add(EnableInfNaN);
        break;
      case 'P':
        options.Add(PrettyFormatting);
        break;
      case 'U':
        options.Add(EscapeUnicode);
        break;
      case 'L':
        options.Add(DisallowLossOfPrecision);
        break;
      case 'I':
        options.Add(TryIntegerForFloat);
        break;
      case 'E':
        options.Add(BreakOnError);
        break;

      default:
        throw FormatException("Json");
    }
  }
  return options;
}

} // namespace stp
