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
class request_sample {
public:
  /**
   * @brief Constructor
   * @param use_tcp Flag to indicate whether to use TCP
   * @param be_quiet Flag to indicate whether to be quiet
   * @param cycle The cycle time in milliseconds
   */
  request_sample(bool use_tcp, bool be_quiet, uint32_t cycle, std::string path)
      : app_(vsomeip::runtime::get()->create_application("request_example")),
        request_(vsomeip::runtime::get()->create_request(use_tcp)),
        use_tcp_(use_tcp), be_quiet_(be_quiet), cycle_(cycle), running_(true),
        blocked_(false), is_available_(false),
        sender_(std::bind(&request_sample::run, this)) {}

  /**
   * @brief Initialize the application
   * @return true
   */
  bool init() {
    std::cout << "Request example Initing!" << std::endl;
    /**
     * @brief Initialize the application
     *
     * @note 创建app后，必须首先调用init函数
     * @note
     * 如果app_name==""，则会使用环境变量VSOMEIP_APPLICATION_NAME作为app_name
     * @note 配置文件读取方式：
     * @note    1. 从环境变量VSOMEIP_CONFIGURATION读取配置文件路径
     * @note    2. 默认读取当前路径下的vsomeip.json配置文件
     * @note    3. 从默认路径/etc/vsomeip.json读取配置文件
     *
     * @return true if the application is initialized successfully
     */
    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }
    std::cout << "Request example Inited!" << std::endl;
    std::cout << "App name: " << app_->get_name() << std::endl;

    /**
     * @brief 注册状态处理函数
     * @note 改函数会在app regester和deregister时被调用
     * @note 改函数一般在start函数和stop函数之间调用
     *
     * @param _handler 状态处理函数，typedef std::function<void(state_type_e)>
     * state_handler_t;
     */
    app_->register_state_handler(
        std::bind(&request_sample::on_state, this, std::placeholders::_1));

