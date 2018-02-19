// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Environment.h"

#include "Base/Io/StringWriter.h"
#include "Base/Type/ParseFloat.h"
#include "Base/Text/Utf.h"
#include "Base/Type/ParseInteger.h"

namespace stp {

template<typename T>
static bool tryGetIntegerTmpl(StringSpan name, T& out_value) {
  ASSERT(out_value);
  String str;
  if (!Environment::tryGet(name, str))
    return false;
  return tryParse(str, out_value) == ParseIntegerErrorCode::Ok;
}

bool Environment::tryGet(StringSpan name, int& out_value) {
  return tryGetIntegerTmpl(name, out_value);
}
bool Environment::tryGet(StringSpan name, int64_t& out_value) {
  return tryGetIntegerTmpl(name, out_value);
}

bool Environment::tryGet(StringSpan name, double& out_value) {
  ASSERT(out_value);

  String str;
  if (!tryGet(name, str))
    return false;

  return tryParse(str, out_value);
}

} // namespace stp
