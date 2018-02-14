// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Xml/XmlSaxParser.h"

#include "Base/Text/FormatMany.h"
#include "Xml/XmlContentHandler.h"
#include "Xml/XmlErrorHandler.h"
#include "Xml/XmlNamespaceSupport.h"
#include "Xml/ThirdParty/libxml/src/include/libxml/catalog.h"
#include "Xml/ThirdParty/libxml/src/include/libxml/parser.h"
#include "Xml/ThirdParty/libxml/src/include/libxml/parserInternals.h"

namespace stp {

namespace {

inline StringSpan ToStringSpan(const xmlChar* xml_string) {
  return MakeSpanFromNullTerminated(reinterpret_cast<const char*>(xml_string));
}

inline StringSpan ToStringSpan(const xmlChar* xml_string, int length) {
  return StringSpan(reinterpret_cast<const char*>(xml_string), length);
}

inline XmlSaxParser* ToParser(void* closure) {
  xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(closure);
  return static_cast<XmlSaxParser*>(ctxt->_private);
}

void StartDocumentHandler(void* closure) {
  xmlParserCtxt* ctxt = static_cast<xmlParserCtxt*>(closure);
  XmlSaxParser* parser = ToParser(closure);
  parser->StartDocument(
      ToStringSpan(ctxt->version),
      ToStringSpan(ctxt->encoding),
      ctxt->standalone != 0);
  xmlSAX2StartDocument(closure);
}

void EndDocumentHandler(void* closure) {
  ToParser(closure)->EndDocument();
  xmlSAX2EndDocument(closure);
}

void StartElementHandler(
    void* closure,
    const xmlChar* local_name, const xmlChar* prefix, const xmlChar* uri,
    int nb_namespaces, const xmlChar** namespaces,
    int nb_attributes, int nb_defaulted, const xmlChar** attributes) {
  auto parser = ToParser(closure);

  parser->PushNamespaces(reinterpret_cast<const char**>(namespaces), nb_namespaces);

  parser->StartElement(
      ToStringSpan(local_name), ToStringSpan(prefix), ToStringSpan(uri),
      reinterpret_cast<const char**>(attributes), nb_attributes);
}

void EndElementHandler(
    void* closure,
    const xmlChar* local_name, const xmlChar* prefix, const xmlChar* uri) {
  auto parser = ToParser(closure);

  parser->EndElement(
      ToStringSpan(local_name), ToStringSpan(prefix), ToStringSpan(uri));

  parser->PopNamespaces();
}

void CharactersHandler(void* closure, const xmlChar* chars, int length) {
  ToParser(closure)->Characters(ToStringSpan(chars, length));
}

void IgnorableWhitespaceHandler(void* closure, const xmlChar* chars, int length) {
  ToParser(closure)->IgnorableWhitespace(ToStringSpan(chars, length));
}

void ProcessingInstructionHandler(void* closure, const xmlChar* target, const xmlChar* data) {
  ToParser(closure)->ProcessingInstruction(
      ToStringSpan(target), ToStringSpan(data));
}

XmlParseMessage ToParseMessage(void* closure, const char* message, va_list args) {
  xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(closure);

  XmlParseMessage msg;
  msg.locator.line_number = ctxt->input->line;
  msg.locator.column_number = ctxt->input->col;
  msg.text = StringRawVPrintF(message, args);
  return msg;
}

void WarningHandler(void* closure, const char* message, ...) {
  va_list args;
  va_start(args, message);
  ToParser(closure)->Warning(ToParseMessage(closure, message, args));
  va_end(args);
}

void ErrorHandler(void* closure, const char* message, ...) {
  va_list args;
  va_start(args, message);
  ToParser(closure)->Error(ToParseMessage(closure, message, args));
  va_end(args);
}

void FatalErrorHandler(void* closure, const char* message, ...) {
  va_list args;
  va_start(args, message);
  ToParser(closure)->FatalError(ToParseMessage(closure, message, args));
  va_end(args);
}

class LibXmlNamespaceSupport : public XmlNamespaceSupport {
 public:
  LibXmlNamespaceSupport() {
  }

  void PopContext() override {
    bool update = GetDeclaredPrefixCount() != 0;
    XmlNamespaceSupport::PopContext();
    if (update)
      default_uri_ = XmlNamespaceSupport::GetUri(StringSpan());
  }

  bool DeclarePrefix(StringSpan prefix, StringSpan uri) override {
    if (prefix.IsEmpty())
      default_uri_ = uri;
    return XmlNamespaceSupport::DeclarePrefix(prefix, uri);
  }

  StringSpan GetUri(StringSpan prefix) const override {
    if (prefix.IsEmpty())
      return default_uri_;
    return XmlNamespaceSupport::GetUri(prefix);
  }

  void Reset() override {
    XmlNamespaceSupport::Reset();
    default_uri_ = GetUri(StringSpan());
  }

