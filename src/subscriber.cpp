// Copyright (C) 2014-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
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
#include "type_map.hpp"

/**
 * @brief This class represents a subscribe example.
 */
class subscribe_example {
public:
  /**
   * @brief Construct the subscribe example.
   * @param _use_tcp Use TCP or not.
   */
  subscribe_example(bool _use_tcp)
      : app_(vsomeip::runtime::get()->create_application("subscribe_example")),
        use_tcp_(_use_tcp) {}

  /**
   * @brief Initialize the subscribe example.
   * @return True if the subscribe example is initialized successfully.
   */
  bool init() {
    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }

    app_->register_state_handler(
        std::bind(&subscribe_example::on_state, this, std::placeholders::_1));

    app_->register_message_handler(
        vsomeip::ANY_SERVICE, PublishSubscribe_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&subscribe_example::on_message, this, std::placeholders::_1));

    app_->register_availability_handler(
        PublishSubscribe_SERVICE_ID, PublishSubscribe_INSTANCE_ID,
        std::bind(&subscribe_example::on_availability, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3));

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(PublishSubscribe_EVENTGROUP_ID);
    /**
     * @brief
     * app需要调用改方法从而接受event或者field，改方法会在routing中进行注册
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _event 事件ID
     * @param _eventgroups 事件组集合，每个事件组都包含当前事件
     * @param _type 事件类型，默认为ET_EVENT
     * @param _reliability
     * 可靠性类型，默认为RT_UNKNOWN。设计上不是RT_UNRELIABLE，就是RT_RELIABLE。并且同一个eventgroup中的event需要可靠性类型一致
     */
    app_->request_event(PublishSubscribe_SERVICE_ID,
                        PublishSubscribe_INSTANCE_ID, PublishSubscribe_EVENT_ID,
                        its_groups, vsomeip::event_type_e::ET_EVENT);

    /**
     * @brief 订阅服务
     * @note
     * app必须调用改方法去订阅eventgroups。并且需要先调用request_event方法，将其所有感兴趣的event注册到routing中
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _eventgroup 事件组ID
     * @param _major 服务的Major版本号，默认为DEFAULT_MAJOR
     * @param _event 事件ID，默认为ANY_EVENT,表示订阅所有事件
     */
    app_->subscribe(PublishSubscribe_SERVICE_ID, PublishSubscribe_INSTANCE_ID,
                    PublishSubscribe_EVENTGROUP_ID);

    return true;
  }

  /**
   * @brief Start the subscribe example.
   */
  void start() { app_->start(); }

  /**
   * @brief Stop the subscribe example.
   */
  void stop() {
    app_->clear_all_handler();
    /**
     * @brief unsubscribe特定的eventgroup
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _eventgroup 事件组ID
     */
    app_->unsubscribe(PublishSubscribe_SERVICE_ID, PublishSubscribe_INSTANCE_ID,
                      PublishSubscribe_EVENTGROUP_ID);
    /**
     * @brief 从vsomeip routing中注销当前app之前注册的event
     *
     * @param _service 服务ID
     * @param _instance 实例ID
     * @param _event 事件ID
     */
    app_->release_event(PublishSubscribe_SERVICE_ID,
                        PublishSubscribe_INSTANCE_ID,
                        PublishSubscribe_EVENT_ID);
    app_->release_service(PublishSubscribe_SERVICE_ID,
                          PublishSubscribe_INSTANCE_ID);
    app_->stop();
  }

  /**
   * @brief Callback function for the state of the application.
   * @param _state State of the application.
   */
  void on_state(vsomeip::state_type_e _state) {
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
      std::cout << "Application " << app_->get_name() << " is registered."
                << std::endl;
      app_->request_service(PublishSubscribe_SERVICE_ID,
                            PublishSubscribe_INSTANCE_ID);
    } else {
      std::cout << "Application " << app_->get_name() << " is deregistered."
                << std::endl;
    }
  }

  /**
   * @brief Callback function for the availability of the service.
   * @param _service Service ID.
   * @param _instance Instance ID.
   * @param _is_available True if the service is available.
   */
  void on_availability(vsomeip::service_t _service,
                       vsomeip::instance_t _instance, bool _is_available) {
    std::cout << "Service [" << std::hex << std::setfill('0') << std::setw(4)
              << _service << "." << _instance << "] is "
              << (_is_available ? "available." : "NOT available.") << std::endl;
  }

  /**
   * @brief Callback function for the message.
   * @param _response Message.
   */
  void on_message(const std::shared_ptr<vsomeip::message> &_response) {
    std::stringstream ss;
    ss << "Received a notify from "
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
       << ", message type: " << message_map.at(_response->get_message_type())
       << ", return code: " << return_code_map.at(_response->get_return_code())
       << ", is reliable: " << _response->is_reliable()
       << ", is valid crc: " << _response->is_valid_crc();
    std::cout << ss.str() << std::endl;

    std::stringstream its_message;
    its_message << "Received a notification for Event [" << std::hex
                << std::setfill('0') << std::setw(4) << _response->get_service()
                << "." << std::setw(4) << _response->get_instance() << "."
                << std::setw(4) << _response->get_method()
                << "] to Client/Session [" << std::setw(4)
                << _response->get_client() << "/" << std::setw(4)
                << _response->get_session() << "] = ";

    std::shared_ptr<vsomeip::payload> its_payload = _response->get_payload();
    its_message << "(" << std::dec << its_payload->get_length() << ") "
                << std::hex;
    for (uint32_t i = 0; i < its_payload->get_length(); ++i)
      its_message << std::setw(2)
                  << static_cast<int>(its_payload->get_data()[i]) << " ";
    std::cout << its_message.str() << std::endl;
  }

private:
  std::shared_ptr<vsomeip::application> app_;
  bool use_tcp_;
};

int main(int argc, char **argv) {
  bool use_tcp = true;

  subscribe_example its_sample(use_tcp);
  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
