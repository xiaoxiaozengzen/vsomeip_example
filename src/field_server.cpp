// Copyright (C) 2014-2023 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
#include <csignal>
#endif
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include <vsomeip/vsomeip.hpp>

#include "sample_ids.hpp"
#include "type_map.hpp"

class field_server_example {
public:
  field_server_example()
      : app_(vsomeip::runtime::get()->create_application(
            "field_server_example")),
        is_registered_(false), cycle_(1000), blocked_(false), running_(true),
        is_offered_(false),
        offer_thread_(std::bind(&field_server_example::run, this)),
        notify_thread_(std::bind(&field_server_example::notify, this)) {}

  bool init() {
    std::lock_guard<std::mutex> its_lock(mutex_);

    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }
    app_->register_state_handler(std::bind(&field_server_example::on_state,
                                           this, std::placeholders::_1));

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(FieldClient_EVENTGROUP_ID);

    app_->offer_event(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                      FieldClient_EVENT_ID, its_groups,
                      vsomeip::event_type_e::ET_FIELD,
                      std::chrono::milliseconds::zero(), false, true, nullptr,
                      vsomeip::reliability_type_e::RT_RELIABLE);
    {
      std::lock_guard<std::mutex> its_lock(payload_mutex_);
      payload_ = vsomeip::runtime::get()->create_payload();
    }

    blocked_ = true;
    condition_.notify_one();
    return true;
  }

  void start() { app_->start(); }

  void stop() {
    running_ = false;
    blocked_ = true;
    condition_.notify_one();
    notify_condition_.notify_one();
    app_->clear_all_handler();
    stop_offer();
    if (std::this_thread::get_id() != offer_thread_.get_id()) {
      if (offer_thread_.joinable()) {
        offer_thread_.join();
      }
    } else {
      offer_thread_.detach();
    }
    if (std::this_thread::get_id() != notify_thread_.get_id()) {
      if (notify_thread_.joinable()) {
        notify_thread_.join();
      }
    } else {
      notify_thread_.detach();
    }
    app_->stop();
  }

  void offer() {
    std::lock_guard<std::mutex> its_lock(notify_mutex_);
    app_->offer_service(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID);
    is_offered_ = true;
    notify_condition_.notify_one();
  }

  void stop_offer() {
    app_->stop_offer_service(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID);
    is_offered_ = false;
  }

  void on_state(vsomeip::state_type_e _state) {
    std::cout << "Application " << app_->get_name() << " is "
              << (_state == vsomeip::state_type_e::ST_REGISTERED
                      ? "registered."
                      : "deregistered.")
              << std::endl;

    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
      if (!is_registered_) {
        is_registered_ = true;
      }
    } else {
      is_registered_ = false;
    }
  }

  void run() {
    std::unique_lock<std::mutex> its_lock(mutex_);
    while (!blocked_)
      condition_.wait(its_lock);

    bool is_offer(true);

    if (is_offer) {
      offer();
    }
  }

  void notify() {
    vsomeip::byte_t its_data1[10] = {0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00};
    vsomeip::byte_t its_data2[5] = {0x11, 0x11, 0x11, 0x11, 0x11};
    std::uint32_t count(0);
    bool its_data1_flag(true);

    while (running_) {
      std::unique_lock<std::mutex> its_lock(notify_mutex_);
      while (!is_offered_ && running_)
        notify_condition_.wait(its_lock);
      while (is_offered_ && running_) {
        {
          std::lock_guard<std::mutex> its_lock(payload_mutex_);

          if (its_data1_flag) {
            payload_->set_data(its_data1, 10);
            std::cout << "Notify: num " << count << " times"
                      << ", with payload: 10 bytes" << std::endl;
          } else {
            payload_->set_data(its_data2, 5);
            std::cout << "Notify: num " << count << " times"
                      << ", with payload: 5 bytes" << std::endl;
          }

          app_->notify(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                       FieldClient_EVENT_ID, payload_);
        }

        if (count % 5 == 0) {
          if (its_data1_flag) {
            its_data1_flag = false;
          } else {
            its_data1_flag = true;
          }
        }

        count++;

        std::this_thread::sleep_for(std::chrono::milliseconds(cycle_));
      }
    }
  }

private:
  std::shared_ptr<vsomeip::application> app_;
  bool is_registered_;
  uint32_t cycle_;

  std::mutex mutex_;
  std::condition_variable condition_;
  bool blocked_;
  bool running_;

  std::mutex notify_mutex_;
  std::condition_variable notify_condition_;
  bool is_offered_;

  std::mutex payload_mutex_;
  std::shared_ptr<vsomeip::payload> payload_;

  // blocked_ / is_offered_ must be initialized before starting the threads!
  std::thread offer_thread_;
  std::thread notify_thread_;
};

int main(int argc, char **argv) {
  field_server_example its_sample;
  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
