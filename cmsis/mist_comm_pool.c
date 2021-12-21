/**
 * Message pool for MistComm messages.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#include "mist_comm_pool.h"


comms_error_t comms_pool_init (comms_pool_t * p_pool, int num_messages)
{
    p_pool->pool = osMemoryPoolNew(num_messages, sizeof(comms_msg_t), NULL);
    if (NULL == p_pool->pool)
    {
        return COMMS_ENOMEM;
    }
    return COMMS_SUCCESS;
}


comms_msg_t * comms_pool_get (comms_pool_t * p_pool, uint32_t timeout_ms)
{
    if (UINT32_MAX == timeout_ms)
    {
        return osMemoryPoolAlloc(p_pool->pool, osWaitForever);
    }
    else if (0 == timeout_ms)
    {
        return osMemoryPoolAlloc(p_pool->pool, 0);
    }
    else
    {
        return osMemoryPoolAlloc(p_pool->pool, timeout_ms * 1000 / osKernelGetTickFreq());
    }
}


comms_error_t comms_pool_put (comms_pool_t * p_pool, comms_msg_t * p_msg)
{
    if (osOK == osMemoryPoolFree(p_pool->pool, p_msg))
    {
        return COMMS_SUCCESS;
    }
    return COMMS_FAIL;
}
