// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/SysInfo.h"

#import <UIKit/UIKit.h>

namespace stp {

String SysInfo::OsName() {
  static dispatch_once_t get_system_name_once;
  static String* system_name;
  dispatch_once(&get_system_name_once, ^{
      darwin::ScopedNSAutoreleasePool pool;
      system_name = new String(
          SysNSStringToUtf8([[UIDevice currentDevice] systemName]));
  });
  // Examples of returned value: 'iPhone OS' on iPad 5.1.1 and iPhone 5.1.1.
  return *system_name;
}

} // namespace stp
