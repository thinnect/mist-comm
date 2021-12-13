/**
* MistComm SerialBasicMessage layer implementation.
*
* Copyright Thinnect Inc. 2020
* @author Raido Pahtma
* @license MIT
*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "cmsis_os2.h"

#include "mist_comm.h"
#include "serial_protocol.h"
#include "serial_basicmessage.h"

#include "loglevels.h"
#define __MODUUL__ "sbm"
#define __LOG_LEVEL__ (LOG_LEVEL_serial_basicmessage & BASE_LOG_LEVEL)
#include "log.h"

static comms_error_t serial_basicmessage_send (comms_layer_iface_t * iface,
                                               comms_msg_t * msg,
                                               comms_send_done_f * send_done, void * user);
static uint8_t serial_basicmessage_max_length (comms_layer_iface_t * iface);

static bool serial_bm_receive(uint8_t dspch, const uint8_t data[], uint8_t length, void* user);
static void serial_bm_senddone(uint8_t dspch, const uint8_t data[], uint8_t length, bool acked, void* user);
static void serial_bm_timer_cb(void * argument);

comms_layer_t * serial_basicmessage_init (serial_basicmessage_t * sbm, serial_protocol_t * spr,
                                          uint8_t dispatch, am_id_t amid)
{
	if ( ! comms_verify_api())
	{
		return NULL;
	}

	sbm->mutex = osMutexNew(NULL);
	sbm->timer = osTimerNew(&serial_bm_timer_cb, osTimerOnce, sbm, NULL);

	// Initialize send queueing system
	sbm->send_busy = false;
	sbm->sending = NULL;
	sbm->send_queue = NULL;
	sbm->free_queue = &(sbm->queue_memory[0]);
	sbm->free_queue->next = NULL;
	for (uint8_t i=1; i<sizeof(sbm->queue_memory)/sizeof(sbm_queue_element_t); i++)
	{
		sbm->queue_memory[i].next = sbm->free_queue;
		sbm->free_queue = &(sbm->queue_memory[i]);
	}
	sbm->amid = amid;

	// Set up dispatcher
	sbm->protocol = spr;
	serial_protocol_add_dispatcher(sbm->protocol, dispatch,
	                               &(sbm->dispatcher),
	                               &serial_bm_receive, &serial_bm_senddone,
	                               sbm);

	// Set up the mist-comm layer
	// TODO start-stop handlers
	comms_am_create((comms_layer_t *)sbm, 0,
	                &serial_basicmessage_send, &serial_basicmessage_max_length,
	                NULL, NULL);

	// serial_basicmessage_t is a semi-valid comms_layer_t
	return (comms_layer_t *)sbm;
}

bool serial_basicmessage_deinit (serial_basicmessage_t* sbm)
{
	while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));

	if ((1 == osTimerIsRunning(sbm->timer))
	  ||(NULL != sbm->sending))
	{
		osMutexRelease(sbm->mutex);
		return false;
	}

	serial_protocol_remove_dispatcher(sbm->protocol, &(sbm->dispatcher));
	osTimerDelete(sbm->timer);

	osMutexRelease(sbm->mutex);
	osMutexDelete(sbm->mutex);

	return true;
}

static bool serial_bm_receive(uint8_t dispatch, const uint8_t data[], uint8_t length, void * user)
{
	serial_basicmessage_t * sbm = (serial_basicmessage_t*)user;
	comms_layer_t * lyr = (comms_layer_t*)sbm;
	debugb1("%p data", data, length, sbm);

	while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));

	comms_msg_t msg;
	comms_init_message(lyr, &msg);

	uint8_t* payload = comms_get_payload(lyr, &msg, length);
	if (NULL == payload)
	{
		osMutexRelease(sbm->mutex);
		err1("pl %d", length);
		return false;
	}

	comms_set_packet_type(lyr, &msg, sbm->amid); // Set to specified amid as it needs to pass through deliver
	comms_set_payload_length(lyr, &msg, length);
	memcpy(payload, data, length);

	// comms_set_timestamp(lyr, &msg, timestamp); // TODO Set to now? Make the lowest layer register frame starts ...?

	// Addresses are set to broadcast, as mist-comm may complain about 0 addresses
	comms_am_set_destination(lyr, &msg, AM_BROADCAST_ADDR);
	comms_am_set_source(lyr, &msg, AM_BROADCAST_ADDR);

	debugb1("rx {%02"PRIX8"}",
		payload, comms_get_payload_length(lyr, &msg),
		dispatch);

	comms_deliver(lyr, &msg);

    osMutexRelease(sbm->mutex);

	return true;
}

static void serial_bm_senddone(uint8_t dispatch, const uint8_t data[], uint8_t length, bool acked, void * user)
{
	serial_basicmessage_t * sbm = (serial_basicmessage_t*)user;
	comms_layer_t * lyr = (comms_layer_t*)sbm;

	debugb1("snt(a:%d) %02X", data, length, (int)acked, (unsigned int)dispatch);

	// Set flags on the message that was just sent
	//comms_set_timestamp(lyr, sbm->sending->msg, now());
	comms_set_ack_received(lyr, sbm->sending->msg, acked);

	// Signal send done events
	sbm->sending->send_done(lyr, sbm->sending->msg, COMMS_SUCCESS, sbm->sending->user);
	sbm->sending->msg = NULL;

	// Mark sent and defer to send other pending messages
	while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));
	sbm->send_busy = false;
	osMutexRelease(sbm->mutex);
	osTimerStart(sbm->timer, 1UL);
}

static void serial_bm_timer_cb(void * argument)
{
	serial_basicmessage_t * sbm = (serial_basicmessage_t*)argument;
	comms_layer_t * lyr = (comms_layer_t*)sbm;

	while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));
	if(sbm->send_busy)
	{
		debug1("bsy"); // Must wait for last send to complete
		osMutexRelease(sbm->mutex);
		return;
	}

	if (NULL != sbm->sending)
	{
		// Return the queue element to the free queue
		sbm->sending->next = sbm->free_queue;
		sbm->free_queue = sbm->sending;
		sbm->sending = NULL;
	}

	if (NULL != sbm->send_queue)
	{
		sbm->sending = sbm->send_queue;
		sbm->send_queue = sbm->send_queue->next;
		sbm->send_busy = true;
	}
	osMutexRelease(sbm->mutex);

	if (NULL != sbm->sending)
	{
		uint16_t length = comms_get_payload_length(lyr, sbm->sending->msg);
		void * payload = comms_get_payload(lyr, sbm->sending->msg, length);
		if (NULL != payload)
		{
			if(true == serial_protocol_send(&(sbm->dispatcher), payload, length, false))
			{
				debug1("snd %p %p %p", sbm->sending, sbm->send_queue, sbm->free_queue);
				return;
			}
			else
			{
				err1("busy?");
			}
		}
		else
		{
			err1("payload");
		}

		// Something bad has happened, return message to user with error
		sbm->sending->send_done(lyr, sbm->sending->msg, COMMS_EINVAL, sbm->sending->user);
		sbm->sending->msg = NULL; // Pointer has been returned

		while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));
		sbm->send_busy = false;
		osMutexRelease(sbm->mutex);
		osTimerStart(sbm->timer, 1L); // Try next message
	}
	else
	{
		debug1("idle");
	}
}

static comms_error_t serial_basicmessage_send (comms_layer_iface_t * iface,
                                               comms_msg_t * msg,
                                               comms_send_done_f * send_done, void * user)
{
	comms_error_t result = COMMS_SUCCESS;
	serial_basicmessage_t * sbm = (serial_basicmessage_t*)iface;
	while (osOK != osMutexAcquire(sbm->mutex, osWaitForever));

	debug1("snd %p %p", sbm->send_queue, sbm->free_queue);
	if (NULL != sbm->free_queue)
	{
		sbm_queue_element_t ** qe = &(sbm->send_queue);
		while (NULL != *qe)
		{
			qe = &((*qe)->next);
		}

		*qe = sbm->free_queue;
		sbm->free_queue = sbm->free_queue->next;

		(*qe)->msg = msg;
		(*qe)->send_done = send_done;
		(*qe)->user = user;
		(*qe)->next = NULL;

		debug1("q %p %p", sbm->send_queue, sbm->free_queue);
		osTimerStart(sbm->timer, 1UL);
	}
	else // Queue full
	{
		result = COMMS_EBUSY;
	}

	osMutexRelease(sbm->mutex);

	return result;
}

static uint8_t serial_basicmessage_max_length (comms_layer_iface_t * iface)
{
    return COMMS_MSG_PAYLOAD_SIZE;
}
