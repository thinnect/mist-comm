#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // sleep

#include "mist_comm.h"
#include "mist_comm_am.h"

const char hello[] = "HelloWorld!";

comms_msg_t mmsg1;
comms_msg_t* mcopy1 = &mmsg1;
comms_msg_t* _msg1 = NULL;
comms_send_done_f* _sdf1 = NULL;
void* _user1 = NULL;

comms_error_t fake_comms_send1(comms_layer_iface_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	if(_msg1 == NULL) {
		_msg1 = msg;
		_sdf1 = sdf;
		_user1 = user;
		return COMMS_SUCCESS;
	}
	return COMMS_EBUSY;
}

comms_msg_t mmsg2;
comms_msg_t* mcopy2 = &mmsg2;
comms_msg_t* _msg2 = NULL;
comms_send_done_f* _sdf2 = NULL;
void* _user2 = NULL;

comms_error_t fake_comms_send2(comms_layer_iface_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	if(_msg2 == NULL) {
		_msg2 = msg;
		_sdf2 = sdf;
		_user2 = user;
		return COMMS_SUCCESS;
	}
	return COMMS_EBUSY;
}

comms_msg_t* fake_comms_receive(comms_layer_t* comms, comms_msg_t* msg, void* user) {
	printf("rcv %p, %p, %p\n", comms, msg, user);
	printf("%04X->%04X %s\n", comms_am_get_source(comms, msg),
		comms_am_get_destination(comms, msg),
		(char*)comms_get_payload(comms, msg, comms_get_payload_length(comms, msg)));
	return msg;
}

void send_done(comms_layer_t* comms, comms_msg_t* msg, comms_error_t result, void* user) {
	printf("sd %p, %p, %u, %p\n", comms, msg, result, user);
}

int main() {
	comms_msg_t msg;
	uint8_t r1[256];
	uint8_t r2[256];
	uint8_t* payload;
	comms_layer_t* radio1 = (comms_layer_t*)r1;
	comms_layer_t* radio2 = (comms_layer_t*)r2;
	comms_receiver_t rcv1;
	comms_receiver_t rcv2;
	comms_error_t err;
	uint8_t length = strlen(hello)+1;

	err = comms_am_create(radio1, &fake_comms_send1);
	printf("create1=%d\n", err);
	err = comms_am_create(radio2, &fake_comms_send2);
	printf("create2=%d\n", err);

	comms_register_recv(radio1, &rcv1, &fake_comms_receive, NULL, 0xAB);
	comms_register_recv(radio2, &rcv2, &fake_comms_receive, NULL, 0xAB);

	comms_init_message(radio1, &msg);
	comms_set_packet_type(radio1, &msg, 0xAB);
	comms_am_set_destination(radio1, &msg, 0xFFFF);
	comms_am_set_source(radio1, &msg, 0x0001);

	payload = comms_get_payload(radio1, &msg, length);
	if(payload == NULL) {
		printf("payload NULL\n");
		return 1;
	}
	strncpy(payload, hello, length);
	comms_set_payload_length(radio1, &msg, length);

	//printf("%x\n", comms_am_get_source(radio1, &msg));
	//printf("%04X->%04X\n", comms_am_get_source(radio1, &msg), comms_am_get_destination(radio1, &msg));

	err = comms_send(radio1, &msg, &send_done, NULL);
	printf("send(%p)=%d\n", &msg, err);
	if(err != COMMS_SUCCESS) {
		return 1;
	}

	while(true) {
		sleep(1);
		if(_msg1 != NULL) {
			comms_msg_t* m = _msg1;
			_msg1 = NULL;
			memcpy(mcopy1, m, sizeof(comms_msg_t));
			mcopy1 = comms_deliver(radio2, mcopy1);
			_sdf1(radio1, m, COMMS_SUCCESS, _user1);
		}
		if(_msg2 != NULL) {
			comms_msg_t* m = _msg2;
			_msg2 = NULL;
			memcpy(mcopy2, m, sizeof(comms_msg_t));
			mcopy2 = comms_deliver(radio1, mcopy2);
			_sdf2(radio2, m, COMMS_SUCCESS, _user2);
		}
	}

	return 0;
}
