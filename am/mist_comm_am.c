/**
 * TinyOS ActiveMessage transport layer implementation.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */

#include "mist_comm_iface.h"
#include "mist_comm_ref.h"
#include "mist_comm_am.h"
#include "mist_comm_addrcache.h"

#include <stdbool.h>
#include <string.h>

am_addr_t comms_am_address(comms_layer_t* comms) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	return amcomms->am_address(amcomms);
}

static am_addr_t am_comms_addr(comms_layer_am_t* comms) {
	return comms->am_addr;
}

static void am_comms_init_message(comms_layer_iface_t* comms, comms_msg_t* msg) {
	comms_am_msg_metadata_t* metadata = (comms_am_msg_metadata_t*)(msg->body.metadata);
	memset(&(msg->body.destination), 0, sizeof(msg->body.destination));
	memset(&(msg->body.source), 0, sizeof(msg->body.source));
	msg->body.length = 0;
	metadata->timestamp_valid = false;
	metadata->event_time_valid = false;
	metadata->ack_required = false;
	metadata->ack_received = false;
	metadata->timeout = 0;
	metadata->retries = 0;
	metadata->sent = 0;
	metadata->lqi= 0;
	metadata->rssi = -128;
}

static bool am_comms_deliver(comms_layer_iface_t* iface, comms_msg_t* msg) {
	comms_layer_t* comms = (comms_layer_t*)iface;
	comms_layer_am_t* amcomms = (comms_layer_am_t*)iface;
	am_addr_t dest = comms_am_get_destination(comms, msg);
	comms_error_t result;

	// Attach the eui address of the source
	if(false == comms_cache_get_eui(amcomms->cache,
	                                &(msg->body.source.local), &(msg->body.source.eui)))
	{
		eui64_set_zeros(&(msg->body.destination.eui));
	}

	// Attach the eui address of the destination
	if(AM_BROADCAST_ADDR == dest)
	{
		eui64_set_ones(&(msg->body.destination.eui));
	}
	else if(amcomms->am_addr == dest)
	{
		memcpy(&(msg->body.destination.eui), &(comms->eui), sizeof(ieee_eui64_t));
	}
	else if(false == comms_cache_get_eui(amcomms->cache,
	                                     &(msg->body.destination.local), &(msg->body.destination.eui)))
	{
		eui64_set_zeros(&(msg->body.destination.eui));
	}

	result = comms_basic_deliver(comms, msg);

	// Request eui source address discovery, if it was necessary for receiver
	if(COMMS_NO_ADDR == result)
	{
		comms_am_addrdisco_discover_eui(amcomms->disco, comms_am_get_source(comms, msg));
	}

	return result == COMMS_SUCCESS;
}

static comms_error_t am_comms_send(comms_layer_iface_t* iface, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	comms_layer_t* comms = (comms_layer_t*)iface;
	comms_layer_am_t* amcomms = (comms_layer_am_t*)iface;

	if(0 == comms_am_get_destination(comms, msg)) // Need to see if there is something in the address cache
	{
		if(eui64_is_ones(&(msg->body.destination.eui)))
		{
			comms_am_set_destination(comms, msg, AM_BROADCAST_ADDR);
		}
		else if(false == comms_cache_get_local(amcomms->cache,
                                          &(msg->body.destination.eui), &(msg->body.destination.local)))
		{
			return COMMS_NO_ADDR; // Nothing retrieved from cache
		}
	}
	return amcomms->link_sendf(iface, msg, sdf, user);
}

static am_id_t am_comms_get_packet_type(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return 0xFF & msg->body.type;
}
static void am_comms_set_packet_type(comms_layer_iface_t* comms, comms_msg_t* msg, am_id_t ptype) {
	msg->body.type = 0x3F00 + ptype;
}

static uint16_t am_comms_get_packet_group(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return msg->body.group;
}
static void am_comms_set_packet_group(comms_layer_iface_t* comms, comms_msg_t* msg, uint16_t group) {
	msg->body.group = group;
}

