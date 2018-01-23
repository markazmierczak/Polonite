// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_WIN_CURRENTMODULE_H_
#define STP_BASE_WIN_CURRENTMODULE_H_

#include <winnt.h>

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

// Returns the HMODULE of the dll the macro was expanded in.
// Only use in cpp files, not in h files.
#define CURRENT_MODULE() reinterpret_cast<HMODULE>(&__ImageBase)

#endif // STP_BASE_WIN_CURRENTMODULE_H_
