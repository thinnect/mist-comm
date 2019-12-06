/**
 * Unit-Tests for the MistComm start-stop functionality.
 *
 * Copyright Thinnect Inc. 2019
 * @license MIT
 */

#include "unity.h"
#include "string.h"

#include "mist_comm.h"
#include "mist_comm_iface.h"

static int m_start_requested = 0;
static int m_stop_requested = 0;
static int m_change_called = 0;
static int m_errors = 0;
static comms_status_t m_status = COMMS_UNINITIALIZED;

static void * m_start_user = NULL;
static void * m_stop_user = NULL;
static comms_status_change_f * m_start_cb = NULL;
static comms_status_change_f * m_stop_cb = NULL;


void setUp(void)
{
}

void tearDown(void)
{
}

comms_error_t comms_dummy_start (comms_layer_iface_t* lyr, comms_status_change_f * cb, void * user)
{
	m_start_requested = true;
	m_start_cb = cb;
	m_start_user = user;
	return COMMS_SUCCESS;
}

comms_error_t comms_dummy_stop (comms_layer_iface_t* lyr, comms_status_change_f * cb, void * user)
{
	m_stop_requested = true;
	m_stop_cb = cb;
	m_stop_user = user;
	return COMMS_SUCCESS;
}

void comms_status_changed (comms_layer_t* comms, comms_status_t status, void* user)
{
	m_change_called++;
	m_status = status;
}

typedef struct mocked_deferred
{
	comms_deferred_f * cb;
	void * arg;
	bool pending;
} mocked_deferred_t;

mocked_deferred_t m_deferred = {NULL, NULL, false};

void _comms_deferred_init(comms_layer_t * comms, void ** deferred, comms_deferred_f * cb)
{
	*deferred = &m_deferred;
	m_deferred.cb = cb;
	m_deferred.arg = comms;
	m_deferred.pending = false;
}

void _comms_defer(void * deferred)
{
	((mocked_deferred_t*)deferred)->pending = true;
}

void _comms_deferred_deinit(void * deferred)
{
	((mocked_deferred_t*)deferred)->pending = false;
	((mocked_deferred_t*)deferred)->arg = NULL;
	((mocked_deferred_t*)deferred)->cb = NULL;
}

void err1(char * s, ...)
{
	m_errors++;
}

void test_SleepControllers()
{
	comms_layer_iface_t base;
	comms_sleep_controller_t scs[3];

	base.start = &comms_dummy_start;
	base.stop = &comms_dummy_stop;
	base.status = COMMS_STOPPED;
	base.sleep_controllers = NULL;
	base.sleep_controller_deferred = NULL;
	base.mutex = NULL;

	TEST_ASSERT_EQUAL_INT(COMMS_STOPPED, comms_status((comms_layer_t*)&base));

	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_register_sleep_controller(
		                                     (comms_layer_t*)&base,
                                             &(scs[0]),
                                             comms_status_changed,
                                             &(scs[0])));

	TEST_ASSERT_EQUAL_INT(COMMS_STOPPED, comms_status((comms_layer_t*)&base));

	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_block(&(scs[0])));

	TEST_ASSERT_EQUAL(1, m_start_requested);
	TEST_ASSERT_NOT_EQUAL(NULL, m_start_cb);

	m_start_cb((comms_layer_t*)&base, COMMS_STARTED, m_start_user);

	TEST_ASSERT_EQUAL(1, m_change_called);
	TEST_ASSERT_EQUAL(COMMS_STARTED, m_status);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTED, comms_status((comms_layer_t*)&base));

	// Add another and stop and start both
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_register_sleep_controller(
	                                     (comms_layer_t*)&base,
                                         &(scs[1]),
                                         comms_status_changed,
                                         &(scs[1])));

	m_change_called = 0;
	TEST_ASSERT_EQUAL_INT(COMMS_ALREADY, comms_sleep_block(&(scs[1])));
	TEST_ASSERT_EQUAL(0, m_change_called);

	m_change_called = 0;
	m_stop_requested = 0;
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_allow(&(scs[0])));
	TEST_ASSERT_EQUAL(0, m_change_called);
	TEST_ASSERT_EQUAL(0, m_stop_requested);

	m_change_called = 0;
	m_stop_requested = false;
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_allow(&(scs[1])));
	TEST_ASSERT_EQUAL(0, m_change_called);
	TEST_ASSERT_EQUAL(1, m_stop_requested);

	m_change_called = 0;
	m_stop_cb((comms_layer_t*)&base, COMMS_STOPPED, m_stop_user);
	TEST_ASSERT_EQUAL(0, m_change_called);
	TEST_ASSERT_EQUAL(COMMS_STOPPED, comms_status((comms_layer_t*)&base));

	m_change_called = 0;
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_block(&(scs[1])));
	TEST_ASSERT_EQUAL(0, m_change_called);

	TEST_ASSERT_EQUAL(1, m_start_requested);
	TEST_ASSERT_NOT_EQUAL(NULL, m_start_cb);

	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_block(&(scs[0])));
	TEST_ASSERT_EQUAL(0, m_change_called);

	TEST_ASSERT_EQUAL(1, m_start_requested); // Should not be called multiple times

	m_start_cb((comms_layer_t*)&base, COMMS_STARTED, m_start_user);
	TEST_ASSERT_EQUAL(2, m_change_called);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTED, comms_status((comms_layer_t*)&base));

	// Stop the current ones, add a third one and start it before the ongoing stop completes
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_register_sleep_controller(
	                                     (comms_layer_t*)&base,
                                         &(scs[2]),
                                         comms_status_changed,
                                         &(scs[2])));

	m_start_requested = 0;
	m_stop_requested = 0;
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_allow(&(scs[0])));
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_allow(&(scs[1])));
	TEST_ASSERT_EQUAL(1, m_stop_requested);

	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_sleep_block(&(scs[2])));
	TEST_ASSERT_EQUAL(0, m_start_requested); // Should not start, as still stopping

	m_change_called = 0;
	m_stop_cb((comms_layer_t*)&base, COMMS_STOPPED, m_stop_user);
	TEST_ASSERT_EQUAL(0, m_change_called);

	TEST_ASSERT_EQUAL(true, m_deferred.pending);
	m_deferred.cb(m_deferred.arg);

	TEST_ASSERT_EQUAL(1, m_start_requested);
	TEST_ASSERT_NOT_EQUAL(NULL, m_start_cb);
	m_start_cb((comms_layer_t*)&base, COMMS_STARTED, m_start_user);

	TEST_ASSERT_EQUAL(1, m_change_called);
	TEST_ASSERT_EQUAL(COMMS_STARTED, m_status);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTED, comms_status((comms_layer_t*)&base));

	// Deregister all controllers, layer should be stopped
	m_stop_requested = 0;
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_deregister_sleep_controller(
	                                     (comms_layer_t*)&base,
                                         &(scs[0])));
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_deregister_sleep_controller(
	                                     (comms_layer_t*)&base,
                                         &(scs[1])));
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_deregister_sleep_controller(
                                         (comms_layer_t*)&base,
                                         &(scs[2])));
	TEST_ASSERT_EQUAL(1, m_stop_requested);
	m_stop_cb((comms_layer_t*)&base, COMMS_STOPPED, m_stop_user);
	TEST_ASSERT_EQUAL_INT(COMMS_STOPPED, comms_status((comms_layer_t*)&base));

	// Make sure we did not pick up any errors during the test
	TEST_ASSERT_EQUAL(0, m_errors);
}
