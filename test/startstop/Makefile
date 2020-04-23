UNITY_ROOT=../zoo/ThrowTheSwitch.Unity

CFLAGS = -std=c99 -g
CFLAGS += -Wall

SRC_FILES  = $(UNITY_ROOT)/src/unity.c test_startstop.c
SRC_FILES += test_runners/TestProductionCode_Runner.c
SRC_FILES += ../../api/mist_comm_api.c
SRC_FILES += ../../mock/mist_comm_mutex.c

INC_DIRS  = -I. -I$(UNITY_ROOT)/src
INC_DIRS += -I../../mock
INC_DIRS += -I..
INC_DIRS += -I../../include
INC_DIRS += -I../../include/compat

INC_DIRS += -I../zoo/thinnect.node-platform/include

TARGET = test_startstop.out

all: clean default

default: $(SRC_FILES)
	gcc $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET)
	- ./$(TARGET)

test_runners/TestProductionCode_Runner.c: test_startstop.c
	mkdir -p test_runners
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_startstop.c test_runners/TestProductionCode_Runner.c

clean:
	rm -f $(TARGET)
