// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLCONTENTHANDLER_H_
#define STP_XML_XMLCONTENTHANDLER_H_

#include "Xml/QualifiedName.h"

namespace stp {

class XmlAttributes;

class STP_XML_EXPORT XmlContentHandler {
 public:
  virtual void OnStartDocument() = 0;

  virtual void OnEndDocument() = 0;

  virtual void OnStartPrefixMapping(StringSpan prefix, StringSpan uri) = 0;

  virtual void OnEndPrefixMapping(StringSpan prefix) = 0;

  virtual void OnStartElement(
      const QualifiedNameView& qname, StringSpan uri, const XmlAttributes& atts) = 0;

  virtual void OnEndElement(const QualifiedNameView& qname, StringSpan uri) = 0;

  virtual void OnCharacters(StringSpan ch) = 0;

  virtual void OnIgnorableWhitespace(StringSpan ch) = 0;

  virtual void OnProcessingInstruction(StringSpan target, StringSpan data) = 0;

 protected:
  virtual ~XmlContentHandler() {}
};

} // namespace stp

#endif // STP_XML_XMLCONTENTHANDLER_H_
