// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/TextEncodingRegistry.h"

#include "Base/Containers/FlatMap.h"
#include "Base/Thread/Lock.h"

namespace stp {

namespace {

static const TextCodec* const BuiltinCodecs[] = {
  // Sort this array by frequency of usage.
  &detail::Utf8Codec,
  &detail::Utf16Codec,
  &detail::Utf16BECodec,
  &detail::Utf16LECodec,
  &detail::Utf32Codec,
  &detail::Utf32BECodec,
  &detail::Utf32LECodec,
  &detail::AsciiCodec,
  &detail::Cp1252Codec,
  &detail::Latin1Codec,
  &detail::Latin2Codec,
  &detail::Latin3Codec,
  &detail::Latin4Codec,
  nullptr
};

class BuiltinTextEncodingProvider : public TextEncodingProvider {
 public:
  TextEncoding TryResolveByName(StringSpan name) {
    for (auto iter = BuiltinCodecs; *iter; ++iter) {
      TextEncoding encoding(*iter);
      if (TextEncoding::AreNamesMatching(encoding.GetName(), name))
        return encoding;
    }
    return TextEncoding();
  }
};

class TextEncodingRegistry {
 public:
  TextEncodingRegistry() {
    AddProvider(&builtin_provider_);
  }

  TextEncoding FindByName(StringSpan name) {
    auto find_result = by_name_.find(name);
    if (find_result)
      return find_result.get();

    TextEncoding encoding = Resolve([&name](TextEncodingProvider* provider) {
      return provider->TryResolveByName(name);
    });
    if (encoding.IsValid())
      find_result.add(name, encoding);
    return encoding;
  }

  template<typename TFunctor>
  TextEncoding Resolve(TFunctor&& functor) {
    LinkedListIterator<TextEncodingProvider> iter(&providers_);
    for (; iter.IsValid(); iter.MoveNext()) {
      TextEncoding encoding = functor(iter.get());
      if (encoding.IsValid())
        return encoding;
    }
    return TextEncoding();
  }

  void AddProvider(TextEncodingProvider* provider) {
    providers_.append(provider);
  }

 private:
  LinkedList<TextEncodingProvider> providers_;
  FlatMap<String, TextEncoding> by_name_;
  BuiltinTextEncodingProvider builtin_provider_;
};

BasicLock g_registry_lock = BASIC_LOCK_INITIALIZER;
TextEncodingRegistry* g_registry = nullptr;

TextEncodingRegistry* GetRegistry() {
  if (!g_registry)
    g_registry = new TextEncodingRegistry();
  return g_registry;
}

} // namespace

void InstallTextEncodingProvider(TextEncodingProvider* provider) {
  AutoLock guard(&g_registry_lock);
  GetRegistry()->AddProvider(provider);
}

TextEncoding FindTextEncodingByName(StringSpan name) {
  AutoLock guard(&g_registry_lock);
  return GetRegistry()->FindByName(name);
}

} // namespace stp
