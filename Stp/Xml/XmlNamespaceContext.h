// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLNAMESPACECONTEXT_H_
#define STP_XML_XMLNAMESPACECONTEXT_H_

#include "Base/Containers/Span.h"
#include "Xml/Export.h"

namespace stp {

class STP_XML_EXPORT XmlNamespaceContext {
 public:
  virtual ~XmlNamespaceContext() {}

  virtual void PushContext() = 0;

  virtual void PopContext() = 0;

  virtual bool DeclarePrefix(StringSpan prefix, StringSpan uri) = 0;

  virtual StringSpan GetUri(StringSpan prefix) const = 0;

  virtual StringSpan GetPrefix(StringSpan uri) const = 0;

  virtual int GetDeclaredPrefixCount() const = 0;

  virtual StringSpan GetDeclaredPrefixAt(int index) = 0;

  virtual void Reset() = 0;
};

} // namespace stp

#endif // STP_XML_XMLNAMESPACECONTEXT_H_
