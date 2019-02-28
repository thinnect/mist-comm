#include "mist_comm.h"
#include "mist_comm_iface.h"

#include <stdbool.h>

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

comms_receiver_id_t comms_register_recv(comms_layer_t* comms, comms_receive_f* func, void *user, am_id_t amid) {
	if(comms != NULL) {
		return ((comms_layer_iface_t*)comms)->register_recv((comms_layer_iface_t*)comms, func, user, amid);
	}
	return -1;
}
comms_error_t comms_deregister_recv(comms_layer_t* comms, comms_receiver_id_t rid) {
	if(comms != NULL) {
		return ((comms_layer_iface_t*)comms)->deregister_recv((comms_layer_iface_t*)comms, rid);
	}
	return COMMS_EINVAL;
}

uint8_t comms_get_payload_length(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_payload_length(comms, msg);
		}
	}
	return 0;
}
void comms_set_payload_length(comms_msg_t* msg, uint8_t length) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			comms->set_payload_length(comms, msg, length);
		}
	}
}
void* comms_get_payload(comms_msg_t* msg, uint8_t length) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_payload(comms, msg, length);
		}
	}
	return NULL;
}

uint8_t comms_get_retries(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_retries(comms, msg);
		}
	}
	return 0;
}
comms_error_t comms_set_retries(comms_msg_t* msg, uint8_t count) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->set_retries(comms, msg, count);
		}
	}
	return COMMS_EINVAL;
}

uint8_t comms_get_retries_used(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_retries_used(comms, msg);
		}
	}
	return 0;
}
comms_error_t comms_set_retries_used(comms_msg_t* msg, uint8_t count) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->set_retries_used(comms, msg, count);
		}
	}
	return COMMS_EINVAL;
}

uint32_t comms_get_timeout(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_timeout(comms, msg);
		}
	}
	return 0;
}
comms_error_t comms_set_timeout(comms_msg_t* msg, uint32_t timeout) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->set_timeout(comms, msg, timeout);
		}
		return COMMS_UNINITIALIZED;
	}
	return COMMS_EINVAL;
}

bool comms_is_ack_required(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->is_ack_required(comms, msg);
		}
	}
	return false;
}
comms_error_t comms_set_ack_required(comms_msg_t* msg, bool required) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->set_ack_required(comms, msg, required);
		}
		return COMMS_UNINITIALIZED;
	}
	return COMMS_EINVAL;
}
bool comms_ack_received(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->ack_received(comms, msg);
		}
	}
	return false;
}

comms_error_t comms_set_event_time(comms_msg_t* msg, uint32_t evt) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->set_event_time(comms, msg, evt);
		}
		return COMMS_UNINITIALIZED;
	}
	return COMMS_EINVAL;
}
uint32_t comms_get_event_time(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_event_time(comms, msg);
		}
	}
	return 0;
}
bool comms_event_time_valid(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->event_time_valid(comms, msg);
		}
	}
	return false;
}

uint8_t comms_get_lqi(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_lqi(comms, msg);
		}
	}
	return 0;
}
void comms_set_lqi(comms_msg_t* msg, uint8_t lqi) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			comms->set_lqi(comms, msg, lqi);
		}
	}
}

int8_t comms_get_rssi(comms_msg_t* msg) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			return comms->get_rssi(comms, msg);
		}
	}
	return -128;
}
void comms_set_rssi(comms_msg_t* msg, int8_t rssi) {
	if(msg != NULL) {
		if(msg->layer != NULL) {
			comms_layer_iface_t* comms = (comms_layer_iface_t*)msg->layer;
			comms->set_rssi(comms, msg, rssi);
		}
	}
}
