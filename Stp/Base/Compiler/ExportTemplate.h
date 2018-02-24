// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_COMPILER_EXPORTTEMPLATE_H_
#define STP_BASE_COMPILER_EXPORTTEMPLATE_H_

#define EXPORT_TEMPLATE_DECLARE(EXPORT) \
  EXPORT_TEMPLATE_INVOKE(DECLARE, EXPORT_TEMPLATE_STYLE(EXPORT, ), EXPORT)
#define EXPORT_TEMPLATE_DEFINE(EXPORT) \
  EXPORT_TEMPLATE_INVOKE(DEFINE, EXPORT_TEMPLATE_STYLE(EXPORT, ), EXPORT)

// INVOKE is an internal helper macro to perform parameter replacements
// and token pasting to chain invoke another macro.  E.g.,
//     EXPORT_TEMPLATE_INVOKE(DECLARE, DEFAULT, FOO_EXPORT)
// will export to call
//     EXPORT_TEMPLATE_DECLARE_DEFAULT(FOO_EXPORT, )
// (but with FOO_EXPORT expanded too).
#define EXPORT_TEMPLATE_INVOKE(which, style, EXPORT) \
  EXPORT_TEMPLATE_INVOKE_2(which, style, EXPORT)
#define EXPORT_TEMPLATE_INVOKE_2(which, style, EXPORT) \
  EXPORT_TEMPLATE_##which##_##style(EXPORT, )

// Default style is to apply the FOO_EXPORT macro at declaration sites.
#define EXPORT_TEMPLATE_DECLARE_DEFAULT(EXPORT, _) EXPORT
#define EXPORT_TEMPLATE_DEFINE_DEFAULT(EXPORT, _)

// The "MSVC hack" style is used when FOO_EXPORT is defined
// as __declspec(dllexport), which MSVC requires to be used at
// definition sites instead.
#define EXPORT_TEMPLATE_DECLARE_MSVC_HACK(EXPORT, _)
#define EXPORT_TEMPLATE_DEFINE_MSVC_HACK(EXPORT, _) EXPORT

// EXPORT_TEMPLATE_STYLE is an internal helper macro that identifies which
// export style needs to be used for the provided FOO_EXPORT macro definition.
// "", "__attribute__(...)", and "__declspec(dllimport)" are mapped
// to "DEFAULT"; while "__declspec(dllexport)" is mapped to "MSVC_HACK".
//
// It's implemented with token pasting to transform the __attribute__ and
// __declspec annotations into macro invocations.
#define EXPORT_TEMPLATE_STYLE(EXPORT, _) EXPORT_TEMPLATE_STYLE_2(EXPORT, )
#define EXPORT_TEMPLATE_STYLE_2(EXPORT, _) \
  EXPORT_TEMPLATE_STYLE_3(EXPORT_TEMPLATE_STYLE_MATCH##EXPORT)
#define EXPORT_TEMPLATE_STYLE_3(style) style

// Internal helper macros for EXPORT_TEMPLATE_STYLE.
#define EXPORT_TEMPLATE_STYLE_MATCH DEFAULT
#define EXPORT_TEMPLATE_STYLE_MATCH__attribute__(...) DEFAULT
#define EXPORT_TEMPLATE_STYLE_MATCH__declspec(arg) EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_##arg
#define EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_dllexport MSVC_HACK
#define EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_dllimport DEFAULT

#endif // STP_BASE_COMPILER_EXPORTTEMPLATE_H_