static uint8_t am_comms_get_payload_max_length(comms_layer_iface_t* iface) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)iface;
	return amcomms->link_plenf(iface);
}
static uint8_t am_comms_get_payload_length(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return msg->body.length;
}
static void am_comms_set_payload_length(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t length) {
	msg->body.length = length;
}
static void* am_comms_get_payload(comms_layer_iface_t* comms, const comms_msg_t* msg, uint8_t length) {
	if(length < sizeof(msg->body.payload)) {
		return (void*)(msg->body.payload);
	}
	return NULL;
}

static uint8_t am_comms_get_retries(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->retries;
}
static comms_error_t am_comms_set_retries(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t count) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->retries = count;
	return COMMS_SUCCESS;
}

static uint8_t am_comms_get_retries_used(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->sent;
}
static comms_error_t am_comms_set_retries_used(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t count) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->sent = count;
	return COMMS_SUCCESS;
}

static uint32_t am_comms_get_timeout(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->timeout;
}
static comms_error_t am_comms_set_timeout(comms_layer_iface_t* comms, comms_msg_t* msg, uint32_t timeout) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->timeout = timeout;
	return COMMS_SUCCESS;
}

static bool am_comms_is_ack_required(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->ack_required;
}
static comms_error_t am_comms_set_ack_required(comms_layer_iface_t* comms, comms_msg_t* msg, bool required) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->ack_required = required;
	return COMMS_SUCCESS;
}
static bool am_comms_ack_received(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->ack_received;
}
static void am_comms_set_ack_received(comms_layer_iface_t* comms, comms_msg_t* msg) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->ack_received = true;
}

static comms_error_t am_comms_set_timestamp(comms_layer_iface_t* comms, comms_msg_t* msg, uint32_t timestamp) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->timestamp = timestamp;
	((comms_am_msg_metadata_t*)(msg->body.metadata))->timestamp_valid = true;
	return COMMS_SUCCESS;
}
static uint32_t am_comms_get_timestamp(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->timestamp;
}
static bool am_comms_timestamp_valid(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->timestamp_valid;
}

static comms_error_t am_comms_set_event_time(comms_layer_iface_t* comms, comms_msg_t* msg, uint32_t evt) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->event_time = evt;
	((comms_am_msg_metadata_t*)(msg->body.metadata))->event_time_valid = true;
	return COMMS_SUCCESS;
}
static uint32_t am_comms_get_event_time(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->event_time;
}
static bool am_comms_event_time_valid(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->event_time_valid;
}

static uint8_t am_comms_get_lqi(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->lqi;
}
static void am_comms_set_lqi(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t lqi) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->lqi = lqi;
}

static int8_t am_comms_get_rssi(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return ((comms_am_msg_metadata_t*)(msg->body.metadata))->rssi;
}
static void am_comms_set_rssi(comms_layer_iface_t* comms, comms_msg_t* msg, int8_t rssi) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->rssi = rssi;
}

static uint8_t am_comms_get_priority(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	if (true == ((comms_am_msg_metadata_t*)(msg->body.metadata))->priority_valid)
	{
		return ((comms_am_msg_metadata_t*)(msg->body.metadata))->priority;
	}
	else
	{
		return 0xFF;
	}
}
static void am_comms_set_priority(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t priority) {
	((comms_am_msg_metadata_t*)(msg->body.metadata))->priority_valid = true;
	((comms_am_msg_metadata_t*)(msg->body.metadata))->priority = priority;
}

static am_addr_t am_comms_get_destination(comms_layer_am_t* comms, const comms_msg_t* msg) {
#ifdef ALIGN_CM0
	return *((__packed am_addr_t*)msg->body.destination.local.data);
#else
	return *((am_addr_t*)msg->body.destination.local.data);
#endif
}
static void am_comms_set_destination(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t dest) {
	memset(&(msg->body.destination.local), 0, sizeof(msg->body.destination.local));
#ifdef ALIGN_CM0
	*((__packed am_addr_t*)msg->body.destination.local.data) = dest;
#else
	*((am_addr_t*)msg->body.destination.local.data) = dest;
#endif
	msg->body.destination.updated = 0;
	//msg->body.destination.updated) = now_s();
}

