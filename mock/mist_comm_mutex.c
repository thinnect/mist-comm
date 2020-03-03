/**
 * MistComm locking implementation with mock mutexes.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 * @author Raido Pahtma
 */

#include "mist_comm_iface.h"
#include "mist_comm_private.h"

static volatile bool m_mutexes[8];
static int m_mutex_count = 0;

void comms_mutex_acquire(commsMutexId_t mutex)
{
	while(m_mutexes[mutex]);
	m_mutexes[mutex] = true;
}

void comms_mutex_release(commsMutexId_t mutex)
{
	m_mutexes[mutex] = false;
}

commsMutexId_t comms_mutex_create()
{
	m_mutexes[m_mutex_count] = false;
	return m_mutex_count++;
}
