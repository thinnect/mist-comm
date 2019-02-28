#ifndef MIST_COMM_EUI_H_
#define MIST_COMM_EUI_H_

// Address manipulation functions for EUI communications layer -----------------
ieee_eui64_t comms_get_destination(comms_msg_t* msg);
void comms_set_destination(comms_msg_t* msg, ieee_eui64_t dest);

ieee_eui64_t comms_get_source(comms_msg_t* msg);
void comms_set_source(comms_msg_t* msg, ieee_eui64_t source);
// ???
// consider using EUI pointers?
// ???

//
// TODO EUI registry functions will have to be defined
//

// -----------------------------------------------------------------------------

#endif//MIST_COMM_EUI_H_
