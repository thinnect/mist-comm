#ifndef MIST_COMM_REF_H_
#define MIST_COMM_REF_H_

// Add default receiver handling functionality to a layer implementation
comms_error_t comms_initialize_rcvr_management(comms_layer_iface_t* comms);

// Deliver to registered receivers. No further actions taken on the message.
comms_error_t comms_basic_deliver(comms_layer_t* layer, comms_msg_t* msg);

#endif//MIST_COMM_REF_H_
