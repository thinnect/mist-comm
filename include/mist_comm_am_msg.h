#ifndef MIST_COMM_AM_MSG_H_
#define MIST_COMM_AM_MSG_H_

#include "mist_comm_basics.h"

typedef struct comms_am_msg_header {
	am_addr_t source;
	am_addr_t destination;
	am_group_t group;
	am_id_t amid;
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

	bool event_time_valid;
	uint32_t event_time;
} comms_am_msg_metadata_t;

#endif//MIST_COMM_AM_H_
