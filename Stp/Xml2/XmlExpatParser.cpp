// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Xml/XmlExpatParser.h"

#include "Xml/XmlConstants.h"
#include "Xml/XmlContentHandler.h"
#include "Xml/XmlErrorHandler.h"
#include "Xml/XmlNamespaceSupport.h"

#include <expat.h>

#ifdef XML_NS
# error "designed to use without namespace support"
#endif

namespace stp {

namespace {

void XmlDeclHandler(void* closure, const char* version, const char* encoding, int is_standalone) {
  auto parser = static_cast<XmlExpatParser*>(closure);
  parser->ExpatStartDocument(
      MakeSpanFromNullTerminated(version),
      MakeSpanFromNullTerminated(encoding),
      is_standalone);
}

void StartElementHandler(void* closure, const char* name, const char** atts) {
  static_cast<XmlExpatParser*>(closure)->ExpatStartElement(
      MakeSpanFromNullTerminated(name), atts);
}

void EndElementHandler(void* closure, const char* name) {
  static_cast<XmlExpatParser*>(closure)->ExpatEndElement(
      MakeSpanFromNullTerminated(name));
}

void CharacterDataHandler(void* closure, const char* chars, int length) {
  static_cast<XmlExpatParser*>(closure)->ExpatCharacters(StringSpan(chars, length));
}

} // namespace

void XmlExpatAttributes::Reset(const char** array) {
  // Count the number of attributes.
  int count = 0;
  for (const char** iter = array; *iter; iter += 2)
    count++;

  array_ = array;
  set_size(count);
}

QualifiedNameView XmlExpatAttributes::GetQName(int index) const {
  return QualifiedNameView(GetLocalName(index), StringSpan());
}

StringSpan XmlExpatAttributes::GetLocalName(int index) const {
  ASSERT(index < size());
  return MakeSpanFromNullTerminated(array_[index * 2 + 0]);
}

StringSpan XmlExpatAttributes::GetUri(int index) const {
  ASSERT(index < size());
  return StringSpan();
}

StringSpan XmlExpatAttributes::GetValue(int index) const {
  ASSERT(index < size());
  return MakeSpanFromNullTerminated(array_[index * 2 + 1]);
}

XmlAttribute XmlExpatAttributes::GetTuple(int index) const {
  ASSERT(index < size());
  const char** tuple = array_ + index * 2;
  return XmlAttribute {
    StringSpan(),
    MakeSpanFromNullTerminated(tuple[0]),
    MakeSpanFromNullTerminated(tuple[1])
  };
}

bool XmlExpatAttributes::GetValue(QualifiedNameView qname, StringSpan* value) const {
  return GetValue(StringSpan(), qname.GetLocalName(), value);
}

bool XmlExpatAttributes::GetValue(StringSpan uri, StringSpan local_name, StringSpan* value) const {
  for (const char** iter = array_; *iter; iter += 2) {
    if (local_name == MakeSpanFromNullTerminated(iter[0])) {
      if (value)
        *value = MakeSpanFromNullTerminated(iter[1]);
      return true;
    }
  }
  return false;
}

XmlExpatParser::XmlExpatParser()
    : error_(XML_ERROR_NONE) {
  expat_ = XML_ParserCreate(NULL);
  XML_SetUserData(expat_, this);
  XML_SetXmlDeclHandler(expat_, XmlDeclHandler);
  XML_SetElementHandler(expat_, StartElementHandler, EndElementHandler);
  XML_SetCharacterDataHandler(expat_, CharacterDataHandler);
}

XmlExpatParser::~XmlExpatParser() {
  XML_ParserFree(expat_);
}

void XmlExpatParser::SetContentHandler(XmlContentHandler* handler) {
  content_handler_ = handler;
}

void XmlExpatParser::SetErrorHandler(XmlErrorHandler* handler) {
  error_handler_ = handler;
}

void XmlExpatParser::ParseChunk(StringSpan chunk, bool is_final) {
  if (has_error())
    return;

  if (XML_Parse(expat_, chunk.data(), chunk.size(), is_final) != XML_STATUS_OK)
    RaiseError(XML_GetErrorCode(expat_));

  if (is_final) {
    if (!has_error()) {
      if (content_handler_)
        content_handler_->OnEndDocument();
    }
  }
}

void XmlExpatParser::StopParsing() {
  XML_StopParser(expat_, false);
}

void XmlExpatParser::ExpatStartDocument(
    StringSpan version, StringSpan encoding, int is_standalone) {
  if (error_ != XML_ERROR_NONE)
    return;

  if (version != "1.0" || !is_standalone) {
    // expat supports 1.0 version only.
    RaiseError(XML_ERROR_SYNTAX);
    return;
  }

  if (!encoding.IsEmpty() && encoding != "UTF-" && encoding != "utf8") {
    RaiseError(XML_ERROR_INCORRECT_ENCODING);
    return;
  }

  if (content_handler_)
    content_handler_->OnStartDocument();
}

void XmlExpatParser::ExpatStartElement(StringSpan name, const char** attributes) {
  if (has_error())
    return;

  attributes_.Reset(attributes);

  if (content_handler_) {
    QualifiedNameView qname(name, StringSpan());
    content_handler_->OnStartElement(qname, StringSpan(), attributes_);
  }
}

void XmlExpatParser::ExpatEndElement(StringSpan name) {
  if (has_error())
    return;

  if (content_handler_) {
    QualifiedNameView qname(name, StringSpan());
    content_handler_->OnEndElement(qname, StringSpan());
  }
}

void XmlExpatParser::ExpatCharacters(StringSpan text) {
  if (has_error())
    return;

  if (content_handler_)
    content_handler_->OnCharacters(text);
}

void XmlExpatParser::RaiseError(int code) {
  if (has_error())
    return;

  StopParsing();
  error_ = code;

  if (error_handler_) {
    XmlParseMessage msg;
    msg.text = MakeSpanFromNullTerminated(XML_ErrorString(static_cast<XML_Error>(error_)));
    msg.locator.line_number = GetCurrentLineNumber();
    msg.locator.column_number = GetCurrentColumnNumber();
    error_handler_->OnFatalError(msg);
  }
}

int XmlExpatParser::GetCurrentLineNumber() const {
  return static_cast<int>(XML_GetCurrentLineNumber(expat_));
}

int XmlExpatParser::GetCurrentColumnNumber() const {
  return static_cast<int>(XML_GetCurrentColumnNumber(expat_));
}

int XmlExpatParser::GetCurrentByteIndex() const {
  return static_cast<int>(XML_GetCurrentByteIndex(expat_));
}

int XmlExpatParser::GetEventByteCount() const {
  return static_cast<int>(XML_GetCurrentByteCount(expat_));
}

bool XmlExpatParser::has_error() const {
  return error_ != XML_ERROR_NONE;
}

} // namespace stp
