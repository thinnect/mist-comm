#ifndef MIST_COMM_H_
#define MIST_COMM_H_
// Mist Communication API ------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "mist_comm_basics.h"

#include "platform_msg.h"

typedef enum CommsErrors {
	COMMS_UNINITIALIZED = -127,
	COMMS_NOT_SUPPORTED = -126,

	COMMS_NO_ADDR       = -15,
	COMMS_NO_KEY        = -14,
	COMMS_NO_ROUTE      = -13,

	COMMS_ETIMEOUT      = -12,
	COMMS_ENOACK        = -11,
	COMMS_ENOMEM        = -10,
	COMMS_EALREADY      = -9, // Would prefer to use positive COMMS_ALREADY instead
	COMMS_ERESERVE      = -8,
	COMMS_ERETRY        = -7,
	COMMS_EINVAL        = -6,
	COMMS_EBUSY         = -5,
	COMMS_EOFF          = -4,
	COMMS_ECANCEL       = -3,
	COMMS_ESIZE         = -2,
	COMMS_FAIL          = -1,

	COMMS_SUCCESS       = 0,
	COMMS_ALREADY       = 1
} comms_error_t;

typedef enum CommsStatus {
	COMMS_STOPPING = -2,
	COMMS_STARTING = -1,
	COMMS_STOPPED  =  0,
	COMMS_STARTED  =  1
} comms_status_t;

typedef struct comms_layer {
	uint8_t type;
	// Everything else will embed at least this structure
} comms_layer_t;

typedef void comms_status_change_f(comms_layer_t* comms, comms_status_t status, void* user);

comms_error_t comms_start(comms_layer_t* comms, comms_status_change_f* start_done, void* user);

comms_error_t comms_stop(comms_layer_t* comms, comms_status_change_f* stop_done, void* user);

comms_status_t comms_status(comms_layer_t* comms);

typedef struct comms_msg comms_msg_t;

// Callback definitions --------------------------------------------------------

// Signalled when message transmission is completed to return the message object
// to the client. A function of this type must be passed with every call to the
// comms_send function and the passed function will be called in the future if
// the comms_send returns a successful result (message is accepted).
typedef void comms_send_done_f(comms_layer_t* comms, comms_msg_t* msg, comms_error_t result, void* user);

// Signalled when a message is received. Functions of this type must first be
// registered with a communications layer with comms_register_recv.
typedef void comms_receive_f(comms_layer_t* comms, const comms_msg_t* msg, void* user);

// -----------------------------------------------------------------------------

// Initialize a message structure for use on the specified communications
// layer. Must be called before any packet functions can be used.
void comms_init_message(comms_layer_t* comms, comms_msg_t* msg);

// Send a message through the specified communications layer.
// The callback function specified with the send done function argument sdf will
// be called some time in the future if comms_send returns SUCCESS. The sdf
// argument is mandatory, the user argument may be NULL.
comms_error_t comms_send(comms_layer_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user);

// Receiver registration ------------------------------------------

typedef struct comms_receiver comms_receiver_t;

// Receiver must pass an unused comms_receiver_t object, guaranteeing that the
// memory remains allocated and is not used elsewhere until deregister is called.
// One receiver object may not be used in multiple roles at the same time, but
// the same receive function can be registered several times with different
// conditions and/or with a different user argument.
comms_error_t comms_register_recv(comms_layer_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void *user, am_id_t amid);
// ???
// last param should be some form of generic args, but this is C ... are we ok
// to proceed with AM ID or should we expand and rename? 16 bit port?
// and what other filtering is needed for receives ... if any?
// ???

// Remove an already registered receiver.
comms_error_t comms_deregister_recv(comms_layer_t* comms, comms_receiver_t* rcvr);

// Snoopers don't look at the type (amid)
comms_error_t comms_register_snooper(comms_layer_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void *user);

// Remove an already registered snooper.
comms_error_t comms_deregister_snooper(comms_layer_t* comms, comms_receiver_t* rcvr);

// -----------------------------------------------------------------------------

