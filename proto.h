/*
    USB DFU Flasher
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

#pragma pack(push, 1)

#define ISP_START_FRAME                             0x7f

#define ISP_ACK                                     0x79
#define ISP_NACK                                    0x1f

#define ISP_GET                                     0x00
#define ISP_GET_VERSION                             0x01
#define ISP_GET_ID                                  0x02


typedef struct {
    unsigned char cmd;
    unsigned char crc;
} REQ;

#pragma pack(pop)


#endif // PROTO_H
