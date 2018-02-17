// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLCONSTANTS_H_
#define STP_XML_XMLCONSTANTS_H_

#include "Base/Containers/Span.h"
#include "Xml/Export.h"

namespace stp {

constexpr StringSpan XmlPrefix = "xml";
constexpr StringSpan XmlNamespaceUri = "http://www.w3.org/XML/1998/namespace";

constexpr StringSpan XmlNsPrefix = "xmlns";
constexpr StringSpan XmlNsNamespaceUri = "http://www.w3.org/2000/xmlns";

constexpr StringSpan XLinkPrefix = "xlink";
constexpr StringSpan XLinkNamespaceURI = "http://www.w3.org/1999/xlink";
constexpr StringSpan XLinkHrefAttribute = "href";

} // namespace stp

#endif // STP_XML_XMLCONSTANTS_H_