// Packet type -----------------------------------------------------------------
am_id_t comms_get_packet_type(comms_layer_t* comms, const comms_msg_t* msg);
void comms_set_packet_type(comms_layer_t* comms, comms_msg_t* msg, am_id_t ptype);
// -----------------------------------------------------------------------------

// Message payload manipulation functions --------------------------------------
uint8_t comms_get_payload_max_length(comms_layer_t* comms);

uint8_t comms_get_payload_length(comms_layer_t* comms, const comms_msg_t* msg);
void comms_set_payload_length(comms_layer_t* comms, comms_msg_t* msg, uint8_t length);

void* comms_get_payload(comms_layer_t* comms, const comms_msg_t* msg, uint8_t length);
// ???
// Do we want support for larger payloads - uint16_t length?
// ???
// -----------------------------------------------------------------------------

// PacketLink & Acknowledgements
uint8_t comms_get_retries(comms_layer_t* comms, const comms_msg_t* msg);
comms_error_t comms_set_retries(comms_layer_t* comms, comms_msg_t* msg, uint8_t count);

uint8_t comms_get_retries_used(comms_layer_t* comms, const comms_msg_t* msg);
comms_error_t comms_set_retries_used(comms_layer_t* comms, comms_msg_t* msg, uint8_t count);

uint32_t comms_get_timeout(comms_layer_t* comms, const comms_msg_t* msg);
comms_error_t comms_set_timeout(comms_layer_t* comms, comms_msg_t* msg, uint32_t timeout);

bool comms_is_ack_required(comms_layer_t* comms, const comms_msg_t* msg);
comms_error_t comms_set_ack_required(comms_layer_t* comms, comms_msg_t* msg, bool required);

// Check delivery for both PacketLink and simple Ack use cases
bool comms_ack_received(comms_layer_t* comms, const comms_msg_t* msg);

void _comms_set_ack_received(comms_layer_t* comms, comms_msg_t* msg);
// -----------------------------------------------------------------------------

// Message timestamping
// Timestamps are microseconds local clock. Indicate either message reception or
// transmission time.

// Check that the timestamp is valid with comms_timestamp_valid.
uint32_t comms_get_timestamp(comms_layer_t* comms, const comms_msg_t* msg);

// return TRUE if message has valid timestamp
bool comms_timestamp_valid(comms_layer_t* comms, const comms_msg_t* msg);

// Set timestamp (low level - on receive and on send)
comms_error_t comms_set_timestamp(comms_layer_t* comms, comms_msg_t* msg, uint32_t timestamp);
// -----------------------------------------------------------------------------

// TimeSync messaging
// Event time is microseconds local clock

// Set event time
comms_error_t comms_set_event_time(comms_layer_t* comms, comms_msg_t* msg, uint32_t evt);

// Check that event time is valid with comms_event_time_valid.
uint32_t comms_get_event_time(comms_layer_t* comms, const comms_msg_t* msg);

// return TRUE if message has valid event time
bool comms_event_time_valid(comms_layer_t* comms, const comms_msg_t* msg);
// -----------------------------------------------------------------------------

// Message Quality
uint8_t comms_get_lqi(comms_layer_t* comms, const comms_msg_t* msg);
void _comms_set_lqi(comms_layer_t* comms, comms_msg_t* msg, uint8_t lqi);
// ??? standardize on 0-100 instead of 0-255, because for example CC1101 LQI goes from 0-127 and is inverse to Atmel RFX.

// Value in dBm
int8_t comms_get_rssi(comms_layer_t* comms, const comms_msg_t* msg);
void _comms_set_rssi(comms_layer_t* comms, comms_msg_t* msg, int8_t rssi);
// -----------------------------------------------------------------------------

// Other functions -------------------------------------------------------------
// Deliver a message to all registered receivers
// TODO the return value does not make sense any more
comms_msg_t* comms_deliver(comms_layer_t* comms, comms_msg_t* msg);
// -----------------------------------------------------------------------------

#include "mist_comm_private.h"

#endif//MIST_COMM_H_
