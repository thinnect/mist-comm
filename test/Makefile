# Makefile

CFLAGS = -I. -I../mock -I../api -I../include -I../include/compat -std=c99 -Wall

all:
	gcc ${CFLAGS} main.c ../mock/mist_comm_mutex.c ../api/mist_comm_api.c ../api/mist_comm_rcv.c ../am/mist_comm_am.c -o test
