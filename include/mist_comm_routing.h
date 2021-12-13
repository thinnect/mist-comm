/**
 * Routing info callbacks for the MistComm API.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_ROUTING_H
#define MIST_COMM_ROUTING_H

#include "mist_comm_am.h"
#include <string.h>

typedef struct comms_layer_am_mh comms_layer_am_mh_t;
typedef struct comms_routing_info_handler comms_routing_info_handler_t;

comms_error_t comms_init_routing_result_callbacks (comms_layer_am_mh_t * comms);

void comms_routing_notify_routed (comms_layer_t * comms, am_addr_t dest, uint16_t cost, comms_error_t status);

typedef void comms_am_routed_f(comms_layer_t *, am_addr_t, uint16_t, comms_error_t, void*);

comms_error_t comms_routing_register_result_callback (comms_layer_am_mh_t * comms,
                                                      comms_routing_info_handler_t * rih,
                                                      comms_am_routed_f * func, void * user);

comms_error_t comms_routing_deregister_result_callback (comms_layer_am_mh_t * comms,
                                                        comms_routing_info_handler_t * rih);

struct comms_routing_info_handler
{
	comms_am_routed_f * callback;
	void * user;
	comms_routing_info_handler_t * next;
};

struct comms_layer_am_mh {
	comms_layer_am_t base;
	comms_routing_info_handler_t * routing_info_handlers;
};

#endif//MIST_COMM_ROUTING_H
