/**
 * Mock message pool for MistComm messages.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#include "mist_comm_pool.h"

#include <stdlib.h>

comms_error_t comms_pool_init (comms_pool_t * p_pool, int num_messages)
{
    return COMMS_SUCCESS;
}


comms_msg_t * comms_pool_get (comms_pool_t * p_pool, uint32_t timeout_ms)
{
    return malloc(sizeof(comms_msg_t));
}


comms_error_t comms_pool_put (comms_pool_t * p_pool, comms_msg_t * p_msg)
{
    free(p_msg);
    return COMMS_SUCCESS;
}
