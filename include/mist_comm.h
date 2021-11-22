#ifndef MIST_COMM_H_
#define MIST_COMM_H_
// Mist Communication API ------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "eui64.h"

#include "mist_comm_basics.h"
#include "mist_comm_mutex.h"

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
	uint8_t type; // TODO type definitions for reflection
	ieee_eui64_t eui;
	// Everything else will embed at least this structure
} comms_layer_t;

/**
 * The comms local address structure. This carries a link-local address which
 * may take different forms.
 */
typedef struct comms_local_addr {
	uint8_t data[COMMS_MSG_ADDRESSING_SIZE];
} __attribute__((packed))comms_local_addr_t;

/**
 * The comms address structure.
 */
typedef struct comms_address {
	ieee_eui64_t eui;
	comms_local_addr_t local;
	uint32_t updated; // Timestamp, seconds
} __attribute__((packed))comms_address_t;

/*
 * The comms message structure. Should only be manipulated through the APIs.
 */
typedef struct comms_msg comms_msg_t;

// Callback definitions --------------------------------------------------------

/**
 * Signalled when message transmission is completed to return the message object
 * to the client. A function of this type must be passed with every call to the
 * comms_send function and the passed function will be called in the future if
 * the comms_send returns COMMS_SUCCESS (message is accepted).
 *
 * @param comms The layer that was used to send the message.
 * @param msg The message that was sent.
 * @param result The result of the transmission.
 * @param user The user pointer provided to comms_send.
 */
typedef void comms_send_done_f(comms_layer_t* comms, comms_msg_t* msg, comms_error_t result, void* user);

/**
 * Signalled when a message is received. Functions of this type must first be
 * registered with a communications layer with comms_register_recv.
 *
 * @param comms The layer where the message was received.
 * @param msg The message, only valid while this call is running.
 * @param user The user pointer given to comms_register_recv.
 */
typedef void comms_receive_f(comms_layer_t* comms, const comms_msg_t* msg, void* user);

/**
 * Signalled when a status change is requested and completed.
 *
 * @param comms The layer for which the status change took place.
 * @param status The new status (may not be what was actually requested).
 * @param user The user pointer.
 */
typedef void comms_status_change_f(comms_layer_t* comms, comms_status_t status, void* user);

// -----------------------------------------------------------------------------

/**
 * Start a comms layer. Use this or sleep controllers, not both.
 * @param comms A comms layer to start.
 * @param start_done Status change callback.
 * @param user Argument passed to the callback.
 * @return COMMS_SUCCESS if the callback will be called in the future.
 */
comms_error_t comms_start(comms_layer_t* comms, comms_status_change_f* start_done, void* user);

/**
 * Stop a comms layer. Use this or sleep controllers, not both.
 * @param comms A comms layer to stop.
 * @param start_done Status change callback.
 * @param user Argument passed to the callback.
 * @return COMMS_SUCCESS if the callback will be called in the future.
 */
comms_error_t comms_stop(comms_layer_t* comms, comms_status_change_f* stop_done, void* user);

/**
 * Get current status of the comms layer.
 * @return Current status.
 */
comms_status_t comms_status(comms_layer_t* comms);

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

/**
 * Register to receive messages.
 *
 * Must pass an unused comms_receiver_t object, guaranteeing that the
 * memory remains allocated and is not used elsewhere until deregister is called.
 * One receiver object may not be used in multiple roles at the same time, but
 * the same receive function can be registered several times with different
 * conditions and/or with a different user argument.
 */
comms_error_t comms_register_recv(comms_layer_t* comms,
                                  comms_receiver_t* rcvr,
                                  comms_receive_f* func, void *user,
                                  am_id_t amid);

/**
 * Register to receive messages, require the EUI64 addresses to be resolved.
 *
 * Must pass an unused comms_receiver_t object, guaranteeing that the
 * memory remains allocated and is not used elsewhere until deregister is called.
 * One receiver object may not be used in multiple roles at the same time, but
 * the same receive function can be registered several times with different
 * conditions and/or with a different user argument.
 */
comms_error_t comms_register_recv_eui(comms_layer_t* comms,
                                      comms_receiver_t* rcvr,
                                      comms_receive_f* func, void *user,
                                      am_id_t amid);


// ??? Open questions regarding receivers and their registration:
// AMID param should be some form of generic args, but this is C ... are we ok
// to only have the 8-bit AM ID or should we expand and rename? 16 bit port?
// Are any other filtering options needed for receiveres?
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

// Packet group (PAN ID, AM group, etc) ----------------------------------------
uint16_t comms_get_packet_group(comms_layer_t* comms, const comms_msg_t* msg);
void comms_set_packet_group(comms_layer_t* comms, comms_msg_t* msg, uint16_t group);
// -----------------------------------------------------------------------------

// Addressing ------------------------------------------------------------------
void comms_get_destination(comms_layer_t* comms, const comms_msg_t* msg, comms_address_t* destination);
void comms_set_destination(comms_layer_t* comms, comms_msg_t* msg, const comms_address_t* destination);

void comms_get_source(comms_layer_t* comms, const comms_msg_t* msg, comms_address_t* source);
void comms_set_source(comms_layer_t* comms, comms_msg_t* msg, const comms_address_t* source);
// -----------------------------------------------------------------------------

// Message payload manipulation functions --------------------------------------
uint8_t comms_get_payload_max_length(comms_layer_t* comms);

uint8_t comms_get_payload_length(comms_layer_t* comms, const comms_msg_t* msg);
void comms_set_payload_length(comms_layer_t* comms, comms_msg_t* msg, uint8_t length);

