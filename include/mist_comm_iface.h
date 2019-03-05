#ifndef MIST_COMM_IFACE_H_
#define MIST_COMM_IFACE_H_

#include "mist_comm.h"

typedef struct comms_layer_iface comms_layer_iface_t;

typedef void comms_init_message_f(comms_layer_iface_t*, comms_msg_t*);

typedef comms_error_t comms_send_f(comms_layer_iface_t*, comms_msg_t*, comms_send_done_f*, void*);

typedef comms_error_t comms_register_recv_f(comms_layer_iface_t*, comms_receiver_t*, comms_receive_f*, void*, am_id_t);
typedef comms_error_t comms_deregister_recv_f(comms_layer_iface_t*, comms_receiver_t*);

typedef am_id_t comms_get_packet_type_f(comms_layer_iface_t*, comms_msg_t*);
typedef void comms_set_packet_type_f(comms_layer_iface_t*, comms_msg_t*, am_id_t);

typedef uint8_t comms_get_payload_max_length_f(comms_layer_iface_t*);
typedef uint8_t comms_get_payload_length_f(comms_layer_iface_t*, comms_msg_t*);
typedef void comms_set_payload_length_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);
typedef void* comms_get_payload_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef uint8_t comms_get_retries_f(comms_layer_iface_t*, comms_msg_t*);
typedef comms_error_t comms_set_retries_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef uint8_t comms_get_retries_used_f(comms_layer_iface_t*, comms_msg_t*);
typedef comms_error_t comms_set_retries_used_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef uint32_t comms_get_timeout_f(comms_layer_iface_t*, comms_msg_t*);
typedef comms_error_t comms_set_timeout_f(comms_layer_iface_t*, comms_msg_t*, uint32_t);

typedef bool comms_is_ack_requested_f(comms_layer_iface_t*, comms_msg_t*);
typedef comms_error_t comms_set_ack_requested_f(comms_layer_iface_t*, comms_msg_t*, bool);

typedef bool comms_ack_received_f(comms_layer_iface_t*, comms_msg_t*);

typedef comms_error_t comms_set_event_time_f(comms_layer_iface_t*, comms_msg_t*, uint32_t);
typedef uint32_t comms_get_event_time_f(comms_layer_iface_t*, comms_msg_t*);
typedef bool comms_event_time_valid_f(comms_layer_iface_t*, comms_msg_t*);

typedef uint8_t comms_get_lqi_f(comms_layer_iface_t*, comms_msg_t*);
typedef void comms_set_lqi_f(comms_layer_iface_t*, comms_msg_t*, uint8_t);

typedef int8_t comms_get_rssi_f(comms_layer_iface_t*, comms_msg_t*);
typedef void comms_set_rssi_f(comms_layer_iface_t*, comms_msg_t*, int8_t);

// -----------------------------------------------------------------------------

typedef struct comms_layer_iface {
	comms_layer_t layer; // Type info

	comms_init_message_f* init_message;

	comms_send_f* send;
	comms_register_recv_f* register_recv;
	comms_deregister_recv_f* deregister_recv;

	comms_get_packet_type_f* get_packet_type;
	comms_set_packet_type_f* set_packet_type;

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

	comms_is_ack_requested_f* is_ack_requested;
	comms_set_ack_requested_f* set_ack_requested;

	comms_ack_received_f* ack_received;

	comms_set_event_time_f* set_event_time;
	comms_get_event_time_f* get_event_time;
	comms_event_time_valid_f* event_time_valid;

	comms_get_lqi_f* get_lqi;
	comms_set_lqi_f* set_lqi;

	comms_get_rssi_f* get_rssi;
	comms_set_rssi_f* set_rssi;

	// Receivers
	comms_receiver_t* receivers; // Linked list of registered receivers

} comms_layer_iface_t;

#endif//MIST_COMM_IFACE_H_
