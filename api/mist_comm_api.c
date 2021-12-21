/**
 * Mist communications API.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */

#include "mist_comm.h"
#include "mist_comm_iface.h"

#include <stdbool.h>
#include <string.h>

volatile uint8_t MIST_COMM_VERSION_PASTER(g_mist_comm_version_major_, MIST_COMM_VERSION_MAJOR) = MIST_COMM_VERSION_MAJOR;
volatile uint8_t MIST_COMM_VERSION_PASTER(g_mist_comm_version_minor_, MIST_COMM_VERSION_MINOR) = MIST_COMM_VERSION_MINOR;

static void comms_status_change_callback (comms_layer_t * comms, comms_status_t status, void * user)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;

	comms_mutex_acquire(cl->start_stop_mutex);
	cl->status_change_user_cb(comms, status, cl->status_change_user);
	cl->status = status;
	cl->status_change_user_cb = NULL;
	cl->status_change_user = NULL;
	comms_mutex_release(cl->start_stop_mutex);
}

comms_error_t comms_start (comms_layer_t * comms, comms_status_change_f* start_done, void * user)
{
	if ((NULL != comms)&&(NULL != start_done))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		if (NULL != cl->start)
		{
			comms_mutex_acquire(cl->start_stop_mutex);
			if (NULL != cl->status_change_user_cb)
			{
				comms_mutex_release(cl->start_stop_mutex);
				return COMMS_EBUSY;
			}
			else if (COMMS_STARTED == cl->status)
			{
				comms_mutex_release(cl->start_stop_mutex);
				return COMMS_ALREADY;
			}
			else
			{
				comms_status_t status = cl->status;
				cl->status = COMMS_STARTING;
				cl->status_change_user_cb = start_done;
				cl->status_change_user = user;
				comms_mutex_release(cl->start_stop_mutex);

				comms_error_t err = cl->start(cl, &comms_status_change_callback, NULL);
				if (COMMS_SUCCESS != err)
				{
					comms_mutex_acquire(cl->start_stop_mutex);
					cl->status = status;
					cl->status_change_user_cb = NULL;
					cl->status_change_user = NULL;
					comms_mutex_release(cl->start_stop_mutex);
				}
				return err;
			}
		}
	}
	return COMMS_EINVAL;
}

comms_error_t comms_stop (comms_layer_t * comms, comms_status_change_f* stop_done, void * user)
{
	if ((NULL != comms)&&(NULL != stop_done))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		if (NULL != cl->stop)
		{
			comms_mutex_acquire(cl->start_stop_mutex);
			if (NULL != cl->status_change_user_cb)
			{
				comms_mutex_release(cl->start_stop_mutex);
				return COMMS_EBUSY;
			}
			else if (COMMS_STOPPED == cl->status)
			{
				comms_mutex_release(cl->start_stop_mutex);
				return COMMS_ALREADY;
			}
			else
			{
				comms_status_t status = cl->status;
				cl->status = COMMS_STOPPING;
				cl->status_change_user_cb = stop_done;
				cl->status_change_user = user;
				comms_mutex_release(cl->start_stop_mutex);

				comms_error_t err = cl->stop(cl, &comms_status_change_callback, NULL);
				if (COMMS_SUCCESS != err)
				{
					comms_mutex_acquire(cl->start_stop_mutex);
					cl->status = status;
					cl->status_change_user_cb = NULL;
					cl->status_change_user = NULL;
					comms_mutex_release(cl->start_stop_mutex);
				}
				return err;
			}
		}
	}
	return COMMS_EINVAL;
}

comms_status_t comms_status (comms_layer_t * comms)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	comms_status_t status;
	comms_mutex_acquire(cl->start_stop_mutex);
	status = cl->status;
	comms_mutex_release(cl->start_stop_mutex);
	return status;
}

