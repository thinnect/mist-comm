/**
 * MistComm locking implementation with CMSIS mutexes.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 * @author Raido Pahtma
 */

#include "mist_comm_private.h"
#include "cmsis_os2.h"

void _comms_mutex_init(comms_layer_t * comms)
{
	comms->mutex = (void*)osMutexNew(NULL);
}

void _comms_mutex_acquire(comms_layer_t * comms)
{
	osMutexAcquire((osMutexId_t)(comms->mutex), osWaitForever);
}

void _comms_mutex_release(comms_layer_t * comms)
{
	osMutexRelease((osMutexId_t)(comms->mutex));
}
