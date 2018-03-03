// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Compiler/Config.h"

#if SANITIZER(THREAD)

char TsanDefaultSuppressions[] =
"race:TopManager_\n"

"race:SkBaseMutex::acquire\n"

"race:third_party/libjpeg_turbo/simd/jsimd_x86_64.c\n"

;

#endif // SANITIZER(THREAD)
