// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_EXPORT_H_
#define STP_BASE_EXPORT_H_

#ifdef COMPONENT_BUILD
#ifdef _MSC_VER

#ifdef STP_BASE_IMPLEMENTATION
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif

#else
#ifdef STP_BASE_IMPLEMENTATION
#define BASE_EXPORT __attribute__((visibility("default")))
#else
#define BASE_EXPORT
#endif
#endif

#else
#define BASE_EXPORT
#endif // COMPONENT_BUILD

#endif // STP_BASE_EXPORT_H_
