/**
 * Mist communications API for TinyOS ActiveMessage.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_AM_H
#define MIST_COMM_AM_H

#include "mist_comm.h"
#include "mist_comm_iface.h"
#include "mist_comm_addrcache.h"
#include "mist_comm_am_addrdisco.h"

/**
 * Structure type definition for an ActiveMessage comms layer.
 */
typedef struct comms_layer_am comms_layer_am_t;

/**
 * ActiveMessage address of the communications layer.
 * @param comms Pointer to a comms layer.
 * @return      ActiveMessage address.
 */
am_addr_t comms_am_address (comms_layer_t * comms);

/**
 * Get the destination of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Destination address.
 */
am_addr_t comms_am_get_destination (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the destination of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param dest  Destination address.
 */
void comms_am_set_destination (comms_layer_t * comms, comms_msg_t * msg, am_addr_t dest);

/**
 * Get the source of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Source address.
 */
am_addr_t comms_am_get_source (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the source of the message. In most cases this is done automatically!
 *
 * @param comms  Pointer to a comms layer.
 * @param msg    Pointer to a message.
 * @param source Source address.
 */
void comms_am_set_source (comms_layer_t * comms, comms_msg_t * msg, am_addr_t source);

/**
 * Create an ActiveMessage comms layer.
 *
 * @param layer   Pointer to a layer structure to initialize.
 * @param address ActiveMessage address for this layer
 * @param sendf   Actual send function to use for sending the message.
 * @param plenf   Max payload length function for the underlying comms component.
 * @param startf  Layer start function (can be NULL, if always active).
 * @param stopf   Layer stop function (can be NULL, if always active).
 * @return        COMMS_SUCCESS if initialized correctly.
 */
comms_error_t comms_am_create (comms_layer_t * layer, am_addr_t address,
                               comms_send_f * sendf, comms_plen_f * plenf,
                               comms_start_f * startf, comms_stop_f * stopf);


// -----------------------------------------------------------------------------

typedef am_addr_t comms_am_addr_f (comms_layer_am_t*);

typedef am_addr_t comms_am_get_destination_f(comms_layer_am_t*, const comms_msg_t*);
typedef void comms_am_set_destination_f(comms_layer_am_t*, comms_msg_t*, am_addr_t);

typedef am_addr_t comms_am_get_source_f(comms_layer_am_t*, const comms_msg_t*);
typedef void comms_am_set_source_f(comms_layer_am_t*, comms_msg_t*, am_addr_t);

struct comms_layer_am
{
	comms_layer_iface_t base;

	comms_am_get_destination_f * am_get_destination;
	comms_am_set_destination_f * am_set_destination;

	comms_am_get_source_f * am_get_source;
	comms_am_set_source_f * am_set_source;

	comms_am_addr_f * am_address;

	comms_send_f * link_sendf;
	comms_plen_f * link_plenf;

	am_addr_t am_addr;

	comms_addr_cache_t * cache;

	am_addrdisco_t * disco;
};

#endif//MIST_COMM_AM_H
