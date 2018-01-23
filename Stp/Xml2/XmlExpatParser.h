// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLEXPATPARSER_H_
#define STP_XML_XMLEXPATPARSER_H_

#include "Xml/XmlAttributes.h"
#include "Xml/XmlReader.h"

// Forward expat definitions.
struct XML_ParserStruct;
typedef struct XML_ParserStruct* XML_Parser;

namespace stp {

class XmlContentHandler;
class XmlErrorHandler;
class XmlNamespaceContext;

class STP_XML_EXPORT XmlExpatAttributes : public XmlAttributes {
 public:
  XmlExpatAttributes() : array_(nullptr) {}

  void Reset(const char** array);

  QualifiedNameView GetQName(int index) const override;
  StringSpan GetLocalName(int index) const override;
  StringSpan GetUri(int index) const override;
  StringSpan GetValue(int index) const override;
  XmlAttribute GetTuple(int index) const override;
  bool GetValue(QualifiedNameView qname, StringSpan* value) const override;
  bool GetValue(StringSpan uri, StringSpan local_name, StringSpan* value) const override;

 private:
  const char** array_;
};

// Use expat when you need an easy way to get current position in bytes.
// Does NOT support namespaces, use XmlSaxReader if you need them.
class STP_XML_EXPORT XmlExpatParser : public XmlReader {
 public:
  XmlExpatParser();
  ~XmlExpatParser() override;

  void SetContentHandler(XmlContentHandler* handler) override;
  void SetErrorHandler(XmlErrorHandler* handler) override;
  void ParseChunk(StringSpan chunk, bool is_final) override;
  void StopParsing() override;

  int GetCurrentLineNumber() const;
  int GetCurrentColumnNumber() const;
  int GetCurrentByteIndex() const;
  int GetEventByteCount() const;

  // Callbacks needed by expat library.
  void ExpatStartDocument(StringSpan version, StringSpan encoding, int is_standalone);
  void ExpatStartElement(StringSpan name, const char** attributes);
  void ExpatEndElement(StringSpan name);
  void ExpatCharacters(StringSpan text);

 private:
  void RaiseError(int code);
  bool has_error() const;

  XML_Parser expat_;

  XmlContentHandler* content_handler_ = nullptr;
  XmlErrorHandler* error_handler_ = nullptr;

  XmlExpatAttributes attributes_;

  String default_uri_;

  int error_;
};

} // namespace stp

#endif // STP_XML_XMLEXPATPARSER_H_
