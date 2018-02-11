// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

namespace stp {

void FormatSymbol(TextWriter& out, void *pc) {
  Dl_info info;
  if (dladdr(pc, &info))
    DemangleSymbols(out, MakeSpanFromNullTerminated(info.dli_sname));
  else
    out << "(no symbol)";
}

} // namespace stp
