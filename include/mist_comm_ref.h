/**
 * Mist communications reference receiver manager.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_REF_H
#define MIST_COMM_REF_H

/**
 * Add default receiver handling functionality to a layer implementation.
 *
 * @param comms Pointer to a comms layer.
 * @return      COMMS_SUCCESS if no errors.
 */
comms_error_t comms_initialize_rcvr_management (comms_layer_iface_t * comms);

/**
 * Deliver a message to registered receivers.
 *
 * @param comms Pointer to a comms layer.
 * @param msg   The message to deliver.
 * @return      COMMS_SUCCESS or COMMS_NO_ADDR most often.
 */
comms_error_t comms_basic_deliver (comms_layer_t * comms, comms_msg_t * msg);

#endif//MIST_COMM_REF_H
