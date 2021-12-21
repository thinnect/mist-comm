/**
 * Mist communications interface structure.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_IFACE_H
#define MIST_COMM_IFACE_H

#include "mist_comm.h"

typedef struct comms_layer_iface comms_layer_iface_t;

typedef comms_status_t comms_status_f(comms_layer_iface_t*);
typedef comms_error_t comms_start_f(comms_layer_iface_t*, comms_status_change_f*, void*);
typedef comms_error_t comms_stop_f(comms_layer_iface_t*, comms_status_change_f*, void*);

typedef void comms_init_message_f(comms_layer_iface_t*, comms_msg_t*);

typedef comms_error_t comms_send_f(comms_layer_iface_t*, comms_msg_t*, comms_send_done_f*, void*);
typedef bool comms_deliver_f(comms_layer_iface_t*, comms_msg_t*);

typedef uint8_t comms_plen_f(comms_layer_iface_t*);

typedef comms_error_t comms_register_recv_f(comms_layer_iface_t*, comms_receiver_t*, comms_receive_f*, void*, am_id_t, bool);
typedef comms_error_t comms_deregister_recv_f(comms_layer_iface_t*, comms_receiver_t*);

typedef comms_error_t comms_register_snooper_f(comms_layer_iface_t*, comms_receiver_t*, comms_receive_f*, void*);
typedef comms_error_t comms_deregister_snooper_f(comms_layer_iface_t*, comms_receiver_t*);

typedef am_id_t comms_get_packet_type_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_packet_type_f(comms_layer_iface_t*, comms_msg_t*, am_id_t);

typedef uint16_t comms_get_packet_group_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_packet_group_f(comms_layer_iface_t*, comms_msg_t*, uint16_t);

typedef uint8_t comms_get_payload_max_length_f(comms_layer_iface_t*);
typedef uint8_t comms_get_payload_length_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_payload_length_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);
typedef void* comms_get_payload_f(comms_layer_iface_t*, const comms_msg_t*, uint8_t);

typedef uint8_t comms_get_retries_f(comms_layer_iface_t*, const comms_msg_t*);
typedef comms_error_t comms_set_retries_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef uint8_t comms_get_retries_used_f(comms_layer_iface_t*, const comms_msg_t*);
typedef comms_error_t comms_set_retries_used_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef uint32_t comms_get_timeout_f(comms_layer_iface_t*, const comms_msg_t*);
typedef comms_error_t comms_set_timeout_f(comms_layer_iface_t*, comms_msg_t*, uint32_t);

typedef bool comms_is_ack_required_f(comms_layer_iface_t*, const comms_msg_t*);
typedef comms_error_t comms_set_ack_required_f(comms_layer_iface_t*, comms_msg_t*, bool);

typedef bool comms_ack_received_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_ack_received_f(comms_layer_iface_t*, comms_msg_t*, bool);

typedef uint32_t comms_get_time_micro_f(comms_layer_iface_t*);

typedef comms_error_t comms_set_timestamp_f(comms_layer_iface_t*, comms_msg_t*, uint32_t);
typedef uint32_t comms_get_timestamp_f(comms_layer_iface_t*, const comms_msg_t*);
typedef bool comms_timestamp_valid_f(comms_layer_iface_t*, const comms_msg_t*);

typedef comms_error_t comms_set_event_time_f(comms_layer_iface_t*, comms_msg_t*, uint32_t);
typedef uint32_t comms_get_event_time_f(comms_layer_iface_t*, const comms_msg_t*);
typedef bool comms_event_time_valid_f(comms_layer_iface_t*, const comms_msg_t*);

typedef uint8_t comms_get_lqi_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_lqi_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef int8_t comms_get_rssi_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_rssi_f(comms_layer_iface_t*, comms_msg_t*, int8_t);

typedef uint8_t comms_get_priority_f(comms_layer_iface_t*, const comms_msg_t*);
typedef void comms_set_priority_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);
// -----------------------------------------------------------------------------

struct comms_layer_iface {
	comms_layer_t layer; // Type info

	comms_start_f* start;
	comms_stop_f* stop;
	comms_status_change_f* status_change_user_cb;
	void* status_change_user;

	comms_init_message_f* init_message;

	comms_send_f* sendf;

	comms_deliver_f* deliverf;

	comms_register_recv_f* register_recv;
	comms_deregister_recv_f* deregister_recv;
	comms_register_snooper_f* register_snooper;
	comms_deregister_snooper_f* deregister_snooper;

	comms_get_packet_type_f* get_packet_type;
	comms_set_packet_type_f* set_packet_type;

	comms_get_packet_group_f* get_packet_group;
	comms_set_packet_group_f* set_packet_group;

	comms_get_payload_max_length_f* get_payload_max_length;
	comms_get_payload_length_f* get_payload_length;
	comms_set_payload_length_f* set_payload_length;
	comms_get_payload_f* get_payload;

	comms_get_retries_f* get_retries;
	comms_set_retries_f* set_retries;

	comms_get_retries_used_f* get_retries_used;
	comms_set_retries_used_f* set_retries_used;

	comms_get_timeout_f* get_timeout;
	comms_set_timeout_f* set_timeout;

	comms_is_ack_required_f* is_ack_required;
	comms_set_ack_required_f* set_ack_required;
	comms_ack_received_f* ack_received;
	comms_set_ack_received_f* set_ack_received;

	comms_get_time_micro_f* get_time_micro;

	comms_set_timestamp_f* set_timestamp;
	comms_get_timestamp_f* get_timestamp;
	comms_timestamp_valid_f* timestamp_valid;

	comms_set_event_time_f* set_event_time;
	comms_get_event_time_f* get_event_time;
	comms_event_time_valid_f* event_time_valid;

	comms_get_lqi_f* get_lqi;
	comms_set_lqi_f* set_lqi;

	comms_get_rssi_f* get_rssi;
	comms_set_rssi_f* set_rssi;

	comms_get_priority_f* get_priority;
	comms_set_priority_f* set_priority;

	// Receivers
	comms_receiver_t* receivers; // List of registered receivers

	// Snoopers
	comms_receiver_t* snoopers; // List of registered snoopers

	// Sleep controllers
	comms_sleep_controller_t* sleep_controllers; // List of sleep controllers
	void * sleep_controller_deferred;

	// Standard variables
	comms_status_t status;

	commsMutexId_t controller_mutex;
	commsMutexId_t start_stop_mutex;
	commsMutexId_t receiver_mutex;
};

#endif//MIST_COMM_IFACE_H
