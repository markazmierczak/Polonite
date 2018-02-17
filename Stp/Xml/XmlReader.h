// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLREADER_H_
#define STP_XML_XMLREADER_H_

#include "Base/Containers/Span.h"
#include "Xml/Export.h"

namespace stp {

class XmlContentHandler;
class XmlErrorHandler;

class STP_XML_EXPORT XmlReader {
 public:
  virtual ~XmlReader() {}

  // Allow an application to register a content event handler.
  virtual void SetContentHandler(XmlContentHandler* handler) = 0;

  // Allow an application to register an error event handler.
  virtual void SetErrorHandler(XmlErrorHandler* handler) = 0;

  virtual void ParseChunk(StringSpan chunk, bool is_final) = 0;

  // Parse an XML document.
  void Parse(StringSpan content);

  virtual void StopParsing() = 0;
};

inline void XmlReader::Parse(StringSpan content) {
  ParseChunk(content, false);
  ParseChunk(StringSpan(), true);
}

} // namespace stp

#endif // STP_XML_XMLREADER_H_
