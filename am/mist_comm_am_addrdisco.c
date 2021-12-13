/**
 * EUI and link-local address discovery implementation for
 * the TinyOS ActiveMessage transport layer.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */

#include "mist_comm_am_addrdisco_protocol.h"
#include "mist_comm_am_addrdisco.h"
#include "mist_comm_am.h"

#include <string.h>

#include "loglevels.h"
#define __MODUUL__ "addrd"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_comm_am_addrdisco & BASE_LOG_LEVEL)
#include "log.h"
#include "sys_panic.h"

#define AM_ADDRDISCO_FLAG_RESPOND  (1 << 0)
#define AM_ADDRDISCO_FLAG_DISCOVER (1 << 1)
#define AM_ADDRDISCO_FLAG_SENT     (1 << 2)
#define AM_ADDRDISCO_FLAGS         (7)

static void receive_message (comms_layer_t * comms, const comms_msg_t * msg, void * user)
{
	am_addrdisco_t * disco = (am_addrdisco_t*)user;

	uint8_t length = comms_get_payload_length(comms, msg);
	if (length >= sizeof(am_addrdisco_packet_t))
	{
		am_addrdisco_packet_t * packet = (am_addrdisco_packet_t*)comms_get_payload(comms, msg, sizeof(am_addrdisco_packet_t));

		debugb1("rcv", packet, sizeof(am_addrdisco_packet_t));

		switch(packet->header)
		{
			case GUIDDISCOVERY_REQUEST:
			{
				am_addr_t dest = comms_am_get_destination(comms, msg);
				ieee_eui64_t eui;
				eui64_set(&eui, packet->guid);
				if ( // Broadcast and my EUI64
				  ((AM_BROADCAST_ADDR == dest)&&(0 == eui64_compare(&eui, &(comms->eui))))
				||   // My address and FFFFFFFFFFFFFFFF
				  ((comms_am_address(comms) == dest)&&eui64_is_ones(&eui))
				||   // My address and my EUI64
				  ((comms_am_address(comms) == dest)&&(0 == eui64_compare(&eui, &(comms->eui))))
				   )
				{
					// Must respond
					am_addr_t source = comms_am_get_source(comms, msg);
					if((0 != disco->respond)&&(source != disco->respond))
					{
						// A response is pending for someone else, might as well send to broadcast
						disco->respond = AM_BROADCAST_ADDR;
					}
					else
					{
						disco->respond = source;
					}

					debug1("rq %04X", (unsigned int)disco->respond);

					osThreadFlagsSet(disco->thread, 1);
				}
			}
			break;

			case GUIDDISCOVERY_RESPONSE:
			{
				comms_layer_am_t * amcomms = (comms_layer_am_t*)comms;
				comms_address_t source;
				ieee_eui64_t eui;
				comms_get_source(comms, msg, &source);
				eui64_set(&eui, packet->guid);

				debugb1("rp %04X", packet->guid, IEEE_EUI64_LENGTH, comms_am_get_source(comms, msg));

				comms_cache_update(amcomms->cache, &eui, &(source.local));
			}
			break;

			default:
				warnb1("?", packet, length);
			break;
		}
	}
	else warn1("size %u", (unsigned int)comms_get_payload_length(comms, msg));
}

static void radio_send_done (comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user)
{
	am_addrdisco_t * disco = (am_addrdisco_t*)user;
	logger(result == COMMS_SUCCESS ? LOG_DEBUG1: LOG_WARN1, "snt %d", (int)result);
	osThreadFlagsSet(disco->thread, AM_ADDRDISCO_FLAG_SENT);
}

