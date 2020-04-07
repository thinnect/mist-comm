/**
 * MistComm bridge.
 * Bridge messages between 2 mist-comm communication interfaces.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 * @author Raido Pahtma
 */

#ifndef MIST_COMM_BRIDGE_H_
#define MIST_COMM_BRIDGE_H_

#include "cmsis_os2.h"
#include "mist_comm.h"

typedef struct comms_bridge_thread
{
	osThreadId_t thread;

	comms_receiver_t receiver;
	comms_layer_t * layer;

	osMessageQueueId_t rxqueue;
	osMessageQueueId_t txqueue;
	comms_msg_t msg;
} comms_bridge_thread_t;

typedef struct comms_bridge
{
	comms_bridge_thread_t t1;
	comms_bridge_thread_t t2;
} comms_bridge_t;

/**
 * Initialize and start a bridge.
 *
 * @param bridge The bridge to initialize.
 * @param a      The first communications interface.
 * @param b      The second communications interface.
 * @return COMMS_SUCCESS for success.
 */
comms_error_t comms_bridge_init (comms_bridge_t * bridge,
                                 comms_layer_t * a,
                                 comms_layer_t * b);

/**
 * Deinitialize a bridge, freeing all resources.
 *
 * MAY TAKE SOME TIME (10-15 seconds)!
 *
 * @param bridge The bridge to initialize.
 */
void comms_bridge_deinit (comms_bridge_t * bridge);

#endif//MIST_COMM_BRIDGE_H_