void comms_init_message (comms_layer_t * comms, comms_msg_t * msg)
{
	if ((NULL != comms)&&(NULL != msg))
	{
		((comms_layer_iface_t*)comms)->init_message((comms_layer_iface_t*)comms, msg);
	}
}

void comms_get_destination (comms_layer_t * comms, const comms_msg_t * msg, comms_address_t * destination)
{
	memcpy(destination, &(msg->body.destination), sizeof(comms_address_t));
}

void comms_set_destination (comms_layer_t * comms, comms_msg_t * msg, const comms_address_t * destination)
{
	memcpy(&(msg->body.destination), destination, sizeof(comms_address_t));
}

void comms_get_source (comms_layer_t * comms, const comms_msg_t * msg, comms_address_t * source)
{
	memcpy(source, &(msg->body.source), sizeof(comms_address_t));
}

void comms_set_source (comms_layer_t * comms, comms_msg_t * msg, const comms_address_t * source)
{
	memcpy(&(msg->body.source), source, sizeof(comms_address_t));
}

comms_error_t comms_send (comms_layer_t * comms, comms_msg_t * msg, comms_send_done_f* sdf, void * user)
{
	if (NULL != comms)
	{
		return ((comms_layer_iface_t*)comms)->sendf((comms_layer_iface_t*)comms, msg, sdf, user);
	}
	return COMMS_EINVAL;
}

bool comms_deliver (comms_layer_t * comms, comms_msg_t * msg)
{
	comms_layer_iface_t * iface = (comms_layer_iface_t*)comms;
	if ((iface != NULL)&&(iface->deliverf != NULL))
	{
		return iface->deliverf(iface, msg);
	}
	return false;
}

comms_error_t comms_register_recv (comms_layer_t * comms, comms_receiver_t* rcvr, comms_receive_f* func, void * user, am_id_t amid)
{
	if ((NULL != comms)&&(rcvr != NULL)&&(func != NULL))
	{
		return ((comms_layer_iface_t*)comms)->register_recv((comms_layer_iface_t*)comms, rcvr, func, user, amid, false);
	}
	return COMMS_EINVAL;
}
comms_error_t comms_register_recv_eui (comms_layer_t * comms, comms_receiver_t* rcvr, comms_receive_f* func, void * user, am_id_t amid)
{
	if ((NULL != comms)&&(rcvr != NULL)&&(func != NULL))
	{
		return ((comms_layer_iface_t*)comms)->register_recv((comms_layer_iface_t*)comms, rcvr, func, user, amid, true);
	}
	return COMMS_EINVAL;
}
comms_error_t comms_deregister_recv (comms_layer_t * comms, comms_receiver_t* rcvr)
{
	if (NULL != comms)
	{
		return ((comms_layer_iface_t*)comms)->deregister_recv((comms_layer_iface_t*)comms, rcvr);
	}
	return COMMS_EINVAL;
}

comms_error_t comms_register_snooper (comms_layer_t * comms, comms_receiver_t* rcvr, comms_receive_f* func, void * user)
{
	if ((NULL != comms)&&(rcvr != NULL)&&(func != NULL))
	{
		return ((comms_layer_iface_t*)comms)->register_snooper((comms_layer_iface_t*)comms, rcvr, func, user);
	}
	return COMMS_EINVAL;
}
comms_error_t comms_deregister_snooper (comms_layer_t * comms, comms_receiver_t* rcvr)
{
	if (NULL != comms)
	{
		return ((comms_layer_iface_t*)comms)->deregister_snooper((comms_layer_iface_t*)comms, rcvr);
	}
	return COMMS_EINVAL;
}

am_id_t comms_get_packet_type (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_packet_type(cl, msg);
	}
	return 0;
}
void comms_set_packet_type (comms_layer_t * comms, comms_msg_t * msg, am_id_t ptype)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_packet_type(cl, msg, ptype);
	}
}

uint16_t comms_get_packet_group (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_packet_group(cl, msg);
	}
	return 0;
}
void comms_set_packet_group (comms_layer_t * comms, comms_msg_t * msg, uint16_t group)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_packet_group(cl, msg, group);
	}
}

