/**
 * Routing info callback handlers for the MistComm API.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */

#include "mist_comm_routing.h"
#include <string.h>

comms_error_t comms_routing_register_result_callback (
    comms_layer_am_mh_t* comms,
    comms_routing_info_handler_t* rih,
    comms_am_routed_f* func, void* user)
{
    comms_routing_info_handler_t** indirect;
    for (indirect=&(comms->routing_info_handlers); NULL != *indirect; indirect = &((*indirect)->next))
    {
        if (*indirect == rih)
        {
            return COMMS_FAIL;
        }
    }
    *indirect = rih;

    rih->callback = func;
    rih->user = user;
    rih->next = NULL;

    return COMMS_SUCCESS;
}

comms_error_t comms_routing_deregister_result_callback (
    comms_layer_am_mh_t* comms,
    comms_routing_info_handler_t* rih)
{
    comms_routing_info_handler_t** indirect;
    for (indirect=&(comms->routing_info_handlers); NULL != *indirect; indirect = &((*indirect)->next))
    {
        if (*indirect == rih)
        {
            *indirect = rih->next;
            return COMMS_SUCCESS;
        }
    }
    return COMMS_FAIL;
}

void comms_routing_notify_routed (comms_layer_t* comms, am_addr_t dest, uint16_t cost, comms_error_t status)
{
    comms_layer_am_mh_t * cl = (comms_layer_am_mh_t*)comms;
    comms_routing_info_handler_t * rih;

    for (rih=cl->routing_info_handlers; NULL != rih; rih=rih->next)
    {
        rih->callback(comms, dest, cost, status, rih->user);
    }
}

comms_error_t comms_init_routing_result_callbacks (comms_layer_am_mh_t * comms)
{
    comms->routing_info_handlers = NULL;
    return COMMS_SUCCESS;
}