static void addrdisco_loop (void * arg)
{
	am_addrdisco_t * disco = (am_addrdisco_t*)arg;

	for(;;)
	{
		am_addr_t dest = 0;

		comms_init_message(disco->comms, &(disco->msg));
		am_addrdisco_packet_t * packet = comms_get_payload(disco->comms, &(disco->msg), sizeof(am_addrdisco_packet_t));

		if (NULL == packet)
		{
			sys_panic("pckt");
		}

		while (osOK != osMutexAcquire(disco->mutex, osWaitForever));

		if(0 != disco->respond)
		{
			dest = disco->respond;
			packet->header = GUIDDISCOVERY_RESPONSE;
			eui64_get(&(disco->comms->eui), packet->guid);
			disco->respond = 0;
		}
		else if(0 != disco->query_addr)
		{
			dest = disco->query_addr;
			packet->header = GUIDDISCOVERY_REQUEST;
			memset(packet->guid, 0xff, sizeof(packet->guid));
			disco->query_addr = 0;
		}
		else if(false == eui64_is_zeros(&(disco->query_eui)))
		{
			dest = AM_BROADCAST_ADDR;
			packet->header = GUIDDISCOVERY_REQUEST;
			eui64_get(&(disco->query_eui), packet->guid);
			eui64_set_zeros(&(disco->query_eui));
		}

		osMutexRelease(disco->mutex);

		if (0 != dest)
		{
			comms_am_set_destination(disco->comms, &(disco->msg), dest);

			comms_set_packet_type(disco->comms, &(disco->msg), AMID_ADDRESS_DISCOVERY);
			comms_set_payload_length(disco->comms, &(disco->msg), sizeof(am_addrdisco_packet_t));

			comms_error_t result = comms_send(disco->comms, &(disco->msg), radio_send_done, disco);
			logger(result == COMMS_SUCCESS ? LOG_DEBUG1: LOG_WARN1, "snd %d", (int)result);
			if (COMMS_SUCCESS == result)
			{
				osThreadFlagsWait(AM_ADDRDISCO_FLAG_SENT, osFlagsWaitAny, osWaitForever);
			}

			// Message has been sent, now take a break for a bit
			osDelay(ADDRDISCO_BACKOFF_MS);
		}
		else // No pending activities, wait for something to happen
		{
			osThreadFlagsWait(AM_ADDRDISCO_FLAGS, osFlagsWaitAny, osWaitForever);
		}
	}
}

void comms_am_addrdisco_init (comms_layer_t * comms, am_addrdisco_t * disco, comms_addr_cache_t * cache)
{
	comms_layer_am_t * amcomms = (comms_layer_am_t*)comms;
	disco->comms = comms;
	disco->respond = 0;

	amcomms->disco = disco;

	comms_cache_init(cache);
	amcomms->cache = cache;

	comms_register_recv(comms, &(disco->rcvr), receive_message, disco, AMID_ADDRESS_DISCOVERY);

	const osMutexAttr_t app_mutex_attr = { .attr_bits = osMutexPrioInherit };
	disco->mutex = osMutexNew(&app_mutex_attr);

	const osThreadAttr_t app_thread_attr = { .name = "disco", .stack_size = 1536 };
	disco->thread = osThreadNew(addrdisco_loop, disco, &app_thread_attr);
}

void comms_am_addrdisco_deinit (comms_layer_t * comms)
{
	// TODO implement deinit
	// Stop the thread, wait out any ongoing send
	// Free the mutex
	// Remove the receiver
	sys_panic("TODO");
}

void comms_am_addrdisco_discover_eui (am_addrdisco_t * disco, am_addr_t local)
{
	if (NULL != disco)
	{
		while(osOK != osMutexAcquire(disco->mutex, osWaitForever));
		if(0 == disco->query_addr)
		{
			disco->query_addr = local;
		}
		osMutexRelease(disco->mutex);
		osThreadFlagsSet(disco->thread, AM_ADDRDISCO_FLAG_DISCOVER);
	}
}

void comms_am_addrdisco_discover_local (am_addrdisco_t * disco, ieee_eui64_t * eui)
{
	if (NULL != disco)
	{
		while(osOK != osMutexAcquire(disco->mutex, osWaitForever));
		if(eui64_is_zeros(&(disco->query_eui)))
		{
			memcpy(&(disco->query_eui), eui, sizeof(ieee_eui64_t));
		}
		osMutexRelease(disco->mutex);
		osThreadFlagsSet(disco->thread, AM_ADDRDISCO_FLAG_DISCOVER);
	}
}