 private:
  StringSpan default_uri_;
};

} // namespace

XmlSaxAttributes::XmlSaxAttributes(XmlSaxParser* parser)
    : atts_(nullptr), parser_(parser) {}

void XmlSaxAttributes::Reset(const char** atts, int count) {
  ASSERT(count >= 0);

  // As specified in libxml2 documentation |atts| are formed as tuples:
  //   localname/prefix/URI/value/end

  atts_ = atts;
  set_size(count);
}

QualifiedNameView XmlSaxAttributes::GetQName(int index) const {
  ASSERT(index < size());
  const char** tuple = atts_ + index * 5;
  return QualifiedNameView {
    MakeSpanFromNullTerminated(tuple[0]),
    MakeSpanFromNullTerminated(tuple[1]),
  };
}

StringSpan XmlSaxAttributes::GetLocalName(int index) const {
  ASSERT(index < size());
  const char** tuple = atts_ + index * 5;
  return MakeSpanFromNullTerminated(tuple[0]);
}

StringSpan XmlSaxAttributes::GetUri(int index) const {
  ASSERT(index < size());
  const char** tuple = atts_ + index * 5;
  return MakeSpanFromNullTerminated(tuple[2]);
}

StringSpan XmlSaxAttributes::GetValue(int index) const {
  ASSERT(index < size());
  const char** tuple = atts_ + index * 5;
  return StringSpan(tuple[3], tuple[4] - tuple[3]);
}

XmlAttribute XmlSaxAttributes::GetTuple(int index) const {
  ASSERT(index < size());
  const char** tuple = atts_ + index * 5;
  return XmlAttribute {
    MakeSpanFromNullTerminated(tuple[2]),
    MakeSpanFromNullTerminated(tuple[0]),
    StringSpan(tuple[3], tuple[4] - tuple[3]),
  };
}

bool XmlSaxAttributes::GetValue(QualifiedNameView qname, StringSpan* value) const {
  // We can't use qname.prefix here, because multiple prefixes can map to single URI.
  // Resolve the URI first.
  StringSpan uri = parser_->namespace_context()->GetUri(qname.GetPrefix());
  if (uri.IsEmpty() && !qname.GetPrefix().IsEmpty())
    return false;

  return GetValue(uri, qname.GetLocalName(), value);
}

bool XmlSaxAttributes::GetValue(StringSpan uri, StringSpan local_name, StringSpan* value) const {
  int count = size();
  const char** tuple = atts_;
  for (int i = 0; i < count; ++i, tuple += 5) {
    if (local_name == MakeSpanFromNullTerminated(tuple[0]) &&
        uri == MakeSpanFromNullTerminated(tuple[2])) {
      if (value)
        *value = StringSpan(tuple[3], tuple[4] - tuple[3]);
      return true;
    }
  }
  return false;
}

XmlSaxParser::XmlSaxParser()
    : namespace_context_(new LibXmlNamespaceSupport()),
      attributes_(this) {
  xmlSAXHandler sax;
  SetupLibxmlSAXHandler(&sax);

  xmlParserCtxtPtr context = xmlCreatePushParserCtxt(&sax, 0, 0, 0, 0);
  ASSERT(context);

  // Internal initialization
  context->_private = this;
  context->replaceEntities = true;

  libxml_context_ = context;
}

XmlSaxParser::~XmlSaxParser() {
  if (libxml_context_)
    xmlFreeParserCtxt(libxml_context_);
}

void XmlSaxParser::SetContentHandler(XmlContentHandler* handler) {
  content_handler_ = handler;
}

void XmlSaxParser::SetErrorHandler(XmlErrorHandler* handler) {
  error_handler_ = handler;
}

void XmlSaxParser::ParseChunk(StringSpan chunk, bool is_final) {
  if (!libxml_context_) {
    if (error_handler_) {
      XmlParseMessage msg;
      msg.text = "Unable to create context";
      error_handler_->OnFatalError(msg);
    }
    return;
  }

  xmlParseChunk(libxml_context_, chunk.data(), chunk.size(), is_final ? 1 : 0);

  // Release the context.
  if (is_final) {
    if (libxml_context_->myDoc)
      xmlFreeDoc(libxml_context_->myDoc);
  }
}

void XmlSaxParser::StopParsing() {
  if (libxml_context_)
    xmlStopParser(static_cast<xmlParserCtxtPtr>(libxml_context_));
}

void XmlSaxParser::SetupLibxmlSAXHandler(void* handler) {
  xmlSAXHandlerPtr sax = static_cast<xmlSAXHandlerPtr>(handler);

  memset(sax, 0, sizeof(xmlSAXHandler));
  sax->startDocument = StartDocumentHandler;
  sax->endDocument = EndDocumentHandler;
  sax->startElementNs = StartElementHandler;
  sax->endElementNs = EndElementHandler;
  sax->characters = CharactersHandler;
  sax->ignorableWhitespace = IgnorableWhitespaceHandler;
  sax->processingInstruction = ProcessingInstructionHandler;

  sax->cdataBlock = xmlSAX2CDataBlock;
  sax->internalSubset = xmlSAX2InternalSubset;
  sax->externalSubset = xmlSAX2ExternalSubset;
  sax->isStandalone = xmlSAX2IsStandalone;
  sax->hasInternalSubset = xmlSAX2HasInternalSubset;
  sax->hasExternalSubset = xmlSAX2HasExternalSubset;
  sax->resolveEntity = xmlSAX2ResolveEntity;
  sax->getEntity = xmlSAX2GetEntity;
  sax->entityDecl = xmlSAX2EntityDecl;

  sax->warning = WarningHandler;
  sax->error = ErrorHandler;
  sax->fatalError = FatalErrorHandler;

  sax->initialized = XML_SAX2_MAGIC;
}

void XmlSaxParser::StartDocument(StringSpan version, StringSpan encoding, bool is_standalone) {
  if (content_handler_)
    content_handler_->OnStartDocument();
}

void XmlSaxParser::EndDocument() {
  if (content_handler_)
    content_handler_->OnEndDocument();
}

void XmlSaxParser::PushNamespaces(const char** namespaces, int count) {
  namespace_context_->PushContext();

  // namespaces are tuples:
  //   string prefix
  //   string uri

  for (int i = 0; i < count; ++i) {
    namespace_context_->DeclarePrefix(
        MakeSpanFromNullTerminated(namespaces[i * 2 + 0]),
        MakeSpanFromNullTerminated(namespaces[i * 2 + 1]));
  }

  // Get actual number of prefixes (declared prefixes might not be unique).
  count = namespace_context_->GetDeclaredPrefixCount();
  for (int i = 0; i < count; ++i) {
    if (content_handler_) {
      StringSpan prefix = namespace_context_->GetDeclaredPrefixAt(i);
      StringSpan uri = namespace_context_->GetUri(prefix);
      content_handler_->OnStartPrefixMapping(prefix, uri);
    }
  }
}

void XmlSaxParser::PopNamespaces() {
  int count = namespace_context_->GetDeclaredPrefixCount();
  for (int i = 0; i < count; ++i) {
    if (content_handler_) {
      StringSpan prefix = namespace_context_->GetDeclaredPrefixAt(i);
      content_handler_->OnEndPrefixMapping(prefix);
    }
  }

  namespace_context_->PopContext();
}

void XmlSaxParser::StartElement(
    StringSpan local_name, StringSpan prefix, StringSpan uri,
    const char** atts, int atts_count) {
  attributes_.Reset(atts, atts_count);

  if (uri.IsEmpty())
    uri = namespace_context_->GetUri(prefix);

  if (uri.IsEmpty()) {
    StopParsingWithNamespaceError(prefix);
    return;
  }

  if (content_handler_) {
    QualifiedNameView qname(local_name, prefix);
    content_handler_->OnStartElement(qname, uri, attributes_);
  }
}

void XmlSaxParser::EndElement(StringSpan local_name, StringSpan prefix, StringSpan uri) {
  if (content_handler_) {
    QualifiedNameView qname(local_name, prefix);
    content_handler_->OnEndElement(qname, uri);
  }
}

void XmlSaxParser::Characters(StringSpan text) {
  if (content_handler_)
    content_handler_->OnCharacters(text);
}

void XmlSaxParser::IgnorableWhitespace(StringSpan text) {
  if (content_handler_)
    content_handler_->OnIgnorableWhitespace(text);
}

void XmlSaxParser::ProcessingInstruction(StringSpan target, StringSpan data) {
  if (content_handler_)
    content_handler_->OnProcessingInstruction(target, data);
}

void XmlSaxParser::Warning(const XmlParseMessage& message) {
  if (error_handler_)
    error_handler_->OnWarning(message);
}

void XmlSaxParser::Error(const XmlParseMessage& message) {
  if (error_handler_)
    error_handler_->OnError(message);
}

void XmlSaxParser::FatalError(const XmlParseMessage& message) {
  if (error_handler_)
    error_handler_->OnFatalError(message);
}

void XmlSaxParser::StopParsingWithNamespaceError(StringSpan prefix) {
  StopParsing();

  xmlParserCtxt* ctxt = static_cast<xmlParserCtxt*>(libxml_context_);

  auto msg_text = String::Format("Namespace of prefix '{}' not found", prefix);
  XmlParseMessage msg;
  msg.text = msg_text;
  msg.locator.line_number = ctxt->input->line;
  msg.locator.column_number = ctxt->input->col;
  FatalError(msg);
}

} // namespace stp
