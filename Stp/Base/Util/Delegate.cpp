// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Delegate.h"

#include "Base/Debug/StackTrace.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Type/Formattable.h"

namespace stp {
namespace delegate_impl {

void FormatDelegate(TextWriter& out, const StringSpan& opts, void* ptr) {
  bool symbolize = true;
  if (!opts.isEmpty()) {
    bool ok = opts.size() == 1;

    char format_char = opts[0];
    if (format_char == 'X')
      symbolize = false;
    else
      ok = format_char == 'S';

    if (!ok)
      throw FormatException("Delegate");
  }

  if (symbolize)
    FormatSymbol(out, ptr);
  else
    format(out, ptr, StringSpan());
}

} // namespace delegate_impl
} // namespace stp
