#include <string>
#include <unordered_map>

#include "vsomeip/enumeration_types.hpp"

const std::unordered_map<vsomeip::state_type_e, std::string> state_map = {
    {vsomeip::state_type_e::ST_REGISTERED, "ST_REGISTERED"},
    {vsomeip::state_type_e::ST_DEREGISTERED, "ST_DEREGISTERED"},
};

const std::unordered_map<vsomeip::message_type_e, std::string> message_map = {
    {vsomeip::message_type_e::MT_REQUEST, "MT_REQUEST"},
    {vsomeip::message_type_e::MT_REQUEST_NO_RETURN, "MT_REQUEST_NO_RETURN"},
    {vsomeip::message_type_e::MT_NOTIFICATION, "MT_NOTIFICATION"},
    {vsomeip::message_type_e::MT_REQUEST_ACK, "MT_REQUEST_ACK"},
    {vsomeip::message_type_e::MT_REQUEST_NO_RETURN_ACK,
     "MT_REQUEST_NO_RETURN_ACK"},
    {vsomeip::message_type_e::MT_NOTIFICATION_ACK, "MT_NOTIFICATION_ACK"},
    {vsomeip::message_type_e::MT_RESPONSE, "MT_RESPONSE"},
    {vsomeip::message_type_e::MT_ERROR, "MT_ERROR"},
    {vsomeip::message_type_e::MT_RESPONSE_ACK, "MT_RESPONSE_ACK"},
    {vsomeip::message_type_e::MT_ERROR_ACK, "MT_ERROR_ACK"},
    {vsomeip::message_type_e::MT_UNKNOWN, "MT_UNKNOWN"},
};

const std::unordered_map<vsomeip::return_code_e, std::string> return_code_map =
    {
        {vsomeip::return_code_e::E_OK, "E_OK"},
        {vsomeip::return_code_e::E_NOT_OK, "E_NOT_OK"},
        {vsomeip::return_code_e::E_UNKNOWN_SERVICE, "E_UNKNOWN_SERVICE"},
        {vsomeip::return_code_e::E_UNKNOWN_METHOD, "E_UNKNOWN_METHOD"},
        {vsomeip::return_code_e::E_NOT_READY, "E_NOT_READY"},
        {vsomeip::return_code_e::E_NOT_REACHABLE, "E_NOT_REACHABLE"},
        {vsomeip::return_code_e::E_TIMEOUT, "E_TIMEOUT"},
        {vsomeip::return_code_e::E_WRONG_PROTOCOL_VERSION,
         "E_WRONG_PROTOCOL_VERSION"},
        {vsomeip::return_code_e::E_WRONG_INTERFACE_VERSION,
         "E_WRONG_INTERFACE_VERSION"},
        {vsomeip::return_code_e::E_MALFORMED_MESSAGE, "E_MALFORMED_MESSAGE"},
        {vsomeip::return_code_e::E_WRONG_MESSAGE_TYPE, "E_WRONG_MESSAGE_TYPE"},
        {vsomeip::return_code_e::E_UNKNOWN, "E_UNKNOWN"},
};

const std::unordered_map<vsomeip::routing_state_e, std::string>
    running_state_map = {
        {vsomeip::routing_state_e::RS_RUNNING, "RS_RUNNING"},
        {vsomeip::routing_state_e::RS_SUSPENDED, "RS_SUSPENDED"},
        {vsomeip::routing_state_e::RS_RESUMED, "RS_RESUMED"},
        {vsomeip::routing_state_e::RS_SHUTDOWN, "RS_SHUTDOWN"},
        {vsomeip::routing_state_e::RS_DIAGNOSIS, "RS_DIAGNOSIS"},
        {vsomeip::routing_state_e::RS_UNKNOWN, "RS_UNKNOWN"},
};

const std::unordered_map<vsomeip::offer_type_e, std::string> offer_type_map = {
    {vsomeip::offer_type_e::OT_LOCAL, "OT_LOCAL"},
    {vsomeip::offer_type_e::OT_REMOTE, "OT_REMOTE"},
    {vsomeip::offer_type_e::OT_ALL, "OT_ALL"},
};

const std::unordered_map<vsomeip::event_type_e, std::string> event_type_map = {
    {vsomeip::event_type_e::ET_EVENT, "ET_EVENT"},
    {vsomeip::event_type_e::ET_SELECTIVE_EVENT, "ET_SELECTIVE_EVENT"},
    {vsomeip::event_type_e::ET_FIELD, "ET_FIELD"},
    {vsomeip::event_type_e::ET_UNKNOWN, "ET_UNKNOWN"},
};

const std::unordered_map<vsomeip::security_mode_e, std::string>
    security_mode_map = {
        {vsomeip::security_mode_e::SM_OFF, "SM_OFF"},
        {vsomeip::security_mode_e::SM_ON, "SM_ON"},
        {vsomeip::security_mode_e::SM_AUDIT, "SM_AUDIT"},
};

const std::unordered_map<vsomeip::reliability_type_e, std::string>
    reliability_type_map = {
        {vsomeip::reliability_type_e::RT_RELIABLE, "RT_RELIABLE"},
        {vsomeip::reliability_type_e::RT_UNRELIABLE, "RT_UNRELIABLE"},
        {vsomeip::reliability_type_e::RT_BOTH, "RT_BOTH"},
        {vsomeip::reliability_type_e::RT_UNKNOWN, "RT_UNKNOWN"},
};