void* comms_get_payload(comms_layer_t* comms, const comms_msg_t* msg, uint8_t length);
// ??? Open questions regarding payload:
// Do we want support larger payloads - uint16_t length?
// ???
// -----------------------------------------------------------------------------

// PacketLink & Acknowledgements -----------------------------------------------
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

// Message timestamping --------------------------------------------------------
// Timestamps are microseconds local clock. Indicate either message reception or
// transmission time.

// Check that the timestamp is valid with comms_timestamp_valid.
uint32_t comms_get_timestamp(comms_layer_t* comms, const comms_msg_t* msg);

// return TRUE if message has valid timestamp
bool comms_timestamp_valid(comms_layer_t* comms, const comms_msg_t* msg);

// Set timestamp (low level - on receive and on send)
comms_error_t comms_set_timestamp(comms_layer_t* comms, comms_msg_t* msg, uint32_t timestamp);
// -----------------------------------------------------------------------------

// TimeSync messaging ----------------------------------------------------------
// Event time is microseconds local clock

// Set event time
comms_error_t comms_set_event_time(comms_layer_t* comms, comms_msg_t* msg, uint32_t evt);

// Check that event time is valid with comms_event_time_valid.
uint32_t comms_get_event_time(comms_layer_t* comms, const comms_msg_t* msg);

// return TRUE if message has valid event time
bool comms_event_time_valid(comms_layer_t* comms, const comms_msg_t* msg);
// -----------------------------------------------------------------------------

// Message Quality -------------------------------------------------------------
uint8_t comms_get_lqi(comms_layer_t* comms, const comms_msg_t* msg);
void _comms_set_lqi(comms_layer_t* comms, comms_msg_t* msg, uint8_t lqi);
// ??? standardize on 0-100 instead of 0-255, because for example CC1101 LQI goes from 0-127 and is inverse to Atmel RFX.

// Value in dBm
int8_t comms_get_rssi(comms_layer_t* comms, const comms_msg_t* msg);
void _comms_set_rssi(comms_layer_t* comms, comms_msg_t* msg, int8_t rssi);

int8_t comms_get_priority(comms_layer_t* comms, const comms_msg_t* msg);
void comms_set_priority(comms_layer_t* comms, comms_msg_t* msg, int8_t priority);
// -----------------------------------------------------------------------------

// Received message delivery ---------------------------------------------------
/**
 * Deliver a message to all registered receivers.
 *
 * @param comms The later to use.
 * @param msg The message to deliver.
 * @return true if the message was delivered to at least one receiver
*/
bool comms_deliver(comms_layer_t* comms, comms_msg_t* msg);
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Sleep management controller
// If a sleep controller is registered for a comms layer, it will entirely take
// over start-stop management, the comms_start and comms_stop functions should
// be ignored altogether and only comms_sleep_block and comms_sleep_allow
// should be used.
// -----------------------------------------------------------------------------
typedef struct comms_sleep_controller comms_sleep_controller_t;

/**
 * Register a sleep controller.
 * @param comms Layer to register controller to.
 * @param ctrl An unused controller.
 * @param start_done Callback function called when a sleep block starts the comms layer.
 * @param user User parameter passed with the callback.
 */
comms_error_t comms_register_sleep_controller(comms_layer_t * comms, comms_sleep_controller_t * ctrl,
                                              comms_status_change_f * start_done, void * user);

/**
 * De-register a sleep controller.
 * @param comms Layer to remove the controller from.
 * @param ctrl A registered ccontroller.
 * @return COMMS_SUCCESS when controller was removed.
 */
comms_error_t comms_deregister_sleep_controller(comms_layer_t * comms, comms_sleep_controller_t * ctrl);

// Block may be asynchronous, wakeup may take time, or EALREADY may be returned

/**
 * Request a sleep block and the layer to be started. May return COMMS_ALREADY if
 * the block takes effect immediately or COMMS_SUCCESS if the layer needs to be
 * started and therefore a state change callback will be called once ready.
 *
 * @param ctrl A registered ccontroller.
 * @return COMMS_SUCCESS when callback will be called in the future, COMMS_ALREADY if block immediately successful.
 */
comms_error_t comms_sleep_block(comms_sleep_controller_t * ctrl);

/**
 * Release a sleep block. The block is released immediately but the layer shutdown
 * will only happen if other blocks are not present ... it will also take time.
 * No callback will be fired, it is ok to request another block immediately.
 *
 * @param ctrl A registered ccontroller.
 * @return COMMS_SUCCESS when block was released, COMMS_ALREADY if it was not event active.
 */
comms_error_t comms_sleep_allow(comms_sleep_controller_t * ctrl);

/**
 * Query current block status. Use comms_status to get actual status.
 *
 * @param ctrl A registered ccontroller.
 * @return true if block is active or pending.
 */
bool comms_sleep_blocked(comms_sleep_controller_t * ctrl);

// -----------------------------------------------------------------------------
// Locking, for internal use mostly --------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Create a mutex.
 * @return NULL for failure.
 */
commsMutexId_t comms_mutex_create(void);

/**
 * Acquire the specified mutex. Blocks until aquired.
 * @param mutex - The mutex to acquire.
 */
void comms_mutex_acquire(commsMutexId_t mutex);

/**
 * Release the specified mutex. Returns "immediately".
 * @param mutex - The mutex to release.
 */
void comms_mutex_release(commsMutexId_t mutex);

// -----------------------------------------------------------------------------
// Include implementation details.
// -----------------------------------------------------------------------------
#include "mist_comm_private.h"

#endif//MIST_COMM_H_
