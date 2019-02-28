#ifndef MIST_COMM_AM_H_
#define MIST_COMM_AM_H_

#include "mist_comm.h"
#include "mist_comm_iface.h"

typedef struct comms_layer_am comms_layer_am_t;

typedef am_addr_t comms_am_get_destination_f(comms_layer_am_t*, comms_msg_t*);
typedef void comms_am_set_destination_f(comms_layer_am_t*, comms_msg_t*, am_addr_t);

typedef am_addr_t comms_am_get_source_f(comms_layer_am_t*, comms_msg_t*);
typedef void comms_am_set_source_f(comms_layer_am_t*, comms_msg_t*, am_addr_t);

typedef struct comms_layer_am {
	comms_layer_iface_t base;

	comms_am_get_destination_f* am_get_destination;
	comms_am_set_destination_f* am_set_destination;

	comms_am_get_source_f* am_get_source;
	comms_am_set_source_f* am_set_source;
} comms_layer_am_t;

// Message manipulation functions for ActiveMessage layer ----------------------
am_addr_t comms_am_get_destination(comms_msg_t* msg);
void comms_am_set_destination(comms_msg_t* msg, am_addr_t dest);

am_addr_t comms_am_get_source(comms_msg_t* msg);
void comms_am_set_source(comms_msg_t* msg, am_addr_t source);
// -----------------------------------------------------------------------------

comms_error_t comms_am_create(comms_layer_t* layer);

#endif//MIST_COMM_AM_H_
