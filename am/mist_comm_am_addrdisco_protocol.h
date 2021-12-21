/**
 * EUI vs link-local address discovery protocol for Mist over ActiveMessage.
 *
 * The message is sent to broadcast with a specific EUI64 in the payload
 * to discover the link-local ActiveMessage address.
 * The message is sent to unicast with the guid field set to all ones, to
 * discover the EUI64 address.
 * A broadcast message with the guid field set to all ones is forbidden.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_AM_ADDRDISCO_PROTOCOL_H_
#define MIST_COMM_AM_ADDRDISCO_PROTOCOL_H_

#include "eui64.h"

#define AMID_ADDRESS_DISCOVERY 0xFC

enum GuidDiscoveryHeaders
{
	GUIDDISCOVERY_REQUEST  = 1,
	GUIDDISCOVERY_RESPONSE = 2
};

#pragma pack(push, 1)
typedef struct am_addrdisco_packet
{
	uint8_t header;
	uint8_t guid[IEEE_EUI64_LENGTH];
} am_addrdisco_packet_t;
#pragma pack(pop)

#endif//MIST_COMM_AM_ADDRDISCO_PROTOCOL_H_
