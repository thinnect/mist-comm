/**
 * MistComm mock radio layer.
 * Exports outgoing messages, giving send done events to users.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef MIST_MOCK_RADIO_CMSIS_H
#define MIST_MOCK_RADIO_CMSIS_H

#include "mist_comm_am.h"

/**
 * Create a mock radio.
 * @param address - radio address.
 * @param send_copy - function to call with outgoing messages.
 * @return Mock radio pointer.
 */
comms_layer_t * mist_mock_cmsis_radio_init (am_addr_t address, comms_send_f * send_copy);

#endif//MIST_MOCK_RADIO_CMSIS_H
