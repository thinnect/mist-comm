/**
 * MistComm locking implementation with CMSIS mutexes.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 * @author Raido Pahtma
 */

#include "mist_comm_iface.h"
#include "mist_comm_private.h"
#include "cmsis_os2.h"

static void mutex_acquire(comms_layer_iface_t * cl)
{
	osMutexAcquire((osMutexId_t)(cl->mutex), osWaitForever);
}

static void mutex_release(comms_layer_iface_t * cl)
{
	osMutexRelease((osMutexId_t)(cl->mutex));
}


void _comms_mutex_init(comms_layer_t * comms)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	cl->mutex = (void*)osMutexNew(NULL);
	cl->mutex_acquire = mutex_acquire;
	cl->mutex_release = mutex_release;
}

