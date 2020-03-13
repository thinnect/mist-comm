/**
 * Sleep controller implementation.
 *
 * Copyright Thinnect Inc. 2019
 * @author Raido Pahtma
 * @license MIT
 */

#include "mist_comm.h"
#include "mist_comm_iface.h"

#include "loglevels.h"
#define __MODUUL__ "mctrl"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_comm_controller & BASE_LOG_LEVEL)
#include "log.h"

static bool unsafe_update_state(comms_layer_t * comms);
static bool unsafe_service_callbacks(comms_layer_t * comms, comms_status_t status);

static void status_change_callback(comms_layer_t * comms, comms_status_t status, void * user)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	bool ok;
	comms_mutex_acquire(cl->controller_mutex);
	ok = unsafe_service_callbacks(comms, status);
	comms_mutex_release(cl->controller_mutex);
	if (!ok)
	{
		_comms_defer(cl->sleep_controller_deferred);
	}
}

static void deferred_callback(void * arg)
{
	comms_layer_t * comms = (comms_layer_t*)arg;
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	bool ok;
	comms_mutex_acquire(cl->controller_mutex);
	ok = unsafe_update_state(comms);
	comms_mutex_release(cl->controller_mutex);
	if (!ok)
	{
		_comms_defer(cl->sleep_controller_deferred);
	}
}

/**
 * Cannot defer here, as mutex must be released.
 *
 * @return true if current state is good, false if state needs to be changed.
 */
static bool unsafe_service_callbacks(comms_layer_t * comms, comms_status_t status)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	comms_sleep_controller_t** indirect;
	for (indirect=&(cl->sleep_controllers); NULL != *indirect; indirect = &((*indirect)->next))
	{
		if ((*indirect)->pending)
		{
			if (COMMS_STARTED == status)
			{
				(*indirect)->cb(comms, status, (*indirect)->user);
				(*indirect)->pending = false;
			}
			else
			{
				return false;
			}
		}
		if ((*indirect)->block)
		{
			if (COMMS_STOPPED == status)
			{
				return false;
			}
		}
	}
	return true;
}

/**
 * Cannot defer here, as mutex must be released.
 *
 * @return true if current state is good, false if state needs to be changed.
 */
static bool unsafe_update_state(comms_layer_t * comms)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	comms_status_t status = comms_status(comms);
	comms_sleep_controller_t ** indirect;
	for (indirect=&(cl->sleep_controllers); NULL != *indirect; indirect = &((*indirect)->next))
	{
		if ((*indirect)->block) // Somebody wants it to be ON
		{
			if (COMMS_STOPPED == status)
			{
				if (COMMS_SUCCESS != comms_start(comms, &status_change_callback, NULL))
				{
					err1("start"); // Not really expecting resistance here, try again?
					return false;
				}
			}
			return true; // Can make decision based on first block encountered
		}
	}

	// Nobody actually wants it to be ON, so turn it OFF
	if (COMMS_SUCCESS != comms_stop(comms, &status_change_callback, NULL))
	{
		err1("stop"); // Not really expecting resistance here, try again?
		return false;
	}

	return true;
}

comms_error_t comms_register_sleep_controller(comms_layer_t * comms, comms_sleep_controller_t * ctrl,
                                              comms_status_change_f * start_done, void * user)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	comms_error_t err = COMMS_SUCCESS;
	comms_sleep_controller_t** indirect;

	comms_mutex_acquire(cl->controller_mutex);

	if (NULL == cl->sleep_controller_deferred)
	{
		_comms_deferred_init(comms, &(cl->sleep_controller_deferred), deferred_callback);
	}

	for (indirect=&(cl->sleep_controllers); NULL != *indirect; indirect = &((*indirect)->next))
	{
		if (*indirect == ctrl)
		{
			err = COMMS_ALREADY;
			break;
		}
	}

	if (COMMS_SUCCESS == err)
	{
		*indirect = ctrl;

		ctrl->comms = comms;
		ctrl->block = false;
		ctrl->pending = false;
		ctrl->user = user;
		ctrl->cb = start_done;
		ctrl->next = NULL;
	}

	comms_mutex_release(cl->controller_mutex);

	return err;
}

comms_error_t comms_deregister_sleep_controller(comms_layer_t * comms, comms_sleep_controller_t * ctrl)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)comms;
	comms_error_t err = COMMS_FAIL;
	comms_sleep_controller_t** indirect;

	comms_mutex_acquire(cl->controller_mutex);

	for(indirect=&(cl->sleep_controllers); NULL != *indirect; indirect = &((*indirect)->next))
	{
		if(*indirect == ctrl)
		{
			*indirect = ctrl->next;

			ctrl->comms = NULL;
			err = COMMS_SUCCESS;
			if (true != unsafe_update_state(comms))
			{
				err1("panic");
				// TODO actual panic
			}
			break;
		}
	}

	comms_mutex_release(cl->controller_mutex);

	return err;
}

// Block may be asynchronous, wakeup may take time, or EALREADY may be returned
comms_error_t comms_sleep_block(comms_sleep_controller_t * ctrl)
{
	comms_error_t err = COMMS_EINVAL;
	if ((NULL != ctrl)&&(NULL != ctrl->comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)(ctrl->comms);
		comms_mutex_acquire(cl->controller_mutex);

		if(ctrl->pending)
		{
			err = COMMS_EBUSY;
		}
		else if (true == ctrl->block)
		{
			err = COMMS_ALREADY;
		}
		else
		{
			ctrl->block = true;
			if (COMMS_STARTED == comms_status(ctrl->comms))
			{
				err = COMMS_ALREADY;
			}
			else
			{
				err = COMMS_SUCCESS;
				ctrl->pending = true;
				if (true != unsafe_update_state(ctrl->comms))
				{
					ctrl->pending = false;
					err = COMMS_ERETRY;
				}
			}
		}

		comms_mutex_release(cl->controller_mutex);
	}
	return err;
}

// Allow is synchronous, sleep is not guaranteed, but block is released immediately
comms_error_t comms_sleep_allow(comms_sleep_controller_t * ctrl)
{
	comms_error_t err = COMMS_EINVAL;
	if ((NULL != ctrl)&&(NULL != ctrl->comms))
	{
		comms_layer_iface_t * cl = (comms_layer_iface_t*)(ctrl->comms);
		comms_mutex_acquire(cl->controller_mutex);

		if(false == ctrl->block)
		{
			err = COMMS_ALREADY;
		}
		else
		{
			bool pending = ctrl->pending;
			err = COMMS_SUCCESS;
			ctrl->block = false;
			ctrl->pending = false;
			if (true != unsafe_update_state(ctrl->comms))
			{
				ctrl->block = true;
				ctrl->pending = pending;
				err = COMMS_ERETRY;
			}
		}

		comms_mutex_release(cl->controller_mutex);
	}
	return err;
}

bool comms_sleep_blocked(comms_sleep_controller_t * ctrl)
{
	comms_layer_iface_t * cl = (comms_layer_iface_t*)(ctrl->comms);
	bool blocked;
	comms_mutex_acquire(cl->controller_mutex);
	blocked = ctrl->block;
	comms_mutex_release(cl->controller_mutex);
	return blocked;
}
