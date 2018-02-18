// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_APP_BASEAPPLICATIONPART_H_
#define STP_BASE_APP_BASEAPPLICATIONPART_H_

#include "Base/App/ApplicationPart.h"

namespace stp {

class BASE_EXPORT BaseApplicationPart {
 public:
  static ApplicationPart Instance;

 private:
  static const ApplicationPartInfo Info_;

  static void init();
  static void fini();

  static constexpr ApplicationPartInfo makeInfo();
};

constexpr ApplicationPartInfo BaseApplicationPart::makeInfo() {
  ApplicationPartInfo p("Stp/Base");
  p.init = &init;
  p.fini = &fini;
  return p;
}

} // namespace stp

#endif // STP_BASE_APP_BASEAPPLICATIONPART_H_
