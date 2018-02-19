// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONOPTIONS_H_
#define STP_BASE_JSON_JSONOPTIONS_H_

#include "Base/Containers/Span.h"

namespace stp {

class BASE_EXPORT JsonOptions {
 public:
  enum Option {
    // Common:
    EnableInfNaN            = 1 << 0,

    // Parser-only:
    AllowTrailingCommas     = 1 << 1,
    ReferenceInput          = 1 << 2,
    UniqueKeys              = 1 << 3,

    // Formatter-only:
    PrettyFormatting        = 1 << 4,
    EmitTrailingCommas      = AllowTrailingCommas,
    EscapeUnicode           = 1 << 5,
    DisallowLossOfPrecision = 1 << 6,
    TryIntegerForFloat      = 1 << 7,
    BreakOnError            = 1 << 8,
  };

  static JsonOptions Parse(StringSpan string);

  JsonOptions() {}

  void add(Option option) { Set(option, true); }

  void Set(Option option, bool state);

  bool Has(Option option) const { return (bits_ & option) != 0; }

  void SetDepthLimit(int limit) { depth_limit_ = limit; }
  int GetDepthLimit() const { return depth_limit_; }

 private:
  int bits_ = 0;

  int depth_limit_ = 100;
};

inline void JsonOptions::Set(Option option, bool state) {
  if (state)
    bits_ |= option;
  else
    bits_ &= ~option;
}

} // namespace stp

#endif // STP_BASE_JSON_JSONOPTIONS_H_
