#ifndef MIST_COMM_BASICS
#define MIST_COMM_BASICS

typedef uint8_t am_group_t;
typedef uint8_t am_id_t;
typedef uint16_t am_addr_t;

typedef struct ieee_eui64 {
	uint8_t data[8];
} ieee_eui64_t;

#endif//MIST_COMM_BASICS
