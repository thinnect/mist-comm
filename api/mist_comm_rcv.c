#include "mist_comm_iface.h"
#include <string.h>

static comms_error_t rcv_comms_register_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr, comms_receive_f* func, void* user, am_id_t amid) {
	rcvr->type = amid;
	rcvr->callback = func;
	rcvr->user = user;
	rcvr->next = NULL;
	if(comms->receivers == NULL) {
		comms->receivers = rcvr;
	}
	else {
		comms_receiver_t* receiver;
		for(receiver=comms->receivers;receiver->next!=NULL;receiver=receiver->next);
		receiver->next = rcvr;
	}
	return COMMS_SUCCESS;
}

static comms_error_t rcv_comms_deregister_recv(comms_layer_iface_t* comms, comms_receiver_t* rcvr) {
	if(comms->receivers == NULL) {
		return COMMS_FAIL; // Nothing registered
	}
	else if(comms->receivers == rcvr) {
		comms->receivers = rcvr->next;
		return COMMS_SUCCESS;
	}
	else {
		comms_receiver_t* receiver;
		comms_receiver_t* last = comms->receivers;
		for(receiver=last->next;receiver!=NULL;receiver=receiver->next) {
			if(receiver == rcvr) {
				last->next = receiver->next;
				return COMMS_SUCCESS;
			}
			last = receiver;
		}
	}
	return COMMS_FAIL;
}

comms_msg_t* comms_deliver(comms_layer_t* comms, comms_msg_t* msg) {
	comms_layer_iface_t* cl = (comms_layer_iface_t*)comms;
	am_id_t ptype = comms_get_packet_type(comms, msg);

	comms_msg_t copy;
	bool copied = false;

	comms_receiver_t* receiver = cl->receivers;
	for(receiver=cl->receivers;receiver!=NULL;receiver=receiver->next) {
		if(receiver->type == ptype) {
			if(copied) { // msg has been received from user and may be modified
				memcpy(msg, &copy, sizeof(comms_msg_t));
			}
			else { // make a backup of msg
				memcpy(&copy, msg, sizeof(comms_msg_t));
				copied = true;
			}
			msg = receiver->callback(comms, msg, receiver->user);
		}
	}
	return msg;
}

comms_error_t comms_initialize_rcvr_management(comms_layer_iface_t* comms) {
	comms->receivers = NULL;
	comms->register_recv = &rcv_comms_register_recv;
	comms->deregister_recv = &rcv_comms_deregister_recv;
	return COMMS_SUCCESS;
}
