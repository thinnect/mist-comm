#include <stdio.h>

#include "mist_comm.h"
#include "mist_comm_am.h"

int main() {
	comms_msg_t msg;
	uint8_t m[256];
	comms_layer_t* comm = (comms_layer_t*)m;

	comms_error_t err = comms_am_create(comm);
	printf("create=%d\n", err);

	comms_init_message(comm, &msg);
	comms_am_set_destination(&msg, 0xFFFF);
	comms_am_set_source(&msg, 0x0001);

	printf("%04X->%04X\n", comms_am_get_source(&msg), comms_am_get_destination(&msg));

	return 0;
}

