// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_QUALIFIEDNAME_H_
#define STP_XML_QUALIFIEDNAME_H_

#include "Base/Containers/List.h"
#include "Xml/Export.h"

namespace stp {

class STP_XML_EXPORT QualifiedName {
 public:
  explicit QualifiedName(StringSpan qname);

  QualifiedName(StringSpan prefix, StringSpan local_name);

  ALWAYS_INLINE const String& value() const { return qname_; }

  StringSpan GetPefix() const {
    return name_offset_ == 0 ? StringSpan() : StringSpan(qname_.data(), name_offset_ - 1);
  }
  StringSpan GetLocalName() const {
    return StringSpan(qname_.data() + name_offset_, qname_.size() - name_offset_);
  }

 private:
  // "prefix:name"
  String qname_;
  int name_offset_;
};

class STP_XML_EXPORT QualifiedNameView {
 public:
  QualifiedNameView(const QualifiedName& qname)
      : local_name_(qname.GetLocalName()),
        prefix_(qname.GetPefix()) {}

  explicit QualifiedNameView(StringSpan qname);

  constexpr QualifiedNameView(StringSpan local_name, StringSpan prefix)
      : local_name_(local_name), prefix_(prefix) {}

  const StringSpan& GetLocalName() const { return local_name_; }
  const StringSpan& GetPrefix() const { return prefix_; }

 private:
  StringSpan local_name_;
  StringSpan prefix_;
};

inline bool operator==(const QualifiedNameView& lhs, const QualifiedNameView& rhs) {
  return lhs.GetLocalName() == rhs.GetLocalName() && lhs.GetPrefix() == rhs.GetPrefix();
}
inline bool operator!=(const QualifiedNameView& lhs, const QualifiedNameView& rhs) {
  return !(lhs == rhs);
}

} // namespace stp

#endif // STP_XML_QUALIFIEDNAME_H_
