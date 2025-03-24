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

class field_client_example {
public:
  field_client_example(bool _use_tcp)
      : app_(vsomeip::runtime::get()->create_application(
            "field_client_example")),
        use_tcp_(_use_tcp) {}

  bool init() {
    if (!app_->init()) {
      std::cerr << "Couldn't initialize application" << std::endl;
      return false;
    }

    app_->register_state_handler(std::bind(&field_client_example::on_state,
                                           this, std::placeholders::_1));

    app_->register_message_handler(vsomeip::ANY_SERVICE,
                                   FieldClient_INSTANCE_ID, vsomeip::ANY_METHOD,
                                   std::bind(&field_client_example::on_message,
                                             this, std::placeholders::_1));

    app_->register_availability_handler(
        FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
        std::bind(&field_client_example::on_availability, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3));

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(FieldClient_EVENTGROUP_ID);
    app_->request_event(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                        FieldClient_EVENT_ID, its_groups,
                        vsomeip::event_type_e::ET_FIELD);
    app_->subscribe(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                    FieldClient_EVENTGROUP_ID);

    return true;
  }

  void start() { app_->start(); }

  void stop() {
    app_->clear_all_handler();
    app_->unsubscribe(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                      FieldClient_EVENTGROUP_ID);
    app_->release_event(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID,
                        FieldClient_EVENT_ID);
    app_->release_service(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID);
    app_->stop();
  }

  void on_state(vsomeip::state_type_e _state) {
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
      std::cout << "Application " << app_->get_name() << " is registered."
                << std::endl;
      app_->request_service(FieldClient_SERVICE_ID, FieldClient_INSTANCE_ID);
    } else {
      std::cout << "Application " << app_->get_name() << " is deregistered."
                << std::endl;
    }
  }

  void on_availability(vsomeip::service_t _service,
                       vsomeip::instance_t _instance, bool _is_available) {
    std::cout << "Service [" << std::hex << std::setfill('0') << std::setw(4)
              << _service << "." << _instance << "] is "
              << (_is_available ? "available." : "NOT available.") << std::endl;
  }

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

  field_client_example its_sample(use_tcp);
  if (its_sample.init()) {
    its_sample.start();
    return 0;
  } else {
    return 1;
  }
}
