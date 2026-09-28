#include "net/bitbuff.hpp"

// Dereference increment
#define DEREFINC(p) (*p)++

#define _WRITE_INT(buff, pos, value) \
    _write_intgeneric(buff, pos, (uintmax_t)(value), sizeof(value))




// Read little endian
static inline bool _read_u32(const char* buff, size_t bufflen, size_t* pos, u32* out)
{
    if (*pos + sizeof(u32) > bufflen) return false;
    u32 value = 0;

    value |= (u32)(u8)buff[DEREFINC(pos)] << 24;
    value |= (u32)(u8)buff[DEREFINC(pos)] << 16;
    value |= (u32)(u8)buff[DEREFINC(pos)] << 8;
    value |= (u32)(u8)buff[DEREFINC(pos)];

    *out = value;
    return true;
}


static inline bool _read_u16(char* buff, size_t bufflen, size_t* pos, u16* out){
    if (*pos + sizeof(u16) > bufflen) return false;
    u16 val = 0;
    val |=  (u16)(u8)buff[DEREFINC(pos)] << 8;
    val |=  (u16)(u8)buff[DEREFINC(pos)];
    *out = val;
    return true;
}
// Writes in big endian
/*
    u32 u = 0x12345678;
    buff[pos++] = u >> 24; // 0x12
    buff[pos++] = u >> 16; // 0x34
    buff[pos++] = u >> 8;  // 0x56
    buff[pos++] = u;       // 0x78
 */
static inline void _write_u32(char* buff, size_t* pos, u32 value){
    buff[DEREFINC(pos)] = value >> 24;
    buff[DEREFINC(pos)] = value >> 16;
    buff[DEREFINC(pos)] = value >> 8;
    buff[DEREFINC(pos)] = value;
}

static inline void _write_u16(char* buff, size_t* pos, u16 value){
    buff[DEREFINC(pos)] = value >> 8;
    buff[DEREFINC(pos)] = value;
}



// Big endian write
static inline void _write_intgeneric(
    char* buff,
    size_t* pos,
    uintmax_t value,
    size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
        buff[DEREFINC(pos)] =
            (value >> (8 * (bytes - 1 - i))) & 0xFF;
}

static inline bool _read_intgeneric(
    char* buff,
    size_t bufflen,
    size_t* pos,
    size_t bytes,
    uintmax_t* out)
{
    uintmax_t value = 0;
    if (*pos + bytes > bufflen) return false;
    for (size_t i = 0; i < bytes; i++){
        value |= (uintmax_t)(u8)    buff[DEREFINC(pos)] << (8 * (bytes - 1 - i));
    }
    *out = value;
    return true;
}

#define TYPEMATCH(match, item) (typeof(match))item 


// Writes in host byte form
static inline void _write_netaddr(char* buff, size_t* pos, u32 ip, u16 port){
    _WRITE_INT(buff, pos, ip);
    _WRITE_INT(buff, pos, port);
}

static inline bool _read_netaddr(char* buff, size_t bufflen, size_t* pos, u32* ip_out, u16* port_out){

    uintmax_t tmp;
    if (!_read_intgeneric(buff, bufflen, pos, sizeof(*ip_out), &tmp)) return false;
    *ip_out = TYPEMATCH(*ip_out, tmp); 
    if (!_read_intgeneric(buff, bufflen, pos, sizeof(*port_out), &tmp)) return false;
    *port_out = TYPEMATCH(*port_out, tmp); 
    return true;
}







bool bf_read::ReadU32At( u32* out, size_t* pos ){
    m_bytehead = *pos;
    u32 val = 0;
    if (!_read_u32(m_pData, m_datalen, &m_bytehead, &val)) return false;
    *pos = m_bytehead;
    m_bithead = m_bytehead * 8;
    *out = val;
    return true;
}

bool bf_read::ReadU16At( u16* out, size_t* pos ){
    m_bytehead = *pos;
    u16 val = 0;
    if (!_read_u16(m_pData, m_datalen, &m_bytehead, &val)) return false;
    *pos = m_bytehead;
    m_bithead = m_bytehead * 8;
    *out = val;
    return true;
}


bool bf_read::ReadU8At(u8* out, size_t* pos) {
    if (*pos >= m_datalen)
        return false;

    m_bytehead = *pos;
    *out = m_pData[m_bytehead];

    m_bytehead++;
    *pos = m_bytehead;
    m_bithead = m_bytehead * 8;

    return true;
}

void bf_write::WriteU32At( u32 val, size_t* pos){
    m_bytehead = *pos;
    _write_u32(m_pData, &m_bytehead, val);
    *pos = m_bytehead;
    m_bithead = m_bytehead * 8;
}



void bf_write::WriteU16At( u16 val, size_t* pos){
    m_bytehead = *pos;
    _write_u16(m_pData, &m_bytehead, val);
    *pos = m_bytehead;
    m_bithead = m_bytehead * 8;
}

void bf_write::WriteU8At( u8 val, size_t* pos){
    m_bytehead = *pos;
    m_bithead = m_bytehead * 8;
    m_pData[m_bytehead] = val;
}
