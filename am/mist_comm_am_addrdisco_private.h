/**
 * Active Message address discovery object structure.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef MIST_COMM_AM_ADDRDISCO_PRIVATE_H_
#define MIST_COMM_AM_ADDRDISCO_PRIVATE_H_

struct am_addrdisco
{
	comms_layer_t * comms;

	comms_receiver_t rcvr;

	comms_msg_t msg; // Memory for sending message

	am_addr_t respond; // Who to respond with my EUI

	// TODO Discoveries should probably have a queue
	am_addr_t    query_addr; // Whose EUI needs to be discovered
	ieee_eui64_t query_eui;  // Whose local address needs to be discovered

	osMutexId_t mutex;
	osThreadId_t thread; // TODO having a dedicated thread here is wasteful
};

#endif//MIST_COMM_AM_ADDRDISCO_PRIVATE_H_
