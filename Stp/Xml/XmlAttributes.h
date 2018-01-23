// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLATTRIBUTES_H_
#define STP_XML_XMLATTRIBUTES_H_

#include "Base/Type/Limits.h"
#include "Xml/QualifiedName.h"

namespace stp {

struct XmlAttribute {
  StringSpan uri;
  StringSpan local_name;
  StringSpan value;
};

class STP_XML_EXPORT XmlAttributes {
 public:
  virtual QualifiedNameView GetQName(int index) const = 0;
  virtual StringSpan GetLocalName(int index) const = 0;
  virtual StringSpan GetUri(int index) const = 0;
  virtual StringSpan GetValue(int index) const = 0;

  virtual XmlAttribute GetTuple(int index) const = 0;

  int size() const { return size_; }

  virtual bool GetValue(QualifiedNameView qname, StringSpan* value) const = 0;

  virtual bool GetValue(
      StringSpan uri, StringSpan local_name, StringSpan* value) const = 0;

 protected:
  explicit XmlAttributes(int size = 0) : size_(size) {}
  virtual ~XmlAttributes() {}

  void set_size(int size) { size_ = size; }

 private:
  int size_;
};

} // namespace stp

#endif // STP_XML_XMLATTRIBUTES_H_
