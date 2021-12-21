/**
 * MistComm mock radio layer.
 * Exports outgoing messages, giving send done events to users.
 *
 * Copyright Thinnect Inc. 2021
 * @license MIT
 */
#include "mist_comm_am.h"

#include "mist_mock_radio_cmsis.h"

#include "cmsis_os2.h"

#define RADIO_TOS_MAX_PAYLOAD_LENGTH 114

static comms_layer_am_t m_radio_iface;

static osThreadId_t m_thread;
static osMutexId_t m_mutex;

static bool m_send_busy = false;
static comms_msg_t * mp_send_msg;
static comms_send_done_f * mf_send_done;
static void * mp_send_user;

static comms_send_f * mf_send_copy;


static comms_error_t radio_send (comms_layer_iface_t * iface, comms_msg_t * msg,
                                 comms_send_done_f * send_done, void * user)
{
	while (osOK != osMutexAcquire(m_mutex, osWaitForever));
	if (false == m_send_busy)
	{
		m_send_busy = true;
		mp_send_msg = msg;
		mf_send_done = send_done;
		mp_send_user = user;
		osThreadFlagsSet(m_thread, 1);
		osMutexRelease(m_mutex);
		return COMMS_SUCCESS;
	}
	osMutexRelease(m_mutex);
	return COMMS_EBUSY;
}


static void mock_radio_loop (void * arg)
{
	for (;;)
	{
		osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
		while (osOK != osMutexAcquire(m_mutex, osWaitForever));
		if (m_send_busy)
		{
			m_send_busy = false;
			mf_send_copy((comms_layer_iface_t *)&m_radio_iface, mp_send_msg, NULL, mp_send_user);
			mf_send_done((comms_layer_t *)&m_radio_iface, mp_send_msg, COMMS_SUCCESS, mp_send_user);
		}
		osMutexRelease(m_mutex);
	}
}

static uint8_t radio_max_length (comms_layer_iface_t * iface)
{
    return RADIO_TOS_MAX_PAYLOAD_LENGTH;
}

comms_layer_t * mist_mock_cmsis_radio_init (am_addr_t address, comms_send_f * send_copy)
{
	mf_send_copy = send_copy;

	comms_am_create((comms_layer_t *)&m_radio_iface, address, radio_send, radio_max_length, NULL, NULL);

	m_mutex = osMutexNew(NULL);

    const osThreadAttr_t thread_attr = { .name = "radio", .stack_size = 3072 };
    m_thread = osThreadNew(mock_radio_loop, NULL, &thread_attr);

	return (comms_layer_t*)&m_radio_iface;
}
