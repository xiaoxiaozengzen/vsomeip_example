// Copyright (C) 2014-2023 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
#pragma message                                                                \
    "Signal handling is disabled. The application will not be able to stop gracefully."
#include <csignal>
#endif
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include "vsomeip/vsomeip.hpp"

#include "sample_ids.hpp"
#include "type_map.hpp"

/**
 * @brief This class implements a simple VSOMEIP client that sends a request to
 * a VSOMEIP service.
 */
class routing_sample {
public:
  routing_sample()
      : app_(vsomeip::runtime::get()->create_application("routing_example")) {}

  bool init() {
    std::cout << "Routing example Initing!" << std::endl;
    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }
    std::cout << "Routing example Inited!" << std::endl;
    std::cout << "App name: " << app_->get_name() << std::endl;
    return true;
  }

  void start() { app_->start(); }

  void stop() {
    app_->clear_all_handler();
    app_->stop();
  }

private:
  /// the VSOMEIP application
  std::shared_ptr<vsomeip::application> app_;
};

int main(int argc, char **argv) {
  routing_sample its_sample = routing_sample();

  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
