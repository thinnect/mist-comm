/**
 * Mist communications API.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_H
#define MIST_COMM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "eui64.h"

#include "mist_comm_basics.h"
#include "mist_comm_mutex.h"

#include "platform_msg.h"

// -----------------------------------------------------------------------------
#define MIST_COMM_VERSION_MAJOR 0
#define MIST_COMM_VERSION_MINOR 1
#define MIST_COMM_VERSION_PATCH 0
// -----------------------------------------------------------------------------

/**
 * Verify that the comms api layer seems to be correctly configured.
 *
 * Call this before setting up a comms layer and check that it returns true,
 * panic if it does not.
 *
 * This should fail at compile-time for library-api version conflicts.
 *
 * @return true if API and library version matches, config appears correct.
 */
inline bool comms_verify_api (void) __attribute__((always_inline));

// -----------------------------------------------------------------------------
// Result and status values ----------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Communication layer result values.
 * Generally derived from TinyOS error_t, however, all bad things have
 * a negative value and there is a positive ALREADY instead of EALREADY.
 */
typedef enum CommsErrors {
	COMMS_UNINITIALIZED = -127,
	COMMS_NOT_SUPPORTED = -126,

	COMMS_NO_ADDR       = -15,
	COMMS_NO_KEY        = -14,
	COMMS_NO_ROUTE      = -13,

	COMMS_ETIMEOUT      = -12,
	COMMS_ENOACK        = -11,
	COMMS_ENOMEM        = -10,
//	COMMS_EALREADY      = -9, // Use positive COMMS_ALREADY instead!
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

/**
 * Communication layer status values (states).
 */
typedef enum CommsStatus {
	COMMS_STOPPING = -2,
	COMMS_STARTING = -1,
	COMMS_STOPPED  =  0,
	COMMS_STARTED  =  1
} comms_status_t;

// -----------------------------------------------------------------------------
// Layer, address and message structures ---------------------------------------
// -----------------------------------------------------------------------------

/**
 * Base structure for communications layers. Every interface has a type
 * and an EUI-64 address.
 */
typedef struct comms_layer {
	uint8_t type; // TODO type definitions for reflection
	ieee_eui64_t eui;
	// Everything else will embed at least this structure
} comms_layer_t;

/**
 * The comms local address structure. This carries a link-local address which
 * may take different forms, depending on the environment.
 */
typedef struct comms_local_addr {
	uint8_t data[COMMS_MSG_ADDRESSING_SIZE];
} __attribute__((packed)) comms_local_addr_t;

/**
 * The comms address structure, carries mappings from globally unique EUI64
 * addresses to link-local platform/environment specific addresses.
 * It should be assumed that the local address may change at runtime!
 * It is possible that either the eui or local address is not known (all zeros)!
 */
typedef struct comms_address {
	ieee_eui64_t eui;
	comms_local_addr_t local;
	uint32_t updated; // Timestamp, seconds, monotonic
} __attribute__((packed)) comms_address_t;

/**
 * The comms message structure. Should only be manipulated through the APIs.
 */
typedef struct comms_msg comms_msg_t;

// -----------------------------------------------------------------------------
// Callback definitions --------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Signalled when message transmission is completed to return the message object
 * to the client. A function of this type must be passed with every call to the
 * comms_send function and the passed function will be called in the future if
 * the comms_send returns COMMS_SUCCESS (message is accepted).
 *
 * @param comms  The layer that was used to send the message.
 * @param msg    The message that was sent.
 * @param result The result of the transmission.
 * @param user   The user pointer provided to comms_send.
 */
typedef void comms_send_done_f (comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user);

/**
 * Signalled when a message is received. Functions of this type must first be
 * registered with a communications layer with comms_register_recv.
 *
 * @param comms The layer where the message was received.
 * @param msg   The message, only valid while this call is running.
 * @param user  The user pointer given to comms_register_recv.
 */
typedef void comms_receive_f (comms_layer_t * comms, const comms_msg_t * msg, void * user);

/**
 * Signalled when a status change is requested and completed.
 *
 * @param comms  The layer for which the status change took place.
 * @param status The new status (may not be what was actually requested).
 * @param user   The user pointer.
 */
typedef void comms_status_change_f (comms_layer_t * comms, comms_status_t status, void * user);

// -----------------------------------------------------------------------------
// Communications control and status -------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Start a comms layer. Use this or sleep controllers, not both.
 *
 * @param comms      A comms layer to start.
 * @param start_done Status change callback.
 * @param user       Argument passed to the callback.
 * @return           COMMS_SUCCESS if the callback will be called in the future.
 */
comms_error_t comms_start (comms_layer_t * comms, comms_status_change_f * start_done, void * user);

/**
 * Stop a comms layer. Use this or sleep controllers, not both.
 * @param comms A comms layer to stop.
 * @param start_done Status change callback.
 * @param user Argument passed to the callback.
 * @return COMMS_SUCCESS if the callback will be called in the future.
 */
comms_error_t comms_stop (comms_layer_t * comms, comms_status_change_f * stop_done, void * user);

/**
 * Get current status of the comms layer.
 *
 * @param comms The comms layer to query.
 * @return      Current status.
 */
comms_status_t comms_status (comms_layer_t * comms);

// -----------------------------------------------------------------------------
// Message sending -------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Initialize a message structure for use on the specified communications
 * layer. Must be called before any packet functions can be used. A message
 * initialized with one comms layer must not be used with another layer, unless
 * initialized again with that layer!
 *
 * @param comms The comms layer to use for initilization.
 * @param msg   The message to initialize.
 */
void comms_init_message (comms_layer_t * comms, comms_msg_t * msg);

/**
 * Send a message through the specified communications layer.
 *
 * The message must stay allocated and not be modified while the layer is
 * processing it (until send-done is called).
 *
 * @param comms The comms layer to use for sending the message.
 * @param msg   The message to send.
 * @param sdf   Pointer to a send-done function.
 * @param user  Optional user pointer, returned in send-done.
 * @return      COMMS_SUCCESS, if the send-done function will be called some time in the future.
 */
comms_error_t comms_send (comms_layer_t * comms, comms_msg_t * msg, comms_send_done_f * sdf, void * user);

// -----------------------------------------------------------------------------
// Receiver registration -------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Receiver structure for registering a receive callback.
 * Must not go out of scope before it is deregistered!
 */
typedef struct comms_receiver comms_receiver_t;

/**
 * Register to receive messages of a specific type.
 *
 * Must pass an unused comms_receiver_t object, guaranteeing that the
 * memory remains allocated and is not used elsewhere until deregister is called.
 *
 * One receiver object may not be used in multiple roles at the same time
 *
 * The same receive function may be registered several times with different
 * conditions and/or with a different user pointer.
 *
 * Multiple receivers may be registered for the same packet type, the message will
 * be delivered to all receivers.
 *
 * @param comms Pointer to a comms layer.
 * @param rcvr  Receiver structure.
 * @param func  The receive function to call when a packet is received.
 * @param user  User pointer to be passed to the receive function on packet reception.
 * @param amid  The message type to register the receiver for.
 */
comms_error_t comms_register_recv (comms_layer_t * comms,
                                   comms_receiver_t * rcvr,
                                   comms_receive_f * func, void * user,
                                   am_id_t amid);

/**
 * Register to receive messages, requesting the EUI64 addresses to be resolved.
 *
 * Receive callbacks will still be called, even if the EUI64 of the sender is not
 * known, but the comms layer will attempt to resolve the address for subsequent
 * messages.
 *
 * Must pass an unused comms_receiver_t object, guaranteeing that the
 * memory remains allocated and is not used elsewhere until deregister is called.
 *
 * One receiver object may not be used in multiple roles at the same time
 *
 * The same receive function may be registered several times with different
 * conditions and/or with a different user pointer.
 *
 * Multiple receivers may be registered for the same packet type, the message will
 * be delivered to all receivers.
 *
 * @param comms Pointer to a comms layer.
 * @param rcvr  Receiver structure.
 * @param func  The receive function to call when a packet is received.
 * @param user  User pointer to be passed to the receive function on packet reception.
 * @param amid  The message type to register the receiver for.
 * @return      COMMS_SUCCESS if successfully registered.
 */
comms_error_t comms_register_recv_eui (comms_layer_t * comms,
                                       comms_receiver_t * rcvr,
                                       comms_receive_f * func, void * user,
                                       am_id_t amid);

/**
 * Remove an already registered receiver.
 *
 * @param comms Pointer to a comms layer.
 * @param rcvr  Receiver structure to deregister.
 * @return      COMMS_SUCCESS if receiver has been removed and can be recycled.
 */
comms_error_t comms_deregister_recv (comms_layer_t * comms, comms_receiver_t * rcvr);

/**
 * Register to snoop for messages:
 *  All messages, regardless of AMID.
 *  All destinations, if underlying device is in promiscuous mode.
 *
 * Must pass an unused comms_receiver_t object, guaranteeing that the
 * memory remains allocated and is not used elsewhere until deregister is called.
 *
 * One receiver object may not be used in multiple roles at the same time
 *
 * The same receive function may be registered several times with different
 * conditions and/or with a different user pointer.
 *
 * Multiple receivers may be registered for the same packet type, the message will
 * be delivered to all receivers.
 *
 * @param comms Pointer to a comms layer.
 * @param rcvr  Receiver structure.
 * @param func  The receive function to call when a packet is received.
 * @param user  User pointer to be passed to the receive function on packet reception.
 */
comms_error_t comms_register_snooper (comms_layer_t * comms,
                                      comms_receiver_t * rcvr,
                                      comms_receive_f * func, void * user);

/**
 * Remove an already registered snooper.
 *
 * @param comms Pointer to a comms layer.
 * @param rcvr  Receiver structure to deregister.
 * @return      COMMS_SUCCESS if receiver has been removed and can be recycled.
 */
comms_error_t comms_deregister_snooper (comms_layer_t * comms, comms_receiver_t * rcvr);

// -----------------------------------------------------------------------------
// Packet type -----------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the packet type (AM ID - Active Message ID) of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Packet type (AM ID).
 */
am_id_t comms_get_packet_type (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the packet type (AM ID - Active Message ID) of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param ptype The type.
 */
void comms_set_packet_type (comms_layer_t * comms, comms_msg_t * msg, am_id_t ptype);

// -----------------------------------------------------------------------------
// Packet group (PAN ID, AM group, etc) ----------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the packet group (AM group / PAN ID) of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Packet group (PAN ID).
 */
uint16_t comms_get_packet_group (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the packet group (AM group / PAN ID) of the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param group Packet group (PAN ID).
 */
void comms_set_packet_group (comms_layer_t * comms, comms_msg_t * msg, uint16_t group);

// -----------------------------------------------------------------------------
// Addressing ------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the destination of the message.
 *
 * @param comms       Pointer to a comms layer.
 * @param msg         Pointer to a message.
 * @param destination Pointer to a comms_address_t object for copying destination info.
 */
void comms_get_destination (comms_layer_t * comms, const comms_msg_t * msg, comms_address_t * destination);

/**
 * Set the destination of the message.
 *
 * @param comms       Pointer to a comms layer.
 * @param msg         Pointer to a message.
 * @param destination Pointer to an object holding the destination.
 */
void comms_set_destination (comms_layer_t * comms, comms_msg_t * msg, const comms_address_t * destination);

/**
 * Get the source of the message.
 *
 * @param comms  Pointer to a comms layer.
 * @param msg    Pointer to a message.
 * @param source Pointer to a comms_address_t object for copying source info.
 */
void comms_get_source (comms_layer_t * comms, const comms_msg_t * msg, comms_address_t * source);

/**
 * Set the source of the message. Source is also set automatically on send, if not set before,
 * it needs to be set explicitly when for example forwarding messages and the source is not
 * the sending node itself.
 *
 * @param comms  Pointer to a comms layer.
 * @param msg    Pointer to a message.
 * @param source Pointer to an object holding the source.
 */
void comms_set_source (comms_layer_t * comms, comms_msg_t * msg, const comms_address_t * source);

// -----------------------------------------------------------------------------
// Message payload functions ---------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the maximum payload length supported by this communications layer.
 *
 * @param comms Pointer to a comms layer.
 * @return      Maximum supported payload length.
 */
uint8_t comms_get_payload_max_length (comms_layer_t * comms);

/**
 * Get the current payload length of this message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Payload length.
 */
uint8_t comms_get_payload_length (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the payload length for this message. The value must be <= comms_get_payload_max_length.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Payload length.
 */
void comms_set_payload_length (comms_layer_t * comms, comms_msg_t * msg, uint8_t length);

/**
 * Get a pointer to the payload. NULL is returned if the requested size cannot be
 * accomodated by the comms layer. Actual size of the payload must be retrieved
 * with comms_get_payload_length when reading the payload and set
 * with comms_set_payload_length after writing the payload.
 *
 * @param comms  Pointer to a comms layer.
 * @param msg    Pointer to a message.
 * @param length Expected max length of the payload.
 * @return       Pointer to payload or NULL
 */
void * comms_get_payload (comms_layer_t * comms, const comms_msg_t * msg, uint8_t length);

// -----------------------------------------------------------------------------
// PacketLink & Acknowledgements -----------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the number of allowed retries for the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Number of allowed retries.
 */
uint8_t comms_get_retries (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the number of allowed retries for the message. Setting to > 0 will cause
 * an acknowledgement to be requested when sent to unicast. For broadcast the
 * the message will be repeated the specified number of times.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param count Number of allowed retries.
 * @return      COMMS_SUCCESS when retry parameters were valid.
 */
comms_error_t comms_set_retries (comms_layer_t * comms, comms_msg_t * msg, uint8_t count);

/**
 * Get the number of retries used for the message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Number of used retries, when sending the message.
 */
uint8_t comms_get_retries_used(comms_layer_t* comms, const comms_msg_t* msg);

/**
 * Set the number of used retries.
 *
 * For communication layer implementation internal use!
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Number of allowed retries.
 */
comms_error_t comms_set_retries_used(comms_layer_t* comms, comms_msg_t* msg, uint8_t count);

/**
 * Get the configured retry timeout.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Retry timeout (milliseconds).
 */
uint32_t comms_get_timeout(comms_layer_t* comms, const comms_msg_t* msg);

/**
 * Set the retry timeout for sending this message.
 *
 * @param comms   Pointer to a comms layer.
 * @param msg     Pointer to a message.
 * @param timeout Retry timeout (milliseconds).
 * @return        COMMS_SUCCESS if the timeout value was valid.
 */
comms_error_t comms_set_timeout(comms_layer_t* comms, comms_msg_t* msg, uint32_t timeout);

/**
 * Check if an ack has been requested for this message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      true if an ack was / will be requested for the message.
 */
bool comms_is_ack_required(comms_layer_t* comms, const comms_msg_t* msg);

/**
 * Set if the ack is required or not.
 *
 * @param comms    Pointer to a comms layer.
 * @param msg      Pointer to a message.
 * @param required Set to true to request an ack.
 * @return         COMMS_SUCCESS if the requested ack configuration can be supported.
 */
comms_error_t comms_set_ack_required (comms_layer_t * comms, comms_msg_t * msg, bool required);

/**
 * Check if an ack was received for this message.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      true if an acknowledgement was received.
 */
bool comms_ack_received (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the ack field in the message.
 *
 * For communication layer implementation internal use!
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      true if an acknowledgement was received.
 */
void comms_set_ack_received (comms_layer_t * comms, comms_msg_t * msg, bool received);

// -----------------------------------------------------------------------------
// Message timestamping --------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get the current communications layer time. Timestamps of messages are set
 * relative to this counter. The value is in microseconds, however, depending on
 * the underlying implementation, ticks may be many microseconds - for example
 * the clock may actually have millisecond precision, so the returned timestamps
 * will jump in 1000 microsecond steps.
 *
 * @param comms Pointer to a comms layer.
 * @return      Current timestamp (microseconds).
 */
uint32_t comms_get_time_micro (comms_layer_t * comms);

/**
 * Check if the message has a valid timestamp.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      true if the message has been timestamped on RX or TX.
 */
bool comms_timestamp_valid (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Get the message timestamp:
 *  * RX timestamp, if message was received
 *  * TX timestamp, if the message was sent
 *
 * Always check first that the timestamp is valid with comms_timestamp_valid!
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Message RX or TX timestamp.
 */
uint32_t comms_get_timestamp_micro (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set the timestamp field in the message.
 * Setting the timestamp will make future calls to comms_timestamp_valid return true.
 *
 * @param comms     Pointer to a comms layer.
 * @param msg       Pointer to a message.
 * @param timestamp The timestamp.
 * @return          COMMS_SUCCESS unless something went horribly wrong.
 */
comms_error_t comms_set_timestamp_micro (comms_layer_t * comms, comms_msg_t * msg, uint32_t timestamp);

// -----------------------------------------------------------------------------
// TimeSync messaging ----------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Set an event time in respect to the comms_get_time_micro clock.
 *
 * @param comms     Pointer to a comms layer.
 * @param msg       Pointer to a message.
 * @param timestamp The timestamp.
 * @return          COMMS_SUCCESS unless something went horribly wrong.
 */
comms_error_t comms_set_event_time_micro (comms_layer_t * comms, comms_msg_t * msg, uint32_t evt);

/**
 * Get the event time communicated with the message.
 *
 * Always check that event time is valid with comms_event_time_valid!
 *
 * @param comms     Pointer to a comms layer.
 * @param msg       Pointer to a message.
 * @return          Event timestamp relative to comms_get_time_micro.
 */
uint32_t comms_get_event_time_micro (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Check if the message carries a valid event time.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      true if the message includes an event time.
 */
bool comms_event_time_valid (comms_layer_t * comms, const comms_msg_t * msg);

// -----------------------------------------------------------------------------
// Message Quality -------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get message link-quality. 0-255, 255 is best.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Message quality (0-255, 255 is best).
 */
uint8_t comms_get_lqi (comms_layer_t* comms, const comms_msg_t* msg);

/**
 * Set message link-quality.
 *
 * For communication layer implementation internal use!
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param lqi   Message quality (0-255, 255 is best).
 */
void comms_set_lqi (comms_layer_t* comms, comms_msg_t* msg, uint8_t lqi);

/**
 * Get message RSSI - Received Signal Strength Indicator, dBm.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Message RSSI, dBm.
 */
int8_t comms_get_rssi (comms_layer_t* comms, const comms_msg_t* msg);

/**
 * Set message RSSI - Received Signal Strength Indicator, dBm.
 *
 * For communication layer implementation internal use!
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @param rssi  Message RSSI, dBm.
 */
void comms_set_rssi (comms_layer_t* comms, comms_msg_t* msg, int8_t rssi);

// -----------------------------------------------------------------------------
// Message Priority ------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Get message priority, 0 is default.
 *
 * Priority handling mostly depends on the specific of the underlying
 * implementation, but is relative to 0 ... higher value, higher priority.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   Pointer to a message.
 * @return      Message priority.
 */
int8_t comms_get_priority (comms_layer_t * comms, const comms_msg_t * msg);

/**
 * Set message priority, 0 for normal (default).
 *
 * Priority handling mostly depends on the specific of the underlying
 * implementation, but is relative to 0 ... higher value, higher priority.
 *
 * @param comms     Pointer to a comms layer.
 * @param msg       Pointer to a message.
 * @param priority  Message priority.
 */
void comms_set_priority (comms_layer_t * comms, comms_msg_t * msg, int8_t priority);

// -----------------------------------------------------------------------------
// Received message delivery ---------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Deliver a message to all registered receivers.
 *
 * @param comms The later to use.
 * @param msg   The message to deliver.
 * @return      true if the message was delivered to at least one receiver.
*/
bool comms_deliver (comms_layer_t * comms, comms_msg_t * msg);

// -----------------------------------------------------------------------------
// Sleep management ------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Sleep management controller.
 *
 * If a sleep controller is registered for a comms layer, it will entirely take
 * over start-stop management, the comms_start and comms_stop functions should
 * be ignored altogether and only comms_sleep_block and comms_sleep_allow
 * should be used.
 */
typedef struct comms_sleep_controller comms_sleep_controller_t;

/**
 * Register a sleep controller.
 * @param comms      Layer to register controller to.
 * @param ctrl       An unused controller.
 * @param start_done Callback function called when a sleep block starts the comms layer.
 * @param user       User parameter passed with the callback.
 * @return           COMMS_SUCCESS if sleep controller registered.
 */
comms_error_t comms_register_sleep_controller (comms_layer_t * comms, comms_sleep_controller_t * ctrl,
                                               comms_status_change_f * start_done, void * user);

/**
 * De-register a sleep controller.
 *
 * @param comms Layer to remove the controller from.
 * @param ctrl  A registered ccontroller.
 * @return      COMMS_SUCCESS when controller was removed.
 */
comms_error_t comms_deregister_sleep_controller (comms_layer_t * comms, comms_sleep_controller_t * ctrl);

/**
 * Request a sleep block and the layer to be started. May return COMMS_ALREADY if
 * the block takes effect immediately or COMMS_SUCCESS if the layer needs to be
 * started and therefore a state change callback will be called once ready.
 *
 * @param ctrl A registered ccontroller.
 * @return     COMMS_SUCCESS when callback will be called in the future,
 *             COMMS_ALREADY when block immediately successful.
 */
comms_error_t comms_sleep_block (comms_sleep_controller_t * ctrl);

/**
 * Release a sleep block. The block is released immediately but the layer shutdown
 * will only happen if other blocks are not present ... it will also take time.
 * No callback will be fired, it is ok to request another block immediately.
 *
 * @param ctrl A registered controller.
 * @return     COMMS_SUCCESS when block was released,
 *             COMMS_ALREADY when it was not event active.
 */
comms_error_t comms_sleep_allow (comms_sleep_controller_t * ctrl);

/**
 * Query current block status. Use comms_status to get actual status.
 *
 * @param ctrl A registered controller.
 * @return     true if block is active or pending.
 */
bool comms_sleep_blocked (comms_sleep_controller_t * ctrl);

// -----------------------------------------------------------------------------
// Locking, for internal use mostly --------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Create a mutex.
 *
 * @return A mutex ID or NULL for failure.
 */
commsMutexId_t comms_mutex_create (void);

/**
 * Acquire the specified mutex. Blocks until aquired.
 *
 * @param mutex - The mutex to acquire.
 */
void comms_mutex_acquire (commsMutexId_t mutex);

/**
 * Release the specified mutex. Returns "immediately".
 *
 * @param mutex - The mutex to release.
 */
void comms_mutex_release (commsMutexId_t mutex);

// -----------------------------------------------------------------------------
// Include implementation details.
// -----------------------------------------------------------------------------
#include "mist_comm_private.h"

#endif//MIST_COMM_H
