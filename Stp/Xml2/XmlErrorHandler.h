// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_XMLERRORHANDLER_H_
#define STP_XML_XMLERRORHANDLER_H_

#include "Base/Text/StringSpan.h"
#include "Xml/Export.h"

namespace stp {

struct STP_XML_EXPORT XmlLocator {
  int line_number;
  int column_number;
};

struct STP_XML_EXPORT XmlParseMessage {
  StringSpan text;
  XmlLocator locator;
};

class STP_XML_EXPORT XmlErrorHandler {
 public:
  // Receive notification of a warning.
  virtual void OnWarning(const XmlParseMessage& warning) = 0;

  // Receive notification of a recoverable error.
  virtual void OnError(const XmlParseMessage& error) = 0;

  // Receive notification of a non-recoverable error.
  virtual void OnFatalError(const XmlParseMessage& error) = 0;

 protected:
  virtual ~XmlErrorHandler() {}
};

} // namespace stp

#endif // STP_XML_XMLERRORHANDLER_H_
