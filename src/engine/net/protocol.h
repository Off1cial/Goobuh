#ifndef PROTOCOL_H
#define PROTOCOL_H


#include "common/common.h"

enum 
{
    PKT_UNKNOWN,
    PKT_CHALLENGE_REQUEST,
    PKT_CHALLENGE_SEND, 
    PKT_CHALLENGE_CONFIRM,
    PKT_USERCMD,
};


typedef struct usercmd_t
{
    float angles[3];
    float forwardmove;
    float sidemove;
    float upmove;
    long msec;
    int buttons;
} usercmd_t;
#define USERCMD_SIZE  (5 * sizeof(float) + sizeof(long) + sizeof(int))




#define PACKET_HEADER ( 2 * sizeof(long) + sizeof(u8) )
typedef struct packethdr_t
{
    long size; // data size
    long seq;
    u8 type;
} packethdr_t;  

#endif
