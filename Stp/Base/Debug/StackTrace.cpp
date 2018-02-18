// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Debug/Console.h"
#include "Base/Text/FormatMany.h"

namespace stp {

/**
 * @class StackTrace
 * A stacktrace can be helpful in debugging. For example, you can include a
 * stacktrace member in an object (probably around #ifndef NDEBUG) so that you
 * can later see where the given object was created from.
 */

StackTrace::StackTrace(void* const* trace, int count) {
  count = min(count, isizeofArray(trace_));
  if (count)
    UninitializedCopy(trace_, trace, count);
  count_ = count;
}

void* const* StackTrace::GetAddresses(int* count) const {
  *count = count_;
  if (count_)
    return trace_;
  return nullptr;
}

void StackTrace::PrintToConsole() const {
  format(Console::err(), StringSpan());
}

void StackTrace::FormatImpl(TextWriter& out, const StringSpan& opts) const {
  bool symbolize = true;
  if (!opts.IsEmpty()) {
    bool ok = opts.size() == 1;
    char format_char = opts[0];
    if (format_char == 'X')
      symbolize = false;
    else
      ok = format_char == 'S';
    if (!ok)
      throw FormatException("StackTrace");
  }

  if (symbolize)
    FormatSymbols(out);
  else
    FormatAddresses(out);
}

void StackTrace::FormatAddresses(TextWriter& out) const {
  for (int i = 0; i < count_; ++i)
    format(out, trace_[i], StringSpan());
}

#if OS(WIN) || OS(LINUX) || OS(DARWIN)
void StackTrace::FormatSymbols(TextWriter& out) const {
  for (int i = count_ - 1; i >= 0; --i) {
    out.format(" #{} {} in ", i, trace_[i]);
    FormatSymbol(out, trace_[i]);
    out.WriteLine();
  }
}
#endif

} // namespace stp
