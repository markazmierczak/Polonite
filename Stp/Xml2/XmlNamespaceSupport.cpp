// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Xml/XmlNamespaceSupport.h"

#include "Xml/XmlConstants.h"

namespace stp {

XmlNamespaceSupport::XmlNamespaceSupport() {
  contexts_.EnsureCapacity(8);
  namespaces_.EnsureCapacity(16);

  InitializeXmlns();
}

XmlNamespaceSupport::~XmlNamespaceSupport() {
}

void XmlNamespaceSupport::PushContext() {
  contexts_.Push(namespaces_.size());
}

void XmlNamespaceSupport::PopContext() {
  namespaces_.Truncate(contexts_.Pop());
}

bool XmlNamespaceSupport::DeclarePrefix(StringSpan prefix, StringSpan uri) {
  if (prefix == XmlPrefix || prefix == XmlNsPrefix)
    return false;

  int begin = contexts_.Peek();
  for (int i = namespaces_.size() - 1; i >= begin; --i) {
    if (namespaces_[i].prefix == prefix) {
      namespaces_[i].uri = uri;
      return true;
    }
  }

  namespaces_.Add(NamespaceTuple(prefix, uri));
  return true;
}

StringSpan XmlNamespaceSupport::GetUri(StringSpan prefix) const {
  for (int i = namespaces_.size() - 1; i >= 0; --i) {
    if (namespaces_[i].prefix == prefix)
      return namespaces_[i].uri;
  }
  return StringSpan();
}

StringSpan XmlNamespaceSupport::GetPrefix(StringSpan uri) const {
  for (int i = namespaces_.size() - 1; i >= 0; --i) {
    if (namespaces_[i].uri == uri) {
      if (GetUri(namespaces_[i].prefix) == uri)
        return namespaces_[i].prefix;
    }
  }
  return StringSpan();
}

int XmlNamespaceSupport::GetDeclaredPrefixCount() const {
  return namespaces_.size() - contexts_.Peek();
}

StringSpan XmlNamespaceSupport::GetDeclaredPrefixAt(int index) {
  index += contexts_.Peek();
  ASSERT(index < namespaces_.size());
  return namespaces_[index].prefix;
}

void XmlNamespaceSupport::Reset() {
  namespaces_.Clear();
  contexts_.Clear();

  InitializeXmlns();
}

void XmlNamespaceSupport::InitializeXmlns() {
  namespaces_.Add(NamespaceTuple(XmlPrefix, XmlNamespaceUri));
  namespaces_.Add(NamespaceTuple(XmlNsPrefix, XmlNsNamespaceUri));

  contexts_.Push(namespaces_.size());
}

} // namespace stp
