// Copyright (C) 2014-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef VSOMEIP_EXAMPLES_SAMPLE_IDS_HPP
#define VSOMEIP_EXAMPLES_SAMPLE_IDS_HPP

/**
 * Method ID: 0-32767 for methods, 32768-65535 for events。32768=0x8000
 */

#define RequestResponse_SERVICE_ID 0x1234
#define RequestResponse_INSTANCE_ID 0x5678
#define RequestResponse_INSTANCE2_ID 0x8765
#define RequestResponse_METHOD_ID 0x0421

#define PublishSubscribe_SERVICE_ID 0x4321
#define PublishSubscribe_INSTANCE_ID 0x1111
#define PublishSubscribe_EVENTGROUP_ID 0x2222
#define PublishSubscribe_EVENT_ID 0x8888

#define FieldClient_SERVICE_ID 0x5678
#define FieldClient_INSTANCE_ID 0x1234
#define FieldClient_EVENTGROUP_ID 0x0022
#define FieldClient_EVENT_ID 0x9999
#define FieldClient_SET_METHOD_ID 0x0001
#define FieldClient_GET_METHOD_ID 0x0002

#endif // VSOMEIP_EXAMPLES_SAMPLE_IDS_HPP
