/**
 * MistComm deferred implementation on CMSIS with a timer.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */

#include "mist_comm_private.h"

#include "cmsis_os2.h"

/**
 * Initialize a deferred. In this case deferreds are implemented through an
 * os timer.
 */
void _comms_deferred_init (comms_layer_t * comms, void ** deferred, comms_deferred_f * cb)
{
	*deferred = osTimerNew(cb, osTimerOnce, (void*)comms, NULL);
}

/**
 * This is a slow implementation, will take at least a millisecond to run
 * the deferred.
 */
void _comms_defer (void * deferred)
{
	osTimerStart((osTimerId_t)deferred, 1);
}

/**
 * Free the resources allocated for the deferred.
 */
void _comms_deferred_deinit (void * deferred)
{
	osTimerDelete((osTimerId_t)deferred);
}
