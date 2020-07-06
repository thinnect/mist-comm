/**
* MistComm SerialBasicMessage layer.
*
* Many instances can be created, each needs a serial_protocol.
*
* Copyright Thinnect Inc. 2020
* @author Raido Pahtma
* @license MIT
*/
#ifndef SERIAL_BASICMESSAGE_H_
#define SERIAL_BASICMESSAGE_H_

#include "mist_comm_am.h"
#include "mist_comm.h"
#include "serial_protocol.h"

#include "cmsis_os2.h"

// The serial_basicmessage instance structure
typedef struct serial_basicmessage serial_basicmessage_t;

/**
 * Initialize the SerialbasicMessage layer, providing it memory and access
 * to a lower layer SerialProtocol.
 *
 * @param sbm - The SerialBasicMessage to initialize, a pointer to a persistent
 *              memory structure.
 * @param spr - Pointer to an initialized SerialProtocol layer.
 * @param dispatch - The SerialProtocol dispatch ID to use.
 * @param amid - The AM ID to use for RX messages (for compatibility).
 *
 * @return a MistComm instance that can be used for sending/receiving messages
 *         or NULL for failure.
 */
comms_layer_t* serial_basicmessage_init (serial_basicmessage_t * sbm,
                                         serial_protocol_t * spr,
                                         uint8_t dispatch,
                                         am_id_t amid);

/**
 * Deinitialize the SerialbasicMessage layer. The memory used by the instance
 * may be freed after this.
 *
 * @param sbm - An initialized SerialbasicMessage
 * @return true if successful, false if busy
 */
bool serial_basicmessage_deinit (serial_basicmessage_t * sbm);


// Internal details to allow memory allocation----------------------------------

// send queue structure
typedef struct sbm_queue_element sbm_queue_element_t;
struct sbm_queue_element
{
	comms_msg_t* msg;
	comms_send_done_f *send_done;
	void *user;
	sbm_queue_element_t* next;
};

#ifndef SERIAL_BASICMESSAGE_QUEUE_LENGTH
#define SERIAL_BASICMESSAGE_QUEUE_LENGTH 2
#endif//SERIAL_BASICMESSAGE_QUEUE_LENGTH

struct serial_basicmessage
{
	comms_layer_am_t base;
	serial_dispatcher_t dispatcher;
	serial_protocol_t* protocol;

	osMutexId_t mutex;
	osTimerId_t timer;

	sbm_queue_element_t * sending;
	sbm_queue_element_t * send_queue;
	sbm_queue_element_t * free_queue;
	sbm_queue_element_t queue_memory[SERIAL_BASICMESSAGE_QUEUE_LENGTH];

	am_id_t amid;

	bool send_busy;
};

#endif//SERIAL_BASICMESSAGE_H_
