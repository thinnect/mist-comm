#ifndef MIST_COMM_BASICS
#define MIST_COMM_BASICS

#include <stdint.h>

typedef uint8_t am_group_t; // This is actually PAN
typedef uint8_t am_id_t;
typedef uint16_t am_addr_t;

typedef struct ieee_eui64 {
	uint8_t data[8];
} ieee_eui64_t;
// ??? this should come from some ieee_eui64.h header

#define AM_BROADCAST_ADDR ((uint16_t)0xFFFF)

#endif//MIST_COMM_BASICS
