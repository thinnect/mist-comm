#include "mist_comm_iface.h"
#include <string.h>

static comms_error_t rcv_comms_register_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void* user, am_id_t amid) {
	comms_receiver_t** indirect;

	comms_mutex_acquire(comms->receiver_mutex);

	for(indirect=&(comms->receivers); NULL != *indirect; indirect = &((*indirect)->next)) {
		if(*indirect == rcvr) {
			comms_mutex_release(comms->receiver_mutex);
			return COMMS_ALREADY;
		}
	}
	*indirect = rcvr;

	rcvr->type = amid;
	rcvr->callback = func;
	rcvr->user = user;
	rcvr->next = NULL;

	comms_mutex_release(comms->receiver_mutex);
	return COMMS_SUCCESS;
}

static comms_error_t rcv_comms_register_snooper(comms_layer_iface_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void* user) {
	comms_receiver_t** indirect;

	comms_mutex_acquire(comms->receiver_mutex);

	for(indirect=&(comms->snoopers); NULL != *indirect; indirect = &((*indirect)->next)) {
		if(*indirect == rcvr) {
			comms_mutex_release(comms->receiver_mutex);
			return COMMS_ALREADY;
		}
	}
	*indirect = rcvr;

	rcvr->type = 0; // Not used
	rcvr->callback = func;
	rcvr->user = user;
	rcvr->next = NULL;

	comms_mutex_release(comms->receiver_mutex);
	return COMMS_SUCCESS;
}

static comms_error_t rcv_comms_deregister_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr) {
	comms_receiver_t** indirect;

	comms_mutex_acquire(comms->receiver_mutex);

	for(indirect=&(comms->receivers); NULL != *indirect; indirect = &((*indirect)->next)) {
		if(*indirect == rcvr) {
			*indirect = rcvr->next;
			comms_mutex_release(comms->receiver_mutex);
			return COMMS_SUCCESS;
		}
	}
	comms_mutex_release(comms->receiver_mutex);
	return COMMS_FAIL;
}

static comms_error_t rcv_comms_deregister_snooper(comms_layer_iface_t* comms, comms_receiver_t* rcvr) {
	comms_receiver_t** indirect;

	comms_mutex_acquire(comms->receiver_mutex);

	for(indirect=&(comms->snoopers); NULL != *indirect; indirect = &((*indirect)->next)) {
		if(*indirect == rcvr) {
			*indirect = rcvr->next;
			comms_mutex_release(comms->receiver_mutex);
			return COMMS_SUCCESS;
		}
	}
	comms_mutex_release(comms->receiver_mutex);
	return COMMS_FAIL;
}

comms_msg_t* comms_deliver(comms_layer_t* layer, comms_msg_t* msg) {
	comms_layer_iface_t* comms = (comms_layer_iface_t*)layer;
	am_id_t ptype = comms_get_packet_type(layer, msg);
	comms_receiver_t* receiver;

	comms_mutex_acquire(comms->receiver_mutex);

	// Receivers filter based on type
	for(receiver=comms->receivers;receiver!=NULL;receiver=receiver->next) {
		if(receiver->type == ptype) {
			receiver->callback(layer, msg, receiver->user);
		}
	}

	// Snoopers get everyting
	for(receiver=comms->snoopers;receiver!=NULL;receiver=receiver->next) {
		receiver->callback(layer, msg, receiver->user);
	}

	comms_mutex_release(comms->receiver_mutex);

	return msg;
}

comms_error_t comms_initialize_rcvr_management(comms_layer_iface_t* comms) {
	comms_mutex_acquire(comms->receiver_mutex);
	comms->receivers = NULL;
	comms->register_recv = &rcv_comms_register_recv;
	comms->deregister_recv = &rcv_comms_deregister_recv;
	comms->register_snooper = &rcv_comms_register_snooper;
	comms->deregister_snooper = &rcv_comms_deregister_snooper;
	comms_mutex_release(comms->receiver_mutex);
	return COMMS_SUCCESS;
}
