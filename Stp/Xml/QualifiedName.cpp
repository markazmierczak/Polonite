// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Xml/QualifiedName.h"

namespace stp {

// name offset will wrap to 0 if no prefix
QualifiedName::QualifiedName(StringSpan qname)
    : qname_(qname),
      name_offset_(qname.IndexOf(':') + 1) {}

QualifiedName::QualifiedName(StringSpan prefix, StringSpan local_name) {
  if (prefix.IsEmpty()) {
    qname_ = local_name;
    name_offset_ = 0;
  } else {
    qname_.EnsureCapacity(prefix.size() + 1 + local_name.size());
    qname_.Append(prefix);
    qname_.Append(':');
    qname_.Append(local_name);
    name_offset_ = prefix.size() + 1;
  }
}

QualifiedNameView::QualifiedNameView(StringSpan qname) {
  int colon = qname.IndexOf(':');
  if (colon < 0) {
    prefix_ = StringSpan();
    local_name_ = qname;
  } else {
    prefix_ = StringSpan(qname.data(), colon);
    colon++;
    local_name_ = StringSpan(qname.data() + colon, qname.size() - colon);
  }
}

} // namespace stp
