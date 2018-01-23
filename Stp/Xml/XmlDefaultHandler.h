// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLDEFAULTHANDLER_H_
#define STP_XML_XMLDEFAULTHANDLER_H_

#include "Xml/XmlContentHandler.h"
#include "Xml/XmlErrorHandler.h"

namespace stp {

class STP_XML_EXPORT XmlDefaultHandler
    : public XmlContentHandler,
      public XmlErrorHandler {
 public:
  // XMLContentHandler:
  void OnStartDocument() override;
  void OnEndDocument() override;
  void OnStartPrefixMapping(StringSpan prefix, StringSpan uri) override;
  void OnEndPrefixMapping(StringSpan prefix) override;
  void OnStartElement(const QualifiedNameView& qname, StringSpan uri, const XmlAttributes& atts) override;
  void OnEndElement(const QualifiedNameView& qname, StringSpan uri) override;
  void OnCharacters(StringSpan ch) override;
  void OnIgnorableWhitespace(StringSpan ch) override;
  void OnProcessingInstruction(StringSpan target, StringSpan data) override;

  // XMLErrorHandler:
  void OnWarning(const XmlParseMessage& warning) override;
  void OnError(const XmlParseMessage& error) override;
  void OnFatalError(const XmlParseMessage& error) override;
};

} // namespace stp

#endif // STP_XML_XMLDEFAULTHANDLER_H_
