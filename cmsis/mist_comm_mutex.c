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

static const osMutexAttr_t attr = {"comms", osMutexPrioInherit, NULL, 0U};

void comms_mutex_acquire(commsMutexId_t mutex)
{
	while(osOK != osMutexAcquire((osMutexId_t)(mutex), osWaitForever));
}

void comms_mutex_release(commsMutexId_t mutex)
{
	osMutexRelease((osMutexId_t)mutex);
}

commsMutexId_t comms_mutex_create()
{
	return (commsMutexId_t)osMutexNew(&attr);
}
