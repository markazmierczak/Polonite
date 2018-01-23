// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_SYSTEMSTRING_H_
#define STP_BASE_TEXT_SYSTEMSTRING_H_

#include "Base/Compiler/Os.h"
#include "Base/Containers/List.h"

#if OS(DARWIN)
#include "Base/Darwin/CoreFoundation.h"
OBJC_CLASS NSString;
#endif

namespace stp {

#if OS(DARWIN)

BASE_EXPORT CFStringRef ToCFStringRef(StringSpan utf8);
BASE_EXPORT CFStringRef ToCFStringRef(String16Span utf16);

BASE_EXPORT NSString* ToNSString(StringSpan utf8);
BASE_EXPORT NSString* ToNSString(String16Span utf16);

BASE_EXPORT String ToString(CFStringRef ref);
BASE_EXPORT String16 ToString16(CFStringRef ref);

BASE_EXPORT String ToString(NSString* ref);
BASE_EXPORT String16 ToString16(NSString* ref);

#endif // OS(*)

} // namespace stp

#endif // STP_BASE_TEXT_SYSTEMSTRING_H_
