#include <stdio.h>
#include "pulse.h"

int DEBUG_pulse = 0;

void pulse_module_load_cb (pa_context *c, uint32_t idx, void *userdata)
{
	pulse_con_t *p;
	p = (pulse_con_t*)userdata;

	p->module_idx = (int)idx;
	if (DEBUG_pulse)
		printf ("pulse: Loaded module %d\n", (int)idx);

	pa_threaded_mainloop_signal (p->mainloop, 0);
}

void pulse_module_unload_cb (pa_context *c, int success, void *userdata)
{
	pulse_con_t *p;
	p = (pulse_con_t*)userdata;

	if (DEBUG_pulse)
	{
		if (success)
			printf ("pulse: Unloaded module %d\n", p->module_idx);
		else
			printf ("pulse: Failed to unload module %d\n", p->module_idx);
	}

	pa_threaded_mainloop_signal (p->mainloop, 0);
}

void pulse_context_state_cb (pa_context *context, void *userdata)
{
	pulse_con_t *p;
	char socket[1031]; /* 1024 + 7 for "socket=" */

	p = (pulse_con_t*)userdata;

	switch (pa_context_get_state(context)) {
	case PA_CONTEXT_READY:
		if (DEBUG_pulse)
			printf ("Connection is up, loading module\n");
		snprintf (socket, 1031, "socket=%s", p->socket);
		pa_context_load_module (
			context,
			"module-native-protocol-unix",
			socket,
			pulse_module_load_cb,
			p);
		break;
	case PA_CONTEXT_CONNECTING:
		if (DEBUG_pulse)
			printf ("pulse: Connecting\n");
		break;
	case PA_CONTEXT_AUTHORIZING:
		if (DEBUG_pulse)
			printf ("pulse: Authorizing\n");
		break;
	case PA_CONTEXT_SETTING_NAME:
		if (DEBUG_pulse)
			printf ("pulse: Setting name\n");
		break;
	case PA_CONTEXT_UNCONNECTED:
		if (DEBUG_pulse)
			printf ("pulse: Unconnected\n");
		break;
	case PA_CONTEXT_FAILED:
		if (DEBUG_pulse)
			printf ("pulse: Failed\n");
		break;
	case PA_CONTEXT_TERMINATED:
		if (DEBUG_pulse)
			printf ("pulse: Terminated\n");
		break;
	}
}

void pulse_startup (pulse_con_t *p, char * socket)
{
	/* Clear struct */
	p->mainloop = 0;
	p->api = 0;
	p->context = 0;
	p->socket = socket;
	p->module_idx = -1;

	/* Create mainloop */
	p->mainloop = pa_threaded_mainloop_new ();
	pa_threaded_mainloop_start (p->mainloop);

	if ( !p->mainloop ) {
		fprintf (stderr, "Failed to create pulse mainloop->\n");
		exit (1);
	}

	/* Set up connection to pulse server */
	pa_threaded_mainloop_lock (p->mainloop);
	p->api = pa_threaded_mainloop_get_api (p->mainloop);
	p->context = pa_context_new (p->api, "pulsetest");
	pa_context_set_state_callback (p->context, pulse_context_state_cb, p);

	int err = pa_context_connect (
		p->context,        /* context */
		NULL,              /* default server */ 
		PA_CONTEXT_NOFAIL, /* keep reconnection on failure */
		NULL );            /* use default spawn api */

	if (err != 0) {
		fprintf (stderr, "Pulse error: %s\n",pa_strerror(err));
		exit (1);
	}
	pa_threaded_mainloop_unlock (p->mainloop);
}

void pulse_teardown (pulse_con_t *p)
{
	/* Unload module if module loaded successfully on startup */
	if ( p->module_idx != -1)
	{
		pa_threaded_mainloop_lock (p->mainloop);
		pa_context_unload_module (
			p->context,
			p->module_idx,
			pulse_module_unload_cb,
			p);
		pa_threaded_mainloop_wait (p->mainloop);
		pa_threaded_mainloop_unlock (p->mainloop);
	}

	/* Release pulse */
	pa_threaded_mainloop_lock (p->mainloop);
	pa_context_disconnect (p->context);
	pa_context_unref (p->context);
	pa_threaded_mainloop_unlock (p->mainloop);

	pa_threaded_mainloop_stop (p->mainloop);
	pa_threaded_mainloop_free (p->mainloop);

	p->mainloop = 0;
	p->context = 0;
	p->api = 0; /* destroyed with mainloop */
	p->module_idx = -1;
	p->socket = 0;

	if (DEBUG_pulse)
		printf ("pulse: Teardown complete\n");
}