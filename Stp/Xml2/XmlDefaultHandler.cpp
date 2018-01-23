// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Xml/XmlDefaultHandler.h"

namespace stp {

void XmlDefaultHandler::OnStartDocument() {}
void XmlDefaultHandler::OnEndDocument() {}

void XmlDefaultHandler::OnStartPrefixMapping(StringSpan prefix, StringSpan uri) {}
void XmlDefaultHandler::OnEndPrefixMapping(StringSpan prefix) {}

void XmlDefaultHandler::OnStartElement(const QualifiedNameView& qname, StringSpan uri, const XmlAttributes& atts) {}
void XmlDefaultHandler::OnEndElement(const QualifiedNameView& qname, StringSpan uri) {}

void XmlDefaultHandler::OnCharacters(StringSpan ch) {}

void XmlDefaultHandler::OnIgnorableWhitespace(StringSpan ch) {
  OnCharacters(ch);
}

void XmlDefaultHandler::OnProcessingInstruction(StringSpan target, StringSpan data) {}

void XmlDefaultHandler::OnWarning(const XmlParseMessage& warning) {}
void XmlDefaultHandler::OnError(const XmlParseMessage& error) {}
void XmlDefaultHandler::OnFatalError(const XmlParseMessage& error) {}

} // namespace stp
