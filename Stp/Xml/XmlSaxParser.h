// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLSAXPARSER_H_
#define STP_XML_XMLSAXPARSER_H_

#include "Base/Memory/OwnPtr.h"
#include "Xml/XmlAttributes.h"
#include "Xml/XmlReader.h"

typedef struct _xmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt *xmlParserCtxtPtr;

namespace stp {

struct XmlParseMessage;

class XmlContentHandler;
class XmlErrorHandler;
class XmlNamespaceContext;
class XmlSaxParser;

class XmlSaxAttributes : public XmlAttributes {
 public:
  explicit XmlSaxAttributes(XmlSaxParser* parser);

  void Reset(const char** atts, int count);

  QualifiedNameView GetQName(int index) const override;
  StringSpan GetLocalName(int index) const override;
  StringSpan GetUri(int index) const override;
  StringSpan GetValue(int index) const override;
  XmlAttribute GetTuple(int index) const override;
  bool GetValue(QualifiedNameView qname, StringSpan* value) const override;
  bool GetValue(StringSpan uri, StringSpan local_name, StringSpan* value) const override;

 private:
  const char** atts_;
  XmlSaxParser* parser_;
};

class STP_XML_EXPORT XmlSaxParser : public XmlReader {
 public:
  XmlSaxParser();
  ~XmlSaxParser() override;

  // XmlReader:
  void SetContentHandler(XmlContentHandler* handler) override;
  void SetErrorHandler(XmlErrorHandler* handler) override;
  void ParseChunk(StringSpan chunk, bool is_final) override;
  void StopParsing() override;

  XmlNamespaceContext* namespace_context() const { return namespace_context_; }

  // Internal implementation of SAX parser visible in public due C shims
  // to libxml library.
  void StartDocument(StringSpan version, StringSpan encoding, bool is_standalone);
  void EndDocument();

  void PushNamespaces(const char** namespaces, int count);
  void PopNamespaces();

  void StartElement(
      StringSpan local_name, StringSpan prefix, StringSpan uri,
      const char** attributes, int atts_count);
  void EndElement(StringSpan local_name, StringSpan prefix, StringSpan uri);

  void Characters(StringSpan text);
  void IgnorableWhitespace(StringSpan text);
  void ProcessingInstruction(StringSpan target, StringSpan data);

  void Warning(const XmlParseMessage& message);
  void Error(const XmlParseMessage& message);
  void FatalError(const XmlParseMessage& message);

 protected:
  void SetupLibxmlSAXHandler(void* handler);

  xmlParserCtxtPtr libxml_context_;

 private:
  void StopParsingWithNamespaceError(StringSpan prefix);

  XmlNamespaceContext* namespace_context_;

  XmlSaxAttributes attributes_;

  XmlContentHandler* content_handler_ = nullptr;
  XmlErrorHandler* error_handler_ = nullptr;
};

} // namespace stp

#endif // STP_XML_XMLSAXPARSER_H_
