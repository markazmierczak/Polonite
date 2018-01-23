// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_XML_EXPORT_H_
#define STP_XML_EXPORT_H_

#ifdef COMPONENT_BUILD
#ifdef _MSC_VER

#ifdef STP_XML_IMPLEMENTATION
#define STP_XML_EXPORT __declspec(dllexport)
#else
#define STP_XML_EXPORT __declspec(dllimport)
#endif

#else
#ifdef STP_XML_IMPLEMENTATION
#define STP_XML_EXPORT __attribute__((visibility("default")))
#else
#define STP_XML_EXPORT
#endif
#endif

#else
#define STP_XML_EXPORT
#endif // COMPONENT_BUILD

#endif // STP_XML_EXPORT_H_
