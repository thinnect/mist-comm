/**
 * EUI-local address cache.
 *
 * Will discard older elements when it runs out of space.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#include "mist_comm_addrcache.h"

#include "cmsis_os2_ext.h" // osCounterGetSecond, but should probably use an alternative time-source

#include <string.h>

#include "loglevels.h"
#define __MODUUL__ "addrc"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_comm_addrcache & BASE_LOG_LEVEL)
#include "log.h"

bool comms_cache_get_local (comms_addr_cache_t * cache, const ieee_eui64_t * eui, comms_local_addr_t * local)
{
	if (NULL == cache)
	{
		return false;
	}

	comms_mutex_acquire(cache->mutex);
	for (int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
	{
		if (0 == memcmp(eui, &(cache->euis[i]), sizeof(ieee_eui64_t)))
		{
			memcpy(local, &(cache->locals[i]), sizeof(comms_local_addr_t));
			comms_mutex_release(cache->mutex);
			return true;
		}
	}
	comms_mutex_release(cache->mutex);
	return false;
}

bool comms_cache_get_eui (comms_addr_cache_t * cache, const comms_local_addr_t * local, ieee_eui64_t * eui)
{
	if (NULL == cache)
	{
		return false;
	}

	comms_mutex_acquire(cache->mutex);
	for (int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
	{
		if (0 == memcmp(local, &(cache->locals[i]), sizeof(comms_local_addr_t)))
		{
			memcpy(eui, &(cache->euis[i]), sizeof(ieee_eui64_t));
			comms_mutex_release(cache->mutex);
			return true;
		}
	}
	comms_mutex_release(cache->mutex);
	return false;
}

void comms_cache_update (comms_addr_cache_t * cache, const ieee_eui64_t * eui, const comms_local_addr_t * local)
{
	int free_slot = -1;

	if (NULL == cache)
	{
		return;
	}

	debugb1("update", eui->data, IEEE_EUI64_LENGTH);

	comms_mutex_acquire(cache->mutex);

	// Look for local address match, delete if different eui, otherwise update, can be only 1
	for (int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
	{
		if (0 == memcmp(local, &(cache->locals[i]), sizeof(comms_local_addr_t)))
		{
			if (0 == memcmp(eui, &(cache->euis[i]), sizeof(ieee_eui64_t)))
			{
				debug("exists");
				cache->updated[i] = osCounterGetSecond();
				comms_mutex_release(cache->mutex);
				return;
			}
			else
			{
				debug("eui change");
				memset(&(cache->euis[i]), 0, sizeof(ieee_eui64_t));
				memset(&(cache->locals[i]), 0, sizeof(comms_local_addr_t));
				cache->updated[i] = 0;
				free_slot = i;
				break;
			}
		}
	}

	// Look for eui address match
	for (int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
	{
		if (0 == memcmp(eui, &(cache->euis[i]), sizeof(ieee_eui64_t)))
		{
			debug("loc change");
			memcpy(&(cache->locals[i]), local, sizeof(comms_local_addr_t));
			cache->updated[i] = osCounterGetSecond();

			comms_mutex_release(cache->mutex);
			return;
		}
	}

	// Look for an empty or oldest entry
	if(free_slot < 0)
	{
		uint32_t updated = UINT32_MAX;
		for (int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
		{
			if (eui64_is_zeros(&(cache->euis[i])))
			{
				free_slot = i;
				break;
			}
			else if(cache->updated[i] < updated)
			{
				updated = cache->updated[i];
				free_slot = i;
			}
		}
	}

	debug("add %d", free_slot);
	// Store the information in the slot that was found
	memcpy(&(cache->euis[free_slot]), eui, sizeof(ieee_eui64_t));
	memcpy(&(cache->locals[free_slot]), local, sizeof(comms_local_addr_t));
	cache->updated[free_slot] = osCounterGetSecond();

	comms_mutex_release(cache->mutex);
}

void comms_cache_init (comms_addr_cache_t * cache)
{
	cache->mutex = comms_mutex_create();
	for(int i=0;i<MIST_COMM_ADDRCACHE_SIZE;i++)
	{
		memset(&(cache->euis[i]), 0, sizeof(ieee_eui64_t));
		memset(&(cache->locals[i]), 0, sizeof(comms_local_addr_t));
		cache->updated[i] = 0;
	}
}
