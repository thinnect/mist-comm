#ifndef MIST_COMM_PRIVATE_H_
#define MIST_COMM_PRIVATE_H_

#include "mist_comm.h"

typedef struct comms_msg {
	struct {
		uint16_t application; // AMID, port?
		uint8_t source[COMMS_MSG_ADDRESSING_SIZE];
		uint8_t destination[COMMS_MSG_ADDRESSING_SIZE];
		uint8_t header[COMMS_MSG_HEADER_SIZE];

		uint8_t length;
		uint8_t payload[COMMS_MSG_PAYLOAD_SIZE];

		uint8_t footer[COMMS_MSG_FOOTER_SIZE];
		uint8_t metadata[COMMS_MSG_METADATA_SIZE];
	} body;
	comms_msg_t* next; // ... maybe, could be very useful for transitioning through layers
} comms_msg_t;

typedef struct comms_receiver { // Members are private, should not be accessed
	uint16_t          application;
	comms_receive_f*  callback;
	void*             user;
	comms_receiver_t* next;
} comms_receiver_t;

#endif//MIST_COMM_PRIVATE_H_
