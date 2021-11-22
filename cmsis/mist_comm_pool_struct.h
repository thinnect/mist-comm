/**
 * Message pool for MistComm messages on CMSIS OS2.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_POOL_STRUCT_H
#define MIST_COMM_POOL_STRUCT_H

#include "cmsis_os2.h"

typedef struct comms_pool {
	osMemoryPoolId_t pool;
} comms_pool_t;

#endif//MIST_COMM_POOL_STRUCT_H
