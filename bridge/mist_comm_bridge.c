/**
 * MistComm bridge implementation.
 * Bridge messages between 2 mist-comm communication interfaces.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 * @author Raido Pahtma
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

static void bridge_thread(void * param);
static void bridge_send_done(comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user);
static void bridge_am_snoop(comms_layer_t* comms, const comms_msg_t* msg, void* user);

comms_error_t comms_bridge_init (comms_bridge_t * bridge,
                                 comms_layer_t * a,
                                 comms_layer_t * b)
{
	bridge->t1.rxqueue = osMessageQueueNew(10, sizeof(comms_msg_t), NULL);
	bridge->t1.txqueue = osMessageQueueNew(10, sizeof(comms_msg_t), NULL);
	bridge->t2.rxqueue = bridge->t1.txqueue;
	bridge->t2.txqueue = bridge->t1.rxqueue;

	const osThreadAttr_t bridge_thread_1_attr = { .name = "bt1" };
	bridge->t1.thread = osThreadNew(bridge_thread, &(bridge->t1), &bridge_thread_1_attr);

	const osThreadAttr_t bridge_thread_2_attr = { .name = "bt2" };
	bridge->t2.thread = osThreadNew(bridge_thread, &(bridge->t2), &bridge_thread_2_attr);

	bridge->t1.layer = a;
	bridge->t2.layer = b;

	// set up snoopers
	comms_register_snooper(bridge->t1.layer, &(bridge->t1.receiver), &bridge_am_snoop, &(bridge->t1));
	comms_register_snooper(bridge->t2.layer, &(bridge->t2.receiver), &bridge_am_snoop, &(bridge->t2));

	return COMMS_SUCCESS;
}

static void bridge_send_done (comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user)
{
	comms_bridge_thread_t * bt = (comms_bridge_thread_t*)user;
	logger(result == COMMS_SUCCESS ? LOG_DEBUG3: LOG_WARN1, "%p snt %d", msg, result);
	osThreadFlagsSet(bt->thread, 0x00000001U);
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
		warn1("drop");
	}
}

static void bridge_thread (void * param)
{
	comms_bridge_thread_t * bt = (comms_bridge_thread_t*)param;

	for(;;)
	{
		if (osOK == osMessageQueueGet(bt->txqueue, &(bt->msg), NULL, osWaitForever))
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
				osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);
				debug4("snt");
			}
			else
			{
				warn1("snd");
			}
		}
		else
		{
			err1("q");
		}
	}
}
