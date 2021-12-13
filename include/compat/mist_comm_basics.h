/**
 * Reference basic variables definition.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#ifndef MIST_COMM_BASICS_H
#define MIST_COMM_BASICS_H

#include <stdint.h>

#include "eui64.h"

typedef uint8_t am_group_t; // This is actually PAN
typedef uint8_t am_id_t;
typedef uint16_t am_addr_t;

typedef uint16_t nx_am_addr_t;

#define AM_BROADCAST_ADDR ((uint16_t)0xFFFF)

#endif//MIST_COMM_BASICS_H