uint8_t comms_get_payload_max_length (comms_layer_t * comms)
{
	return ((comms_layer_iface_t*)comms)->get_payload_max_length((comms_layer_iface_t*)comms);
}

uint8_t comms_get_payload_length (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_payload_length(cl, msg);
	}
	return 0;
}
void comms_set_payload_length (comms_layer_t * comms, comms_msg_t * msg, uint8_t length)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_payload_length(cl, msg, length);
	}
}
void * comms_get_payload (comms_layer_t * comms, const comms_msg_t * msg, uint8_t length)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_payload(cl, msg, length);
	}
	return NULL;
}

uint8_t comms_get_retries (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_retries(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_retries (comms_layer_t * comms, comms_msg_t * msg, uint8_t count)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_retries(cl, msg, count);
	}
	return COMMS_EINVAL;
}

uint8_t comms_get_retries_used (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_retries_used(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_retries_used (comms_layer_t * comms, comms_msg_t * msg, uint8_t count)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_retries_used(cl, msg, count);
	}
	return COMMS_EINVAL;
}

uint32_t comms_get_timeout (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_timeout(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_timeout (comms_layer_t * comms, comms_msg_t * msg, uint32_t timeout)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_timeout(cl, msg, timeout);
	}
	return COMMS_EINVAL;
}

bool comms_is_ack_required (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->is_ack_required(cl, msg);
	}
	return false;
}
comms_error_t comms_set_ack_required (comms_layer_t * comms, comms_msg_t * msg, bool required)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_ack_required(cl, msg, required);
	}
	return COMMS_EINVAL;
}
bool comms_ack_received (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->ack_received(cl, msg);
	}
	return false;
}
void comms_set_ack_received (comms_layer_t * comms, comms_msg_t * msg, bool received)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_ack_received(cl, msg, received);
	}
}

uint32_t comms_get_time_micro (comms_layer_t * comms)
{
	if (NULL != comms)
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		if (NULL != cl->get_time_micro)
		{
			return cl->get_time_micro(cl);
		}
	}
	return 0;
}

comms_error_t comms_set_timestamp_micro (comms_layer_t * comms, comms_msg_t * msg, uint32_t timestamp)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_timestamp(cl, msg, timestamp);
	}
	return COMMS_EINVAL;
}
uint32_t comms_get_timestamp_micro (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_timestamp(cl, msg);
	}
	return 0;
}
bool comms_timestamp_valid (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->timestamp_valid(cl, msg);
	}
	return false;
}

comms_error_t comms_set_event_time_micro (comms_layer_t * comms, comms_msg_t * msg, uint32_t evt)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->set_event_time(cl, msg, evt);
	}
	return COMMS_EINVAL;
}
uint32_t comms_get_event_time_micro (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_event_time(cl, msg);
	}
	return 0;
}

bool comms_event_time_valid (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->event_time_valid(cl, msg);
	}
	return false;
}

uint8_t comms_get_lqi (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_lqi(cl, msg);
	}
	return 0;
}
void comms_set_lqi (comms_layer_t * comms, comms_msg_t * msg, uint8_t lqi)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_lqi(cl, msg, lqi);
	}
}

int8_t comms_get_rssi (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_rssi(cl, msg);
	}
	return -128;
}
void comms_set_rssi (comms_layer_t * comms, comms_msg_t * msg, int8_t rssi)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_rssi(cl, msg, rssi);
	}
}

int8_t comms_get_priority (comms_layer_t * comms, const comms_msg_t * msg)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		return cl->get_priority(cl, msg);
	}
	return 0xFF;
}
void comms_set_priority (comms_layer_t * comms, comms_msg_t * msg, int8_t priority)
{
	if ((NULL != msg)&&(NULL != comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
		cl->set_priority(cl, msg, priority);
	}
}
