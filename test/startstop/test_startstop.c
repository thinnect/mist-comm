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

static bool m_start_requested = false;
static bool m_stop_requested = false;
static bool m_started_called = false;
static bool m_stopped_called = false;
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

void comms_start_done (comms_layer_t* comms, comms_status_t status, void* user)
{
	m_started_called = true;
	m_status = status;
}

void comms_stop_done (comms_layer_t* comms, comms_status_t status, void* user)
{
	m_stopped_called = true;
	m_status = status;
}

void test_StartAndStop()
{
	comms_layer_iface_t base;

	base.start = &comms_dummy_start;
	base.stop = &comms_dummy_stop;

	base.status_change_user_cb = NULL;
	base.controller_mutex = comms_mutex_create();
	base.start_stop_mutex = comms_mutex_create();
	base.receiver_mutex = comms_mutex_create();

	// Test starting
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_start((comms_layer_t*)&base, &comms_start_done, NULL));

	TEST_ASSERT_EQUAL(true, m_start_requested);
	TEST_ASSERT_EQUAL(false, m_stop_requested);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTING, comms_status((comms_layer_t*)&base));

	m_start_cb((comms_layer_t*)&base, COMMS_STARTED, m_start_user);

	TEST_ASSERT_EQUAL(true, m_started_called);
	TEST_ASSERT_EQUAL(false, m_stopped_called);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTED, m_status);
	TEST_ASSERT_EQUAL_INT(COMMS_STARTED, comms_status((comms_layer_t*)&base));
	TEST_ASSERT_NOT_EQUAL(NULL, m_start_cb);

	// Reset flags
	m_start_cb = NULL;
	m_stop_cb = NULL;
	m_start_requested = false;
	m_stop_requested = false;
	m_started_called = false;
	m_stopped_called = false;

	// Test stopping
	TEST_ASSERT_EQUAL_INT(COMMS_SUCCESS, comms_stop((comms_layer_t*)&base, &comms_stop_done, NULL));

	TEST_ASSERT_EQUAL(false, m_start_requested);
	TEST_ASSERT_EQUAL(true, m_stop_requested);
	TEST_ASSERT_EQUAL_INT(COMMS_STOPPING, comms_status((comms_layer_t*)&base));
	TEST_ASSERT_NOT_EQUAL(NULL, m_stop_cb);

	m_stop_cb((comms_layer_t*)&base, COMMS_STOPPED, m_stop_user);

	TEST_ASSERT_EQUAL(false, m_started_called);
	TEST_ASSERT_EQUAL(true, m_stopped_called);
	TEST_ASSERT_EQUAL_INT(COMMS_STOPPED, m_status);
	TEST_ASSERT_EQUAL_INT(COMMS_STOPPED, comms_status((comms_layer_t*)&base));
}
