#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_NETMSG 512

#include "common/common.h"

enum 
{
    PKT_UNKNOWN,
    PKT_CHALLENGE_REQUEST,
    PKT_CHALLENGE_SEND, 
    PKT_CHALLENGE_CONFIRM,
};




#define PACKET_HEADER ( sizeof(u32) + sizeof(u8) )
typedef struct packethdr_t
{
    u32 size; // data size
    u8 type;
} packethdr_t;  

#endif