static am_addr_t am_comms_get_source(comms_layer_am_t* comms, const comms_msg_t* msg) {
#ifdef ALIGN_CM0
	return *((__packed am_addr_t*)msg->body.source.local.data);
#else
	return *((am_addr_t*)msg->body.source.local.data);
#endif
}
static void am_comms_set_source(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t source) {
	memset(&(msg->body.source.local), 0, sizeof(msg->body.source.local));
#ifdef ALIGN_CM0
	*((__packed am_addr_t*)msg->body.source.local.data) = source;
#else
	*((am_addr_t*)msg->body.source.local.data) = source;
#endif
	msg->body.source.updated = 0;
	//msg->body.source.updated = now_s();
}

am_addr_t comms_am_get_destination(comms_layer_t* comms, const comms_msg_t* msg) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	return amcomms->am_get_destination(amcomms, msg);
}
void comms_am_set_destination(comms_layer_t* comms, comms_msg_t* msg, am_addr_t dest) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	amcomms->am_set_destination(amcomms, msg, dest);
}

am_addr_t comms_am_get_source(comms_layer_t* comms, const comms_msg_t* msg) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	return amcomms->am_get_source(amcomms, msg);
}
void comms_am_set_source(comms_layer_t* comms, comms_msg_t* msg, am_addr_t source) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	amcomms->am_set_source(amcomms, msg, source);
}

comms_error_t comms_am_create(comms_layer_t* layer, am_addr_t address,
	comms_send_f* sendf, comms_plen_f* plenf,
	comms_start_f* startf, comms_stop_f* stopf) {

	comms_layer_iface_t* comms = (comms_layer_iface_t*)layer;
	comms_layer_am_t* amcomms = (comms_layer_am_t*)layer;

	comms->start = startf;
	comms->stop = stopf;

	comms->init_message = &am_comms_init_message;

	amcomms->link_sendf = sendf;
	comms->sendf = &am_comms_send;

	comms->deliverf = &am_comms_deliver;

	comms->start_stop_mutex = comms_mutex_create();
	comms->controller_mutex = comms_mutex_create();
	comms->receiver_mutex = comms_mutex_create();

	comms_initialize_rcvr_management(comms);

	comms->sleep_controller_deferred = NULL;

	comms->get_packet_type = &am_comms_get_packet_type;
	comms->set_packet_type = &am_comms_set_packet_type;

	comms->get_packet_group = &am_comms_get_packet_group;
	comms->set_packet_group = &am_comms_set_packet_group;

	amcomms->link_plenf = plenf;
	comms->get_payload_max_length = &am_comms_get_payload_max_length;
	comms->get_payload_length = &am_comms_get_payload_length;
	comms->set_payload_length = &am_comms_set_payload_length;
	comms->get_payload = &am_comms_get_payload;

	comms->get_retries = &am_comms_get_retries;
	comms->set_retries = &am_comms_set_retries;

	comms->get_retries_used = &am_comms_get_retries_used;
	comms->set_retries_used = &am_comms_set_retries_used;

	comms->get_timeout = &am_comms_get_timeout;
	comms->set_timeout = &am_comms_set_timeout;

	comms->is_ack_required = &am_comms_is_ack_required;
	comms->set_ack_required = &am_comms_set_ack_required;
	comms->ack_received = &am_comms_ack_received;
	comms->set_ack_received = &am_comms_set_ack_received;

	comms->set_timestamp = &am_comms_set_timestamp;
	comms->get_timestamp = &am_comms_get_timestamp;
	comms->timestamp_valid = &am_comms_timestamp_valid;

	comms->set_event_time = &am_comms_set_event_time;
	comms->get_event_time = &am_comms_get_event_time;
	comms->event_time_valid = &am_comms_event_time_valid;

	comms->get_lqi = &am_comms_get_lqi;
	comms->set_lqi = &am_comms_set_lqi;

	comms->get_rssi = &am_comms_get_rssi;
	comms->set_rssi = &am_comms_set_rssi;

	comms->get_priority = &am_comms_get_priority;
	comms->set_priority = &am_comms_set_priority;

	amcomms->am_get_destination = &am_comms_get_destination;
	amcomms->am_set_destination = &am_comms_set_destination;
	amcomms->am_get_source = &am_comms_get_source;
	amcomms->am_set_source = &am_comms_set_source;

	amcomms->am_address = &am_comms_addr;
	amcomms->am_addr = address;

	amcomms->cache = NULL;

	return COMMS_SUCCESS;
}
