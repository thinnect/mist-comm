
#include "mist_comm.h"
#include "mist_comm_iface.h"
#include "mist_comm_am.h"

#include <stdbool.h>

static void am_comms_init_message(comms_layer_iface_t* comms, comms_msg_t* msg) {
	msg->body.length = 0;
}

static comms_error_t am_comms_send(comms_layer_iface_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	return COMMS_FAIL;
}

static comms_error_t am_comms_register_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void *user, am_id_t amid) {
	return COMMS_FAIL;
}
static comms_error_t am_comms_deregister_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr) {
	return COMMS_FAIL;
}

static uint8_t am_comms_get_payload_max_length(comms_layer_iface_t* comms) {
	return 114;
}
static uint8_t am_comms_get_payload_length(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return msg->body.length;
}
static void am_comms_set_payload_length(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t length) {
	msg->body.length = length;
}
static void* am_comms_get_payload(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t length) {
	if(length < sizeof(msg->body.payload)) {
		return (void*)(msg->body.payload);
	}
	return NULL;
}

static uint8_t am_comms_get_retries(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return 0;
}
static comms_error_t am_comms_set_retries(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t count) {
	return COMMS_EINVAL;
}

static uint8_t am_comms_get_retries_used(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return 0;
}
static comms_error_t am_comms_set_retries_used(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t count) {
	return COMMS_EINVAL;
}

static uint32_t am_comms_get_timeout(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return 0;
}
static comms_error_t am_comms_set_timeout(comms_layer_iface_t* comms, comms_msg_t* msg, uint32_t timeout) {
	return COMMS_EINVAL;
}

static bool am_comms_is_ack_required(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return false;
}
static comms_error_t am_comms_set_ack_required(comms_layer_iface_t* comms, comms_msg_t* msg, bool required) {
	return COMMS_EINVAL;
}
static bool am_comms_ack_received(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return false;
}

static comms_error_t am_comms_set_event_time(comms_layer_iface_t* comms, comms_msg_t* msg, uint32_t evt) {
	return COMMS_EINVAL;
}
static uint32_t am_comms_get_event_time(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return 0;
}
static bool am_comms_event_time_valid(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return false;
}

static uint8_t am_comms_get_lqi(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return 0;
}
static void am_comms_set_lqi(comms_layer_iface_t* comms, comms_msg_t* msg, uint8_t lqi) {
}

static int8_t am_comms_get_rssi(comms_layer_iface_t* comms, comms_msg_t* msg) {
	return -128;
}
static void am_comms_set_rssi(comms_layer_iface_t* comms, comms_msg_t* msg, int8_t rssi) {
}


static am_addr_t am_comms_get_destination(comms_layer_am_t* comms, comms_msg_t* msg) {
	return *((am_addr_t*)msg->body.destination);
}
static void am_comms_set_destination(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t dest) {
	*((am_addr_t*)msg->body.destination) = dest;
}

static am_addr_t am_comms_get_source(comms_layer_am_t* comms, comms_msg_t* msg) {
	return *((am_addr_t*)msg->body.source);
}
static void am_comms_set_source(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t source) {
	*((am_addr_t*)msg->body.source) = source;
}

am_addr_t comms_am_get_destination(comms_layer_t* comms, comms_msg_t* msg) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	return amcomms->am_get_destination(amcomms, msg);
}
void comms_am_set_destination(comms_layer_t* comms, comms_msg_t* msg, am_addr_t dest) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	amcomms->am_set_destination(amcomms, msg, dest);
}

am_addr_t comms_am_get_source(comms_layer_t* comms, comms_msg_t* msg) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	return amcomms->am_get_source(amcomms, msg);
}
void comms_am_set_source(comms_layer_t* comms, comms_msg_t* msg, am_addr_t source) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	amcomms->am_set_source(amcomms, msg, source);
}

comms_error_t comms_am_create(comms_layer_t* layer, comms_send_f* sender) {
	comms_layer_iface_t* comms = (comms_layer_iface_t*)layer;
	comms_layer_am_t* amcomms = (comms_layer_am_t*)layer;

	comms->init_message = &am_comms_init_message;

	if(sender != NULL) {
		comms->send = sender;
	}
	else {
		comms->send = &am_comms_send;
	}
	comms->register_recv = &am_comms_register_recv;
	comms->deregister_recv = &am_comms_deregister_recv;

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

	comms->set_event_time = &am_comms_set_event_time;
	comms->get_event_time = &am_comms_get_event_time;
	comms->event_time_valid = &am_comms_event_time_valid;

	comms->get_lqi = &am_comms_get_lqi;
	comms->set_lqi = &am_comms_set_lqi;

	comms->get_rssi = &am_comms_get_rssi;
	comms->set_rssi = &am_comms_set_rssi;

	amcomms->am_get_destination = &am_comms_get_destination;
	amcomms->am_set_destination = &am_comms_set_destination;
	amcomms->am_get_source = &am_comms_get_source;
	amcomms->am_set_source = &am_comms_set_source;

	return COMMS_SUCCESS;
}
