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
#include <sstream>
#include <thread>

#include <vsomeip/vsomeip.hpp>

#include "sample_ids.hpp"

/**
 * @brief This class represents a response example.
 */
class response_example {
public:
  response_example(bool _use_static_routing, std::string path = "")
      : app_(vsomeip::runtime::get()->create_application("response_example")),
        is_registered_(false), use_static_routing_(_use_static_routing),
        blocked_(false), running_(true),
        offer_thread_(std::bind(&response_example::run, this)) {}

  /**
   *  @brief Initialize the response example.
   */
  bool init() {
    std::lock_guard<std::mutex> its_lock(mutex_);

    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }
    app_->register_state_handler(
        std::bind(&response_example::on_state, this, std::placeholders::_1));
    app_->register_message_handler(
        RequestResponse_SERVICE_ID, RequestResponse_INSTANCE_ID,
        RequestResponse_METHOD_ID,
        std::bind(&response_example::on_message, this, std::placeholders::_1));

    return true;
  }

  /**
   * @brief Start the response example.
   */
  void start() { app_->start(); }

  /**
   * @brief Stop the response example.
   */
  void stop() {
    running_ = false;
    blocked_ = true;
    app_->clear_all_handler();
    stop_offer();
    condition_.notify_one();
    if (std::this_thread::get_id() != offer_thread_.get_id()) {
      if (offer_thread_.joinable()) {
        offer_thread_.join();
      }
    } else {
      offer_thread_.detach();
    }
    app_->stop();
  }

  /**
   * @brief Offer the service.
   */
  void offer() {
    /**
     * @brief 提供一个service instance
     * @note app必须调用该函数，从而将特定的service instance注册到vsomeip
     * routing中，从而客户端可以找到该service instance
     * @note
     * 可以通过配置，从而决定是本地还是跨机器提供服务。如果是跨机器通信，则需要给当前服务示例提供port，否则其他机器无法找到该服务实例
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _major 服务的主版本号，默认为DEFAULT_MAJOR
     * @param _minor 服务的次版本号，默认为DEFAULT_MINOR
     */
    app_->offer_service(RequestResponse_SERVICE_ID,
                        RequestResponse_INSTANCE_ID);
  }

  /**
   * @brief Stop offering the service.
   */
  void stop_offer() {
    /**
     * @brief 停止提供service instance
     *
     * @note 调用该函数撤回当前service
     * instance的注册，从而其他客户端无法找到该service instance
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _major 服务的主版本号，默认为DEFAULT_MAJOR
     * @param _minor 服务的次版本号，默认为DEFAULT_MINOR
     */
    app_->stop_offer_service(RequestResponse_SERVICE_ID,
                             RequestResponse_INSTANCE_ID);
  }

  /**
   * @brief Callback function for the state of the application.
   * @param _state State of the application.
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
        blocked_ = true;
        condition_.notify_one();
      }
    } else {
      is_registered_ = false;
    }
  }

  /**
   * @brief Callback function for the message.
   * @param _request Request message.
   */
  void on_message(const std::shared_ptr<vsomeip::message> &_request) {
    std::cout << "Received a message with Client/Session [" << std::hex
              << std::setfill('0') << std::setw(4) << _request->get_client()
              << "/" << std::setw(4) << _request->get_session() << "]"
              << std::endl;

    std::shared_ptr<vsomeip::message> its_response =
        vsomeip::runtime::get()->create_response(_request);

    std::shared_ptr<vsomeip::payload> its_payload =
        vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> its_payload_data;
    for (std::size_t i = 0; i < 120; ++i)
      its_payload_data.push_back(vsomeip::byte_t(i % 256));
    its_payload->set_data(its_payload_data);
    its_response->set_payload(its_payload);

    app_->send(its_response);
  }

  /**
   * @brief Run the response example.
   */
  void run() {
    std::unique_lock<std::mutex> its_lock(mutex_);
    while (!blocked_)
      condition_.wait(its_lock);
    offer();
  }

private:
  std::shared_ptr<vsomeip::application> app_;
  bool is_registered_;
  bool use_static_routing_;

  std::mutex mutex_;
  std::condition_variable condition_;
  bool blocked_;
  bool running_;

  // blocked_ must be initialized before the thread is started.
  std::thread offer_thread_;
};

int main(int argc, char **argv) {
  bool use_static_routing(false);

  std::string static_routing_enable("--static-routing");

  for (int i = 1; i < argc; i++) {
    if (static_routing_enable == argv[i]) {
      use_static_routing = true;
    }
  }

  std::string path = "/mnt/workspace/cgz_workspace/Exercise/vsomeip_example/"
                     "config/request_response.json";
  response_example its_sample(use_static_routing, path);

  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
