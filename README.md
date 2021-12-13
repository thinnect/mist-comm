# mist-comm

Mist communications APIs, abstractions for various communication interfaces.

# Features

The Mist Communication API abstracts the specifics of different communication
implementations from the higher layers. Applications can use
a consistent globally unique EUI-64 based addressing scheme and common functions
for sending and receiving messages, regardless of the underlying communication
technology. This makes it possible to replace the underlying communication
interface without making any changes to the application code. All API calls
are done through methods that include a pointer to the API layer and for basic
communication flows, it is usually not necessary to be aware of what the actual
underlying communications technology is.


# Included implementations

The mist-comm library contains an adapter for adding the mist-comm API to
radio layers. This is used by the 802.15.4 implementations in
[node-platform](https://github.com/thinnect/node-platform)
and by the `serial_activemessage` and `serial_basicmessage` implementations
bundled with the library.

The API has been intended to be platform independent, however, in practice
a CMSIS2 (cmsis_os2.h) environment is currently required for running the API in
most cases, since no other OS wrappers have been developed. It has, however,
also not been decided to exclusively rely on CMSIS2 and it should be possible to
set up a basic communications flow without an OS using for example
[radio_basic.c](https://github.com/thinnect/node-platform/blob/master/silabs/radio_basic.c).


# Addresses and identifiers

## Addressing

Messages are ideally addressed using the endpoint IEEE EUI-64 addresses, however,
the actual address structure is a composite of a global and a local address,
since the mist-comm API is mostly used on embedded devices and over communication
interfaces where it is necessary to map interface-specific addresses and global
identities.

## Packet type - Active Message ID (AMID)

The Mist AMID concept derives from the TinyOS Active Message ID, which is an
8-bit value. On the cloud side this has been extended to include the
TinyOS message identifier 0x3F as a prefix, with the goal of opening up the rest
of the 16-bit space in the future, as other protocols and tools are extended.
The mist-comm API currently uses the 8-bit value, a future version may move to
the full 16-bit identifier.

## Packet group - Active Message Group / PAN ID

The Mist Group ID concept derives from the TinyOS Active Message Group ID,
which itself relies on the 802.15.4 PAN ID, however, exposes only 8-bits of the
original 16-bit value. The mist-comm API uses the 16-bit value, however, the
high-byte currently needs to be 0, because the serial protocol only transfers
the lower 8 bits.


# Setting up a communications layer

## Initialization

Initializing a communications layer is usually a simple as calling the init or
setup function of a specific communications layer. A pointer to the layer is
returned or NULL if something went wrong. This is usually done once on startup,
allocates some resources and probably starts a background thread. Some layers
may allow de-initialization and re-initialization, but many implementations
lack support for this and subsequent calls to the setup function will fail.

For example:
```C
comms_layer_t * p_radio = radio_setup(node_local_address, node_eui64);
if (NULL == radio)
{
	// panic
}

// START the radio or apply a sleep-block
```

The communications layer may start automatically or may require `comms_start`
to be called or a sleep-block to be applied to actually start it. `comms_status`
can be used to check the status.


## Creation

The mist-comm interface structure (`comms_layer_iface_t`) carries function
pointers for packet manipulation and messaging tasks. These functions are
accessed by the respective API functions. It is possible to create
a completely custom implementation, however, an adapter component exists for
common ActiveMessage layers (used by most 802.15.4 radio and UART implementations).

`comms_am_create` initializes a layer structure with most necessary ActiveMessage
functions, only requiring the send and max packet length functions. Also
start and stop may be added, but are not needed for always-on implementations.

Basic receiver management functionality can be added to a layer by calling
`comms_initialize_rcvr_management`, `comms_am_create` does this internally
already!

When creating a new communications layer, it is a good idea to call `comms_verify_api`
first and check that it returns `true`. This will allow compile-time detection
of some incompatibilities, if static communications libraries are being linked
together.


# Sending a message

Before using a `comms_msg_t` structure, it must be initialized for the layer
that it is going to be used with. This will clear all the contents and may
set some fields in the message necessary for the communications layer. Messages
may not be passed from one layer to another directly.

```C
static comms_msg_t m_msg;
comms_init_message(p_radio, &m_msg); // A message must be initialized

comms_address_t dest; // Send message to 1122334455667788
memset(&dest, 0, sizeof(dest)); // Clear the structure
eui64_set(&dest.eui, "\x11\x22\x33\x44\x55\x66\x77\x88")

comms_set_destination(p_radio, &m_msg, &dest);

custom_packet_t * pcp = (custom_packet_t*)comms_get_payload(p_radio, &m_msg, sizeof(custom_packet_t));
if (NULL != pcp)
{
	pcp->field1 = 1;
	pcp->field2 = 2;

	comms_set_packet_type(p_radio, &m_msg, CUSTOM_PROTOCOL_AMID);
	comms_set_payload_length(p_radio, &m_msg, sizeof(custom_packet_t));

	comms_set_ack_required(p_radio, &m_msg, true); // Request an ack

	comms_error_t result = comms_send(radio, &m_msg, radio_send_done, NULL);
	if (COMMS_SUCCESS == result)
	{
		sending = true; // wait for radio_send_done to be called
	}
}
```

If `comms_send` returns COMMS_SUCCESS, the communications layer takes possession
of the message until it is returned with the send-done event.
*The message structure must not be modified by the user between send and send-done!*

When sending a message, some fields, like the source address or group will default
to the values configured for the communications layer and do not normally need
setting. Some layers may not even support setting these fields to those of other
devices, but they do need to be set when for example bridging / routing messages.

```C
static void radio_send_done (comms_layer_t * comms, comms_msg_t * msg, comms_error_t result, void * user)
{
	// Result can be checked from result
	if (COMMS_SUCCESS != result)
	{
		// Something went wrong
	}
	// User carries what-ever pointer was passed as the final argument to comms_send ... NULL in this example
	if (comms_ack_received(comms, msg))
	{
		// Ack was received
	}

	// The message can now be freed, put back in a pool or re-used (but not directly from send-done)

	// Signal your thread perhaps or clear the sending flag, for example:
	osThreadFlagsSet(my_thread_id, MY_THREAD_FLAG_MESSAGE_SENT);
}
```

The send-done event should be treated like an interrupt - don't do too much in there
as it is usually fired from the communication layer thread and holding it up may
cause subsequent packets to be dropped.


# Receiving a message

Receiving a message requires registering a receiver and a receive function.
A regular receiver will pass a message to the receive function when the type (AM ID)
of the message matches and the message destination is either the address of the
node or the broadcast address. Alternatively a snooper can be registered with
`comms_register_snooper`, which will pass all messages to the receive callback
regardless of type or destination address. Multiple receivers or snoopers may be
registered and each will get a copy of the message, if it matches the parameters.
The same receive function may be used for multiple receivers, but a receiver
structure can obly be registered once, unless de-registered first!

```C
static comms_layer_t * mp_comms;
static comms_receiver_t m_rcvr1;
static comms_receiver_t m_rcvr2;

void setup_function (comms_layer_t * p_comms)
{
	mp_comms = p_comms;
	comms_register_recv_eui(mp_comms, &m_rcvr1, receive_message, NULL, AMID_PROTOCOL_1);
	comms_register_recv(mp_comms, &m_rcvr2, receive_message, NULL, AMID_PROTOCOL_2);
}

// Same function given to both receivers
static void receive_message (comms_layer_t * p_comms, const comms_msg_t * p_msg, void * p_user)
{
	am_id_t ptype = comms_get_packet_type(p_comms, msg);
	if (AMID_PROTOCOL_1 == ptype)
	{
		comms_address_t source;
		comms_get_source(p_comms, p_msg, &source);
		if (eui64_is_zeros(&source.eui))
		{
			return; // Don't know source EUI64
		}
		// packet processing here
	}
	else if (AMID_PROTOCOL_2 == ptype)
	{
		// packet processing where EUI64 of source is not important
	}
}

```


# Addressing

The message sending example above uses global EUI64 addressing when sending the
message. In this case, the sender is starting with a blank `comms_address_t`
structure and knows the EUI64 for the destination. Alternatively a sender may
use a `comms_address_t` saved from a received message, which would already
contain the local address (if one is used). Sending without a local address
(when one is needed) will trigger a lookup from the address mapping table.
If the address is not in the table, then a discovery packet will be sent,
however, sending of the initial message will fail with `COMMS_NO_ADDR`.

The message reception example above for the first message type sets up a receiver
through the `comms_register_recv_eui` function, meaning it will automatically
try to determine the EUI64 for all incoming messages, however, messages will be
delivered to the receive function even if the EUI64 is not known. It is therefore
important to always check the source address, if the source of the message is
important. The example for the second message type uses `comms_register_recv`,
which means messages of that type will not trigger an EUI64 lookup.

Address discovery and caching needs to be set up for each communications layer
that needs it.

```C
    static am_addrdisco_t disco;
    static comms_addr_cache_t cache;
    comms_am_addrdisco_init(p_radio, &disco, &cache); // example for mist_comm_am
```


# Message pool

An allocated `comms_msg_t` structure is necessary for sending a message. In many
cases components can declare the message statically, but for many other components,
that rarely need to send a message, declaring a static message may be wasteful
in terms of overall memory usage. A message pool component is available for
cases where it is desirable to share the message structures among several components.
The message pool needs to be initialized, it normally allocates the message memory
using underlying OS memory allocation methods.

```C
static comms_pool_t m_msg_pool;

void setup_function (void)
{
	if (COMMS_SUCCESS != comms_pool_init(&m_msg_pool, 3)) // allocate 3 messages
	{
		// panic
	}
	custom_component_init(mp_comms, &m_msg_pool);
}

// custom_component.c ---------------------------------------------------------

static comms_pool_t * mp_pool;
static comms_layer_t * mp_comms;

static void send_done (comms_layer_t * comms, comms_msg_t * p_msg, comms_error_t result, void * user)
{
	// signal something
	comms_pool_put(mp_pool, p_msg); // Message not sent, put it back to pool
}

void custom_component_function (void)
{
	comms_msg_t * p_msg = comms_pool_get(mp_pool, 0); // no wait
	if (NULL != p_msg)
	{
		// format message, add payload ...

		comms_error_t result = comms_send(mp_comms, p_msg, send_done, NULL);
		if (COMMS_SUCCESS != result)
		{
			comms_pool_put(mp_pool, p_msg); // Message not sent, put it back to pool!
		}
	}
}
```


# Communications layer sleep management

A communications layer may be always on (started after init) or switched either
by calling `comms_start` and `comms_stop` or through sleep controllers.
Sleep controllers control the layer using the start and stop functions so they
do not work together with direct external control. Therefore, it is recommended
to use sleep controllers if the communications layer needs to be switched
on from more than one place - for example to keep an exteremely low duty-cycle
device out of sleep for a slightly longer session.

Sleep controllers need to be registered with a communications layer first and
then a sleep block may be applied. Starting a communications layer is not
instantaneous, so a callback is fired, if it was not running and will be started.
Allowing sleep, however, does not mean that the communications layer will
actually stop, since another component may be holding a block as well.

```C
static comms_sleep_controller_t m_sleep_ctrl;

// Add a sleep cotnroller
comms_register_sleep_controller(p_comms, &m_sleep_ctrl, radio_start_done, NULL);

// Wake up a communications layer
	comms_error_t rslt = comms_sleep_block(&m_sleep_ctrl);
	if (rslt == COMMS_SUCCESS)
	{
		// radio_start_done will be called when actually started
	}
	else if (rslt == COMMS_ALREADY)
	{
		// it is already started, no callback will fire
	}

// Allow communications layer to sleep again
	comms_sleep_allow(&m_sleep_ctrl);
```


# Timestamps

Received messages are timestamped as they arrive, transmitted messages are
timestamped when sent out and the timestamp can be inspected in the send-done
event. A combination of send and receive timestamping features can be used
to communicate an event timestamp (something in the past or the future) between
two devices that do not share a synchronized clock. Timestamps are relative
to the counter available from `comms_get_time_micro` and are always
represented in microseconds, though the underlying physical clock may have a
different granularity.

**Timestamping features depend on the capabilities of the underlying
implementation and may not be supported for all communications layers!**

## Message timestamps

Message timestamps can be read with the combination of `comms_timestamp_valid`
and `comms_get_timestamp_micro`. It is up to the user to know if the message
was received or sent, to know if they are getting an RX or TX timestamp.

## Event timestamps

Event timestamps are set with the `comms_set_event_time_micro` function and
on reception, can be checked with `comms_event_time_valid` and `comms_get_event_time_micro`.
Event times must be within +- 30 minutes of the current moment.
