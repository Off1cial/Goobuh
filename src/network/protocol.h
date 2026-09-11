#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NETMSG_USERCMD 1;

typedef struct usercmd_t {
  int8_t mv_forward;
  int8_t mv_side;
  int8_t mv_up;
  uint8_t buttons;
  uint32_t tick;
  float viewangles[3];
} usercmd_t;


static inline size_t write_u32(uint8_t* buff, size_t pos, uint32_t u){
  buff[pos++] = u;
  buff[pos++] = u >> 8;
  buff[pos++] = u >> 16;
  buff[pos++] = u >> 24;
  return pos;
}

static inline uint32_t read_u32(uint8_t* buff, size_t* pos){
  uint32_t val = 0;
  val |= (uint32_t)buff[(*pos)++];
  val |= (uint32_t)buff[(*pos)++] << 8;
  val |= (uint32_t)buff[(*pos)++] << 16;
  val |= (uint32_t)buff[(*pos)++] << 24;
  return val;
}


static inline size_t write_f32(uint8_t *buff, size_t pos, float value)
{
  uint32_t bits;
  memcpy(&bits, &value, sizeof(bits));
  return write_u32(buff, pos, bits);
}

static inline float read_f32(uint8_t *buff, size_t *pos)
{
  uint32_t bits;
  float value;
  bits = read_u32(buff, pos);
  memcpy(&value, &bits, sizeof(value));
  return value;
}


static inline size_t serialise_usercmd(
    uint8_t *buff, size_t pos, const usercmd_t *cmd)
{
  buff[pos++] = NETMSG_USERCMD;

  buff[pos++] = (uint8_t)cmd->mv_forward;
  buff[pos++] = (uint8_t)cmd->mv_side;
  buff[pos++] = (uint8_t)cmd->mv_up;
  buff[pos++] = cmd->buttons;

  pos = write_u32(buff, pos, cmd->tick);

  pos = write_f32(buff, pos, cmd->viewangles[0]);
  pos = write_f32(buff, pos, cmd->viewangles[1]);
  pos = write_f32(buff, pos, cmd->viewangles[2]);

  return pos;
}