    /**
     * @brief 注册消息处理函数
     * @note app必须注册消息处理函数，否则无法处理消息
     * @note 对于特定的service_id, instance_id,
     * method_id，只能注册一个handler，如果重复注册，会覆盖之前的handler
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _method 方法ID
     * @param _handler 消息处理函数, typedef std::function<void(const
     * std::shared_ptr<message>&)> message_handler_t;
     */
    app_->register_message_handler(
        vsomeip::ANY_SERVICE, RequestResponse_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&request_sample::on_message, this, std::placeholders::_1));

    // 设置请求报文的service_id, instance_id, method_id
    request_->set_service(RequestResponse_SERVICE_ID);
    request_->set_instance(RequestResponse_INSTANCE_ID);
    request_->set_method(RequestResponse_METHOD_ID);

    // 设置消息的payload
    std::shared_ptr<vsomeip::payload> its_payload =
        vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> its_payload_data;
    for (std::size_t i = 0; i < 10; ++i)
      its_payload_data.push_back(vsomeip::byte_t(i % 256));
    its_payload->set_data(its_payload_data);
    request_->set_payload(its_payload);

    /**
     * @brief 注册服务可用性处理函数
     * @note 该函数会在服务apper和disapper时被调用
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _handler 服务可用性处理函数，typedef std::function<void(service_t,
     * instance_t, bool)> availability_handler_t;
     * @param _major 服务的主版本号，默认为DEFAULT_MAJOR
     * @param _minor 服务的次版本号，默认为DEFAULT_MINOR
     */
    app_->register_availability_handler(
        RequestResponse_SERVICE_ID, RequestResponse_INSTANCE_ID,
        std::bind(&request_sample::on_availability, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    return true;
  }

  void start() {
    /**
     * @brief Start the application
     * @note
     * 该函数必须紧跟着init调用。该函数会一直阻塞，直到stop函数被调用或者ctrl+c
     * @note 该函数会处理收到的消息，并使用对应的registered
     * handler注册的函数处理消息
     */
    app_->start();
  }

  void stop() {
    running_ = false;
    blocked_ = true;
    /**
     * @brief 清空所有的registered handler
     */
    app_->clear_all_handler();
    /**
     * @brief 从routing总unregister当前service instance
     *
     * @note 当不再需要对应的service
     * instance时，需要调用该函数。该函数会从routing中unregister当前service
     * instance
     * @note 该函数可以避免someip router避免发送不必要的find service消息
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     */
    app_->release_service(RequestResponse_SERVICE_ID,
                          RequestResponse_INSTANCE_ID);
    condition_.notify_one();
    if (std::this_thread::get_id() != sender_.get_id()) {
      if (sender_.joinable()) {
        sender_.join();
      }
    } else {
      sender_.detach();
    }
    /**
     * @brief Stop the application
     * @note 该函数会停止处理消息，因此start函数会再该函数调用后返回
     */
    app_->stop();
  }

  /**
   * @brief Callback function to handle the state of the VSOMEIP application
   * @param _state The state of the VSOMEIP application
   */
  void on_state(vsomeip::state_type_e _state) {
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
      std::cout << "Application " << app_->get_name() << " is registered."
                << std::endl;

      /**
       * @brief 将当前app以client的身份注册到VSOMEIP router
       * @note app需要先调用该函数才能使用对应的service
       * instance。该函数会告诉VSOMEIP
       * router此请求，当服务可用时，将app注册到VSOMEIP router
       *
       * @param _service 服务ID
       * @param _instance 实例ID
       * @param _major 服务的主版本号，默认为ANY_MAJOR
       * @param _minor 服务的次版本号，默认为ANY_MINOR
       */
      app_->request_service(RequestResponse_SERVICE_ID,
                            RequestResponse_INSTANCE_ID);
    } else {
      std::cout << "Application " << app_->get_name() << " is deregistered."
                << std::endl;
    }
  }

  /**
   * @brief Callback function to handle the availability of a VSOMEIP service
   * @param _service The service ID
   * @param _instance The instance ID
   * @param _is_available Flag to indicate whether the service is available
   */
  void on_availability(vsomeip::service_t _service,
                       vsomeip::instance_t _instance, bool _is_available) {
    std::cout << "Service [" << std::hex << std::setfill('0') << std::setw(4)
              << _service << "." << _instance << "] is "
              << (_is_available ? "available." : "NOT available.") << std::endl;

    if (RequestResponse_SERVICE_ID == _service &&
        RequestResponse_INSTANCE_ID == _instance) {
      if (is_available_ && !_is_available) {
        is_available_ = false;
      } else if (_is_available && !is_available_) {
        is_available_ = true;
        send();
      }
    }
  }

  /**
   * @brief Callback function to handle the response to a VSOMEIP request
   * @param _response The response message
   */
  void on_message(const std::shared_ptr<vsomeip::message> &_response) {
    std::cout << "Received a response from "
              << "service: " << std::hex << std::setfill('0') << std::setw(4)
              << _response->get_service() << ", instance: " << std::hex
              << std::setfill('0') << std::setw(4) << _response->get_instance()
              << ", client: " << std::hex << std::setfill('0') << std::setw(4)
              << _response->get_client() << ", method: " << std::hex
              << std::setfill('0') << std::setw(4) << _response->get_method()
              << ", message: " << _response->get_message()
              << ", length: " << _response->get_length()
              << ", request: " << _response->get_request()
              << ", session: " << _response->get_session()
              << ", protocol version: " << _response->get_protocol_version()
              << ", interface version: " << _response->get_interface_version()
              << ", message type: "
              << message_map.at(_response->get_message_type())
              << ", return code: "
              << return_code_map.at(_response->get_return_code())
              << ", is reliable: " << _response->is_reliable()
              << ", is valid crc: " << _response->is_valid_crc() << std::endl;
  }

  void send() {
    std::lock_guard<std::mutex> its_lock(mutex_);
    blocked_ = true;
    condition_.notify_one();
  }

  void run() {
    while (running_) {
      {
        std::unique_lock<std::mutex> its_lock(mutex_);
        while (!blocked_)
          condition_.wait(its_lock);
        if (is_available_) {
          /**
           * @brief Send a message
           *
           * @note 将message序列化后，找到对应的target，并发送给target
           * @note
           * 对于消息中的request_id，其会自动使用client_id和session_id进行拼装
           *
           * @param _message message对象
           */
          app_->send(request_);
          std::cout << "Client/Session [" << std::hex << std::setfill('0')
                    << std::setw(4) << request_->get_client() << "/"
                    << std::setw(4) << request_->get_session()
                    << "] sent a request to Service [" << std::setw(4)
                    << request_->get_service() << "." << std::setw(4)
                    << request_->get_instance() << "]" << std::endl;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(cycle_));
    }
  }

private:
  /// the VSOMEIP application
  std::shared_ptr<vsomeip::application> app_;
  /// the request message
  std::shared_ptr<vsomeip::message> request_;
  /// the flag to indicate whether to use TCP
  bool use_tcp_;
  bool be_quiet_;
  uint32_t cycle_;
  /// 用于控制线程的运行
  std::mutex mutex_;
  /// 用于控制线程的运行
  std::condition_variable condition_;
  /// 当前程序是否运行
  bool running_;
  bool blocked_;
  /// 服务是否可用
  bool is_available_;

  /// 循环发送请求的线程
  std::thread sender_;
};

int main(int argc, char **argv) {
  bool use_tcp = true;
  bool be_quiet = false;
  uint32_t cycle = 1000; // Default: 1s
  std::string path = "/mnt/workspace/cgz_workspace/Exercise/vsomeip_example/"
                     "config/request_response.json";

  request_sample its_sample(use_tcp, be_quiet, cycle, path);

  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
