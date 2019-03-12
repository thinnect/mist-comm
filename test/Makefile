# Makefile

CFLAGS = -I. -I../api -I../include -I../include/compat -std=c99

all:
	gcc ${CFLAGS} main.c ../api/mist_comm_api.c ../api/mist_comm_rcv.c ../am/mist_comm_am.c -o test
