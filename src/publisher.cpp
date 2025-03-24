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

/**
 * @brief This class represents a publisher example.
 */
class publisher_example {
public:
  /**
   * @brief Construct the publisher example.
   * @param _cycle Cycle time.
   */
  publisher_example(uint32_t _cycle)
      : app_(vsomeip::runtime::get()->create_application("publisher_example")),
        is_registered_(false), cycle_(_cycle), blocked_(false), running_(true),
        is_offered_(false),
        offer_thread_(std::bind(&publisher_example::run, this)),
        notify_thread_(std::bind(&publisher_example::notify, this)) {}

  /**
   * @brief Initialize the publisher example.
   * @return True if the publisher example is initialized successfully.
   */
  bool init() {
    std::lock_guard<std::mutex> its_lock(mutex_);

    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }
    app_->register_state_handler(
        std::bind(&publisher_example::on_state, this, std::placeholders::_1));

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(PublishSubscribe_EVENTGROUP_ID);

    /**
     * @brief Offer the event.
     * @note
     * app需要调用改接口将event/field注册到vsomeip中，其他app才能订阅到该event/filed
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _notifier 事件ID
     * @param _eventgroups 事件组集合，每个事件组都包含当前事件
     * @param _type 事件类型，默认为ET_EVENT
     * @param _cycle 周期，单位ms，默认为0
     * @param _change_resets_cycle
     * 如果为true，则当event的payload发生变化时，会reset用于周期发布事件的cycle
     * time。默认为false
     * @param _update_on_change
     * 如果为false，则当event发生变化时，不会发布事件。默认为true
     * @param _epsilon_change_func
     * fun对象，用于判断两个event的payload是否相等，默认为nullptr
     * @param _reliability
     * 可靠性类型，默认为RT_UNKNOWN。设计上不是RT_UNRELIABLE，就是RT_RELIABLE。并且同一个eventgroup中的event需要可靠性类型一致
     */
    app_->offer_event(PublishSubscribe_SERVICE_ID, PublishSubscribe_INSTANCE_ID,
                      PublishSubscribe_EVENT_ID, its_groups,
                      vsomeip::event_type_e::ET_EVENT,
                      std::chrono::milliseconds::zero(), false, true, nullptr,
                      vsomeip::reliability_type_e::RT_UNKNOWN);
    {
      std::lock_guard<std::mutex> its_lock(payload_mutex_);
      payload_ = vsomeip::runtime::get()->create_payload();
    }

    blocked_ = true;
    condition_.notify_one();
    return true;
  }

  /**
   * @brief Start the publisher example.
   */
  void start() { app_->start(); }

  /**
   * @brief Stop the publisher example.
   */
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

  /**
   * @brief Offer the service.
   */
  void offer() {
    std::lock_guard<std::mutex> its_lock(notify_mutex_);
    app_->offer_service(PublishSubscribe_SERVICE_ID,
                        PublishSubscribe_INSTANCE_ID);
    is_offered_ = true;
    notify_condition_.notify_one();
  }

  /**
   * @brief Stop offering the service.
   */
  void stop_offer() {
    app_->stop_offer_service(PublishSubscribe_SERVICE_ID,
                             PublishSubscribe_INSTANCE_ID);
    is_offered_ = false;
  }

  /**
   * @brief Handle the state.
   */
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

  /**
   * @brief Run the publisher example.
   */
  void run() {
    std::unique_lock<std::mutex> its_lock(mutex_);
    while (!blocked_)
      condition_.wait(its_lock);

    bool is_offer(true);

    if (is_offer) {
      offer();
    }
  }

  /**
   * @brief Notify the event.
   */
  void notify() {

    vsomeip::byte_t its_data[10];
    uint32_t its_size = 1;

    while (running_) {
      std::unique_lock<std::mutex> its_lock(notify_mutex_);
      while (!is_offered_ && running_)
        notify_condition_.wait(its_lock);
      while (is_offered_ && running_) {
        if (its_size == sizeof(its_data))
          its_size = 1;

        for (uint32_t i = 0; i < its_size; ++i)
          its_data[i] = static_cast<uint8_t>(i);

        {
          std::lock_guard<std::mutex> its_lock(payload_mutex_);
          payload_->set_data(its_data, its_size);

          std::cout << "Setting event (Length=" << std::dec << its_size << ")."
                    << std::endl;

          /**
           * @brief Notify the event.
           * @note
           * 特定的event通过特定的payload来传递数据。根据不通的事件类型，将payload分发给相关的订阅者(event一直会发送，filed只会在payload发生变化时才会发送)。
           * @note 在使用该接口之前，需要先调用offer_event接口
           *
           * @param _service 服务ID
           * @param _instance 实例ID
           * @param _event 事件ID
           * @param _payload vsemeip::payload对象，包含了需要传递的数据
           * @param _force 是否强制发送，默认为false
           */
          app_->notify(PublishSubscribe_SERVICE_ID,
                       PublishSubscribe_INSTANCE_ID, PublishSubscribe_EVENT_ID,
                       payload_);
        }

        its_size++;

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
  uint32_t cycle = 1000; // default 1s

  std::string cycle_arg("--cycle");

  for (int i = 1; i < argc; i++) {
    if (cycle_arg == argv[i] && i + 1 < argc) {
      i++;
      std::stringstream converter;
      converter << argv[i];
      converter >> cycle;
    }
  }

  publisher_example its_sample(cycle);
  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
