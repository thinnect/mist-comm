/**
 * Message pool for MistComm messages.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_POOL_H
#define MIST_COMM_POOL_H

#include "mist_comm.h"

#include "mist_comm_pool_struct.h" // Include a platform-specific structure for the pool definition

/**
 * Initialize a message pool.
 * This will usually allocate a pool using the underlying OS heap.
 *
 * @param pool Pointer to a message pool that is to be initialized.
 * @param num_messages Number of messages to be allocated.
 * @return COMMS_SUCCESS or an error in case of failure.
 */
comms_error_t comms_pool_init (comms_pool_t * pool,  int num_messages);

/**
 * Get a message from the pool.
 *
 * @param pool Pointer to a message pool.
 * @param timeout_ms How long to wait for a message to become available, UINT32_MAX to wait forever.
 * @return message pointer or NULL when a message was not available within the allowed timeout.
 */
comms_msg_t * comms_pool_get (comms_pool_t * pool, uint32_t timeout_ms);

/**
 * Return a message to the pool.
 *
 * @param pool Pointer to a message pool.
 * @param p_msg Pointer to a message to be returned.
 * @return COMMS_SUCCESS or an error, if the pool is full.
 */
comms_error_t comms_pool_put (comms_pool_t * pool, comms_msg_t * p_msg);

#endif//MIST_COMM_POOL_H
