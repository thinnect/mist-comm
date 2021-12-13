#ifndef MIST_COMM_AM_MSG_H
#define MIST_COMM_AM_MSG_H

#include "mist_comm_basics.h"

typedef struct comms_am_msg_header {
} comms_am_msg_header_t;

typedef struct comms_am_msg_footer {
} comms_am_msg_footer_t;

typedef struct comms_am_msg_metadata {
	int8_t rssi;
	uint8_t lqi;

	uint8_t sent;
	uint8_t retries;
	uint32_t timeout;

	bool ack_required;
	bool ack_received;

	bool timestamp_valid;
	uint32_t timestamp;

	bool event_time_valid;
	uint32_t event_time;

	bool priority_valid;
	uint8_t priority;
}__attribute__((packed))comms_am_msg_metadata_t;

#endif//MIST_COMM_AM_MSG_H
