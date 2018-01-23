// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLNAMESPACESUPPORT_H_
#define STP_XML_XMLNAMESPACESUPPORT_H_

#include "Base/Containers/List.h"
#include "Base/Containers/Stack.h"
#include "Xml/XmlNamespaceContext.h"

namespace stp {

class STP_XML_EXPORT XmlNamespaceSupport : public XmlNamespaceContext {
 public:
  XmlNamespaceSupport();
  ~XmlNamespaceSupport() override;

  void PushContext() override;
  void PopContext() override;
  bool DeclarePrefix(StringSpan prefix, StringSpan uri) override;
  StringSpan GetUri(StringSpan prefix) const override;
  StringSpan GetPrefix(StringSpan uri) const override;
  void Reset() override;
  int GetDeclaredPrefixCount() const override;
  StringSpan GetDeclaredPrefixAt(int index) override;

 private:
  // Prefix, URI pair.
  struct NamespaceTuple {
    NamespaceTuple() {}
    NamespaceTuple(StringSpan prefix, StringSpan uri)
        : prefix(prefix), uri(uri) {}

    String prefix;
    String uri;
  };

  void InitializeXmlns();

  List<NamespaceTuple> namespaces_;

  Stack<int> contexts_;
};

} // namespace stp

#endif // STP_XML_XMLNAMESPACESUPPORT_H_
