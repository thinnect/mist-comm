/**
 * EUI and link-local address discovery for the TinyOS ActiveMessage transport layer.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef MIST_COMM_AM_ADDRDISCO_H_
#define MIST_COMM_AM_ADDRDISCO_H_

#include "mist_comm.h"
#include "mist_comm_iface.h"
#include "mist_comm_addrcache.h"

#ifndef ADDRDISCO_BACKOFF_MS
#define ADDRDISCO_BACKOFF_MS 30000UL
#endif//ADDRDISCO_BACKOFF_MS

typedef struct am_addrdisco am_addrdisco_t;

/**
 * Initialize address discovery and address cache.
 * Should be called only once for a single comms layer.
 *
 * @param comms The comms layer for which to initialize discovery and cache.
 *              Needs to be of the AM type or a derivative.
 * @param disco Memory for the discovery module.
 * @param cache Memory for the cache module.
 */
void comms_am_addrdisco_init (comms_layer_t * comms, am_addrdisco_t * disco, comms_addr_cache_t * cache);

/**
 * Remove from layer and deinitialize address discovery and address cache.
 *
 * @param comms The comms layer from which to remove disco and cache modules.
 */
void comms_am_addrdisco_deinit (comms_layer_t * comms);

/**
 * Request that the EUI be discovered for the local address.
 *
 * @param disco The discovery component to use.
 * @param local The local ActiveMessage address for which the EUI is needed.
 */
void comms_am_addrdisco_discover_eui (am_addrdisco_t * disco, am_addr_t local);

/**
 * Request that the local address be discovered for the EUI.
 *
 * @param disco The discovery component to use.
 * @param local The EUI address for which the local address is needed.
 */
void comms_am_addrdisco_discover_local (am_addrdisco_t * disco, ieee_eui64_t * eui);


// Include implementation details (size info for allocation)
#include "mist_comm_am_addrdisco_private.h"

#endif//MIST_COMM_AM_ADDRDISCO_H_
