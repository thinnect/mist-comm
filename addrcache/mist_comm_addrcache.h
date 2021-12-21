/**
 * EUI64<->local address cache.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_ADDRCACHE_H_
#define MIST_COMM_ADDRCACHE_H_

#include "mist_comm.h"

#ifndef MIST_COMM_ADDRCACHE_SIZE
#define MIST_COMM_ADDRCACHE_SIZE 32
#endif//MIST_COMM_ADDRCACHE_SIZE

typedef struct comms_addr_cache comms_addr_cache_t;

/**
 * Initialize the cache structure.
 */
void comms_cache_init(comms_addr_cache_t * cache);

/**
 * Get the local address for the EUI.
 *
 * @param cache Pointer to the cache.
 * @param eui EUI64 address.
 * @param local Storage space for the local address.
 * @return true if an address was available.
 */
bool comms_cache_get_local(comms_addr_cache_t * cache, const ieee_eui64_t * eui, comms_local_addr_t * local);

/**
 * Get the EUI for the local address.
 *
 * @param cache Pointer to the cache.
 * @param local Local address.
 * @param eui Storage space for the EUI64 address.
 * @return true if an address was available.
 */
bool comms_cache_get_eui(comms_addr_cache_t * cache, const comms_local_addr_t * local, ieee_eui64_t * eui);

/**
 * Update the cached mapping.
 *
 * Only 1:1 possible, any conflicting mappings will get thrown out.
 *
 * @param cache Pointer to the cache.
 * @param eui EUI64 address.
 * @param local Local address.
 */
void comms_cache_update(comms_addr_cache_t * cache, const ieee_eui64_t * eui, const comms_local_addr_t * local);


//------------------------------------------------------------------------------
struct comms_addr_cache
{
	commsMutexId_t mutex;
	comms_local_addr_t locals[MIST_COMM_ADDRCACHE_SIZE];
	ieee_eui64_t euis[MIST_COMM_ADDRCACHE_SIZE];
	uint32_t updated[MIST_COMM_ADDRCACHE_SIZE];
};

#endif//MIST_COMM_ADDRCACHE_H_
