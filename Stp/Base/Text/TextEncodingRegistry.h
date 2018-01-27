// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_TEXTENCODINGREGISTRY_H_
#define STP_BASE_TEXT_TEXTENCODINGREGISTRY_H_

#include "Base/Containers/LinkedList.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

class TextEncodingProvider : public LinkedListNode<TextEncodingProvider> {
 public:
  virtual TextEncoding TryResolveByName(StringSpan name) = 0;

 protected:
  virtual ~TextEncodingProvider() {}
};

BASE_EXPORT void InstallTextEncodingProvider(TextEncodingProvider* provider);

BASE_EXPORT TextEncoding FindTextEncodingByName(StringSpan name);

} // namespace stp

#endif // STP_BASE_TEXT_TEXTENCODINGREGISTRY_H_
