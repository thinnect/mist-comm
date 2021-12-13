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

// A communications layer that extends comms_layer_am_t with routing info.
typedef struct comms_layer_am_mh comms_layer_am_mh_t;

// Structure for a routing info handler.
typedef struct comms_routing_info_handler comms_routing_info_handler_t;

/**
 * Initialize routing callbacks for the communication layer.
 *
 * @param comms Pointer to a comms layer.
 * @return      COMMS_SUCCESS if intialized, an error otherwise.
 */
comms_error_t comms_init_routing_result_callbacks (comms_layer_am_mh_t * comms);

/**
 * Notify routing info handlers about a routing result.
 *
 * @param comms  Pointer to a comms layer.
 * @param dest   Destination that was routed.
 * @param cost   Route cost ... implementation specific.
 * @param status COMMS_SUCCESS if routed successfully.
 */
void comms_routing_notify_routed (comms_layer_t * comms,
                                  am_addr_t dest, uint16_t cost, comms_error_t status);

/**
 * Routing result callback type definition.
 *
 * @param comms  Pointer to a comms layer.
 * @param dest   Destination that was routed.
 * @param cost   Route cost ... implementation specific.
 * @param status COMMS_SUCCESS if routed successfully.
 * @param user   User pointer passed during registration.
 */
typedef void comms_am_routed_f (comms_layer_t * comms,
                                am_addr_t dest, uint16_t cost, comms_error_t result,
                                void * user);

/**
 * Register a routing info handler.
 *
 * @param comms  Pointer to a comms layer.
 * @param rih    Pointer to routing info handler.
 * @param func   Routing info callback function.
 * @param user   User pointer to pass to callback, when called.
 * @return       COMMS_SUCCESS if registered.
 */
comms_error_t comms_routing_register_result_callback (comms_layer_am_mh_t * comms,
                                                      comms_routing_info_handler_t * rih,
                                                      comms_am_routed_f * func, void * user);

/**
 * De-register a routing info handler.
 *
 * @param comms  Pointer to a comms layer.
 * @param rih    Pointer to routing info handler.
 * @return       COMMS_SUCCESS if no longer registered.
 */
comms_error_t comms_routing_deregister_result_callback (comms_layer_am_mh_t * comms,
                                                        comms_routing_info_handler_t * rih);

/**
 * Routing info handler structure.
 */
struct comms_routing_info_handler
{
	comms_am_routed_f * callback;
	void * user;
	comms_routing_info_handler_t * next; // A list of handlers is made for their storage
};

/**
 * Routing info capable communications layer structure.
 */
struct comms_layer_am_mh
{
	comms_layer_am_t base;
	comms_routing_info_handler_t * routing_info_handlers;
};

#endif//MIST_COMM_ROUTING_H
