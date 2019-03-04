#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // sleep

#include "mist_comm.h"
#include "mist_comm_am.h"

static bool sent = false;

const char hello[] = "HelloWorld!";

comms_msg_t* _msg = NULL;
comms_send_done_f* _sdf = NULL;
void* _user = NULL;

comms_error_t fake_comms_send(comms_layer_iface_t* comms, comms_msg_t* msg, comms_send_done_f* sdf, void* user) {
	if(_msg == NULL) {
		_msg = msg;
		_sdf = sdf;
		_user = user;
		return COMMS_SUCCESS;
	}
	return COMMS_EBUSY;
}

void send_done(void* user, comms_msg_t* msg, comms_error_t result) {
	printf("sd %p, %p, %u\n", user, msg, result);
	sent = true;
}

int main() {
	comms_msg_t msg;
	uint8_t m[256];
	uint8_t* payload;
	comms_layer_t* comm = (comms_layer_t*)m;

	comms_error_t err = comms_am_create(comm, &fake_comms_send);
	printf("create=%d\n", err);

	comms_init_message(comm, &msg);
	comms_am_set_destination(comm, &msg, 0xFFFF);
	comms_am_set_source(comm, &msg, 0x0001);

	payload = comms_get_payload(comm, &msg, strlen(hello)+1);
	if(payload == NULL) {
		printf("payload NULL\n");
		return 1;
	}
	strncpy(payload, hello, strlen(hello)+1);

	printf("%04X->%04X\n", comms_am_get_source(comm, &msg), comms_am_get_destination(comm, &msg));

	err = comms_send(comm, &msg, &send_done, NULL);
	printf("send(%p)=%d\n", &msg, err);
	if(err != COMMS_SUCCESS) {
		return 1;
	}

	while(sent == false) {
		sleep(1);
		if(_msg != NULL) {
			comms_msg_t* m = _msg;
			_msg = NULL;
			_sdf(_user, m, COMMS_SUCCESS);
		}
	}

	return 0;
}
