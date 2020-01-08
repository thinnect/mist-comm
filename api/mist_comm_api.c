#include "mist_comm.h"
#include "mist_comm_iface.h"

#include <stdbool.h>

static void comms_status_change_callback(comms_layer_t* comms, comms_status_t status, void* user) {
	comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
	cl->status = status;
	cl->status_change_user_cb(comms, status, cl->status_change_user);
	cl->status_change_user_cb = NULL;
	cl->status_change_user = NULL;
}

comms_error_t comms_start(comms_layer_t* comms, comms_status_change_f* start_done, void* user) {
	if((NULL != comms)&&(NULL != start_done)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		if(NULL != cl->start) {
			if(NULL != cl->status_change_user_cb) {
				return COMMS_EBUSY;
			}
			else {
				comms_error_t err = cl->start(cl, &comms_status_change_callback, NULL);
				if(COMMS_SUCCESS == err) {
					cl->status = COMMS_STARTING;
					cl->status_change_user_cb = start_done;
					cl->status_change_user = user;
				}
				return err;
			}
		}
	}
	return COMMS_EINVAL;
}

comms_error_t comms_stop(comms_layer_t* comms, comms_status_change_f* stop_done, void* user) {
	if((NULL != comms)&&(NULL != stop_done)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		if(NULL != cl->stop) {
			if(NULL != cl->status_change_user_cb) {
				return COMMS_EBUSY;
			}
			else {
				comms_error_t err = cl->stop(cl, &comms_status_change_callback, NULL);
				if(COMMS_SUCCESS == err) {
					cl->status = COMMS_STOPPING;
					cl->status_change_user_cb = stop_done;
					cl->status_change_user = user;
				}
				return err;
			}
		}
	}
	return COMMS_EINVAL;
}

comms_status_t comms_status(comms_layer_t* comms) {
	comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
	return cl->status;
}

void comms_init_message(comms_layer_t* comms, comms_msg_t* msg) {
	if((comms != NULL)&&(msg != NULL)) {
		((comms_layer_iface_t*)comms)->init_message((comms_layer_iface_t*)comms, msg);
	}
}

comms_error_t comms_send(comms_layer_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	if(comms != NULL) {
		return ((comms_layer_iface_t*)comms)->send((comms_layer_iface_t*)comms, msg, sdf, user);
	}
	return COMMS_EINVAL;
}

comms_error_t comms_register_recv(comms_layer_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void* user, am_id_t amid) {
	if((comms != NULL)&&(rcvr != NULL)&&(func != NULL)) {
		return ((comms_layer_iface_t*)comms)->register_recv((comms_layer_iface_t*)comms, rcvr, func, user, amid);
	}
	return COMMS_EINVAL;
}
comms_error_t comms_deregister_recv(comms_layer_t* comms, comms_receiver_t* rcvr) {
	if(comms != NULL) {
		return ((comms_layer_iface_t*)comms)->deregister_recv((comms_layer_iface_t*)comms, rcvr);
	}
	return COMMS_EINVAL;
}

comms_error_t comms_register_snooper(comms_layer_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void* user) {
	if((comms != NULL)&&(rcvr != NULL)&&(func != NULL)) {
		return ((comms_layer_iface_t*)comms)->register_snooper((comms_layer_iface_t*)comms, rcvr, func, user);
	}
	return COMMS_EINVAL;
}
comms_error_t comms_deregister_snooper(comms_layer_t* comms, comms_receiver_t* rcvr) {
	if(comms != NULL) {
		return ((comms_layer_iface_t*)comms)->deregister_snooper((comms_layer_iface_t*)comms, rcvr);
	}
	return COMMS_EINVAL;
}

am_id_t comms_get_packet_type(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_packet_type(cl, msg);
	}
	return 0;
}
void comms_set_packet_type(comms_layer_t* comms, comms_msg_t* msg, am_id_t ptype) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		cl->set_packet_type(cl, msg, ptype);
	}
}

uint8_t comms_get_payload_max_length(comms_layer_t* comms) {
	return ((comms_layer_iface_t*)comms)->get_payload_max_length((comms_layer_iface_t*)comms);
}

uint8_t comms_get_payload_length(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_payload_length(cl, msg);
	}
	return 0;
}
void comms_set_payload_length(comms_layer_t* comms, comms_msg_t* msg, uint8_t length) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		cl->set_payload_length(cl, msg, length);
	}
}
void* comms_get_payload(comms_layer_t* comms, const comms_msg_t* msg, uint8_t length) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_payload(cl, msg, length);
	}
	return NULL;
}

uint8_t comms_get_retries(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_retries(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_retries(comms_layer_t* comms, comms_msg_t* msg, uint8_t count) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_retries(cl, msg, count);
	}
	return COMMS_EINVAL;
}

uint8_t comms_get_retries_used(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_retries_used(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_retries_used(comms_layer_t* comms, comms_msg_t* msg, uint8_t count) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_retries_used(cl, msg, count);
	}
	return COMMS_EINVAL;
}

uint32_t comms_get_timeout(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_timeout(cl, msg);
	}
	return 0;
}
comms_error_t comms_set_timeout(comms_layer_t* comms, comms_msg_t* msg, uint32_t timeout) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_timeout(cl, msg, timeout);
	}
	return COMMS_EINVAL;
}

bool comms_is_ack_required(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->is_ack_required(cl, msg);
	}
	return false;
}
comms_error_t comms_set_ack_required(comms_layer_t* comms, comms_msg_t* msg, bool required) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_ack_required(cl, msg, required);
	}
	return COMMS_EINVAL;
}
bool comms_ack_received(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->ack_received(cl, msg);
	}
	return false;
}
void _comms_set_ack_received(comms_layer_t* comms, comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		cl->set_ack_received(cl, msg);
	}
}

comms_error_t comms_set_timestamp(comms_layer_t* comms, comms_msg_t* msg, uint32_t timestamp) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_timestamp(cl, msg, timestamp);
	}
	return COMMS_EINVAL;
}
uint32_t comms_get_timestamp(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_timestamp(cl, msg);
	}
	return 0;
}
bool comms_timestamp_valid(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->timestamp_valid(cl, msg);
	}
	return false;
}

comms_error_t comms_set_event_time(comms_layer_t* comms, comms_msg_t* msg, uint32_t evt) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->set_event_time(cl, msg, evt);
	}
	return COMMS_EINVAL;
}
uint32_t comms_get_event_time(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_event_time(cl, msg);
	}
	return 0;
}
bool comms_event_time_valid(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->event_time_valid(cl, msg);
	}
	return false;
}

uint8_t comms_get_lqi(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_lqi(cl, msg);
	}
	return 0;
}
void _comms_set_lqi(comms_layer_t* comms, comms_msg_t* msg, uint8_t lqi) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		cl->set_lqi(cl, msg, lqi);
	}
}

int8_t comms_get_rssi(comms_layer_t* comms, const comms_msg_t* msg) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		return cl->get_rssi(cl, msg);
	}
	return -128;
}
void _comms_set_rssi(comms_layer_t* comms, comms_msg_t* msg, int8_t rssi) {
	if((msg != NULL)&&(comms != NULL)) {
		comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
		cl->set_rssi(cl, msg, rssi);
	}
}

void _comms_mutex_acquire(comms_layer_t* comms) {
	comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
	if(NULL != cl->mutex) {
		cl->mutex_acquire(cl);
	}
}
void _comms_mutex_release(comms_layer_t* comms) {
	comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
	if(NULL != cl->mutex) {
		cl->mutex_release(cl);
	}
}
