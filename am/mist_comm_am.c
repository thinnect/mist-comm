
#include "mist_comm_iface.h"
#include "mist_comm_ref.h"
#include "mist_comm_am.h"

#include <stdbool.h>

am_addr_t comms_am_address(comms_layer_t* comms) {
	comms_layer_am_t* amcomms = (comms_layer_am_t*)comms;
	amcomms->am_address(amcomms);
}

static am_addr_t am_comms_addr(comms_layer_am_t* comms) {
	return comms->am_addr;
}

static void am_comms_init_message(comms_layer_iface_t* comms, comms_msg_t* msg) {
	msg->body.length = 0;
}

static comms_error_t am_comms_send(comms_layer_iface_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	return COMMS_FAIL;
}

static comms_error_t am_comms_register_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void *user, am_id_t amid) {
	// there is a suitable implementation in mist_comm_rcv.c
	return COMMS_FAIL;
}
static comms_error_t am_comms_deregister_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr) {
	// there is a suitable implementation in mist_comm_rcv.c
	return COMMS_FAIL;
}

static am_id_t am_comms_get_packet_type(comms_layer_iface_t* comms, const comms_msg_t* msg) {
	return msg->body.type;
}
static void am_comms_set_packet_type(comms_layer_iface_t* comms, comms_msg_t* msg, am_id_t ptype) {
	msg->body.type = ptype;
}

static uint8_t am_comms_get_payload_max_length(comms_layer_iface_t* comms) {
	return 114;
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


static am_addr_t am_comms_get_destination(comms_layer_am_t* comms, const comms_msg_t* msg) {
	return *((am_addr_t*)msg->body.destination);
}
static void am_comms_set_destination(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t dest) {
	*((am_addr_t*)msg->body.destination) = dest;
}

static am_addr_t am_comms_get_source(comms_layer_am_t* comms, const comms_msg_t* msg) {
	return *((am_addr_t*)msg->body.source);
}
static void am_comms_set_source(comms_layer_am_t* comms, comms_msg_t* msg, am_addr_t source) {
	*((am_addr_t*)msg->body.source) = source;
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

comms_error_t comms_am_create(comms_layer_t* layer, am_addr_t address, comms_send_f* sender) {
	comms_layer_iface_t* comms = (comms_layer_iface_t*)layer;
	comms_layer_am_t* amcomms = (comms_layer_am_t*)layer;

	comms->init_message = &am_comms_init_message;

	if(sender != NULL) {
		comms->send = sender;
	}
	else {
		comms->send = &am_comms_send;
	}
	//comms->register_recv = &am_comms_register_recv;
	//comms->deregister_recv = &am_comms_deregister_recv;
	comms_initialize_rcvr_management(comms);

	comms->get_packet_type = &am_comms_get_packet_type;
	comms->set_packet_type = &am_comms_set_packet_type;

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

	amcomms->am_address = &am_comms_addr;
	amcomms->am_addr = address;

	return COMMS_SUCCESS;
}
