// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Compiler/Sanitizer.h"

#if SANITIZER(LEAK)

char LSanDefaultSuppressions[] =

// ================ Leaks in third-party code ================

// False positives in libfontconfig.
"leak:libfontconfig\n"

// Leaks in Nvidia's libGL.
"leak:libGL.so\n"

// XRandR has several one time leaks.
"leak:libxrandr\n"

// xrandr leak.
"leak:XRRFindDisplay\n"

// Leaks in swrast_dri.so
"leak:swrast_dri.so\n"

;

#endif // SANITIZER(LEAK)
