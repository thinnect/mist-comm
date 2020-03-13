#ifndef MIST_COMM_PRIVATE_H_
#define MIST_COMM_PRIVATE_H_

#include "mist_comm.h"

struct comms_msg {
	struct {
		am_id_t type; // AMID, port?
		uint8_t source[COMMS_MSG_ADDRESSING_SIZE];
		uint8_t destination[COMMS_MSG_ADDRESSING_SIZE];
		uint8_t header[COMMS_MSG_HEADER_SIZE];

		uint8_t length;
		uint8_t payload[COMMS_MSG_PAYLOAD_SIZE];

		uint8_t footer[COMMS_MSG_FOOTER_SIZE];
		uint8_t metadata[COMMS_MSG_METADATA_SIZE];
	} body;
};

// Receiver structure ----------------------------------------------------------
struct comms_receiver { // Members are private, should not be accessed
	am_id_t           type;
	comms_receive_f*  callback;
	void*             user;
	comms_receiver_t* next;
};

// Sleep controller structure --------------------------------------------------
struct comms_sleep_controller {
	comms_layer_t* comms;

	bool block;
	bool pending;

	comms_status_change_f* cb;
	void* user;

	comms_sleep_controller_t* next;
};

// Deferred calls --------------------------------------------------------------

typedef void comms_deferred_f(void * argument);

void _comms_deferred_init(comms_layer_t * comms, void ** deferred, comms_deferred_f * cb);

void _comms_defer(void * deferred);

void _comms_deferred_deinit(void * deferred);

#endif//MIST_COMM_PRIVATE_H_
