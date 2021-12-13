/**
 * MistComm bridge implementation.
 * Bridge messages between 2 mist-comm communication interfaces.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 */

#include <inttypes.h>

#include "cmsis_os2.h"
#include "mist_comm.h"
#include "mist_comm_am.h"
#include "mist_comm_bridge.h"

#include "loglevels.h"
#define __MODUUL__ "bridge"
#define __LOG_LEVEL__ (LOG_LEVEL_bridge & BASE_LOG_LEVEL)
#include "log.h"

#define MIST_COMM_BRIDGE_FLAG_SEND_DONE (1 << 0)
#define MIST_COMM_BRIDGE_FLAG_TERMINATE (1 << 1)

static void bridge_thread(void * param);
static void bridge_send_done(comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user);
static void bridge_am_snoop(comms_layer_t* comms, const comms_msg_t* msg, void* user);

comms_error_t comms_bridge_init (comms_bridge_t * bridge,
                                 comms_layer_t * a,
                                 comms_layer_t * b)
{
	bridge->t1.rxqueue = osMessageQueueNew(10, sizeof(comms_msg_t), NULL);
	bridge->t2.rxqueue = osMessageQueueNew(10, sizeof(comms_msg_t), NULL);
	bridge->t1.txqueue = bridge->t2.rxqueue;
	bridge->t2.txqueue = bridge->t1.rxqueue;

	const osThreadAttr_t bridge_thread_1_attr = { .name = "bt1" };
	bridge->t1.thread = osThreadNew(bridge_thread, &(bridge->t1), &bridge_thread_1_attr);
	debug1("bt1 %p", bridge->t1.thread);

	const osThreadAttr_t bridge_thread_2_attr = { .name = "bt2" };
	bridge->t2.thread = osThreadNew(bridge_thread, &(bridge->t2), &bridge_thread_2_attr);
	debug1("bt2 %p", bridge->t2.thread);

	bridge->t1.layer = a;
	bridge->t2.layer = b;

	// set up snoopers
	comms_register_snooper(bridge->t1.layer, &(bridge->t1.receiver), &bridge_am_snoop, &(bridge->t1));
	comms_register_snooper(bridge->t2.layer, &(bridge->t2.receiver), &bridge_am_snoop, &(bridge->t2));

	return COMMS_SUCCESS;
}

void comms_bridge_deinit (comms_bridge_t * bridge)
{
	// remove snoopers
	comms_deregister_snooper(bridge->t1.layer, &(bridge->t1.receiver));
	comms_deregister_snooper(bridge->t2.layer, &(bridge->t2.receiver));

	// stop threads
	osThreadFlagsSet(bridge->t1.thread, MIST_COMM_BRIDGE_FLAG_TERMINATE);
	osThreadFlagsSet(bridge->t2.thread, MIST_COMM_BRIDGE_FLAG_TERMINATE);

	// wait for threads to exit
	while (osThreadTerminated != osThreadGetState(bridge->t1.thread))
	{
		osDelay(1);
	}
	while (osThreadTerminated != osThreadGetState(bridge->t2.thread))
	{
		osDelay(1);
	}

	// empty the queues
	while (osOK == osMessageQueueGet(bridge->t1.rxqueue, &(bridge->t1.msg), NULL, 0))
	{
		// it has been removed, nothing else to do with it now, memory is handled by queue
	}
	while (osOK == osMessageQueueGet(bridge->t2.rxqueue, &(bridge->t2.msg), NULL, 0))
	{
		// it has been removed, nothing else to do with it now, memory is handled by queue
	}

	// delete the queues
	osMessageQueueDelete(bridge->t1.rxqueue);
	osMessageQueueDelete(bridge->t2.rxqueue);
}

static void bridge_send_done (comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user)
{
	comms_bridge_thread_t * bt = (comms_bridge_thread_t*)user;
	logger(result == COMMS_SUCCESS ? LOG_DEBUG3: LOG_WARN1, "%p snt %d", msg, result);
	osThreadFlagsSet(bt->thread, MIST_COMM_BRIDGE_FLAG_SEND_DONE);
}

static void bridge_am_snoop (comms_layer_t* comms, const comms_msg_t* msg, void* user)
{
	comms_bridge_thread_t * bt = (comms_bridge_thread_t*)user;

	#if (__LOG_LEVEL__ & LOG_DEBUG2)
	uint8_t plen = comms_get_payload_length(comms, msg);
	uint8_t* payload = comms_get_payload(comms, msg, plen);
	debugb2("rcv %d", payload, plen, (unsigned int)plen);
	#endif//debug

	if(osMessageQueuePut(bt->rxqueue, msg, 0, 0) != osOK)
	{
		warn1("drop %p", user);
	}
}

static void bridge_thread (void * param)
{
	comms_bridge_thread_t * bt = (comms_bridge_thread_t*)param;

	for(;;)
	{
		uint32_t flags = 0;
		if (osOK == osMessageQueueGet(bt->txqueue, &(bt->msg), NULL, 5000))
		{
			#if (__LOG_LEVEL__ & LOG_DEBUG1)
			uint8_t plen = comms_get_payload_length(bt->layer, &(bt->msg));
			uint8_t* payload = comms_get_payload(bt->layer, &(bt->msg), plen);
			//debugb1("snd %d", payload, plen, (unsigned int)plen);

			debugb1("{%02X}%04"PRIX16"->%04"PRIX16"[%02X]",
				payload, plen,
				DEFAULT_PAN_ID,
				comms_am_get_source(bt->layer, &(bt->msg)),
				comms_am_get_destination(bt->layer, &(bt->msg)),
				comms_get_packet_type(bt->layer, &(bt->msg)));
			#endif//debug

			if(COMMS_SUCCESS == comms_send(bt->layer, &(bt->msg), bridge_send_done, bt))
			{
				debug4("%p snd", &(bt->msg));
				flags = osThreadFlagsWait(MIST_COMM_BRIDGE_FLAG_SEND_DONE, osFlagsWaitAny, osWaitForever);
			}
			else
			{
				warn1("snd");
			}
		}

		// Check for termination, may have been(?) set already in previous flag wait, so keep existing flags
		flags |= osThreadFlagsWait(MIST_COMM_BRIDGE_FLAG_TERMINATE, osFlagsWaitAny, 0);
		if(flags & MIST_COMM_BRIDGE_FLAG_TERMINATE)
		{
			osThreadExit();
		}
	}
}
