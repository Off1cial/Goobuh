#pragma once
#include "core/common.h"
#include <array>
#include <sys/socket.h>
#include <netinet/in.h>

#define NET_PACKET_MAX_SIZE 256

class NetAddress
{
  public:
    NetAddress() = default;
    NetAddress(const char* ip, uint16_t port);
    NetAddress(uint16_t port);

    static NetAddress FromNative(const sockaddr_in& addr);
    const sockaddr_in& Native() const { return mAddr; }

    bool operator==(const NetAddress& other) const;
  private:
    sockaddr_in mAddr{};
};



enum class NetProtocol : u8
{
  UDP, TCP
};

enum class NetPacketType : u8
{
  Connect,
  Disconnect,

  Accept,
};

struct NetPacketHeader
{
  u32 sequence;
  u32 size;
  NetPacketType type;
};

class NetBuffer
{
public:
  void Clear();

  void WriteU8(uint8_t value);
  void WriteU16(uint16_t value);
  void WriteU32(uint32_t value);

  void WriteI8(int8_t value);
  void WriteI16(int16_t value);
  void WriteI32(int32_t value);

  void WriteFloat(float value);

  bool ReadU8(uint8_t &value);
  bool ReadU16(uint16_t &value);
  bool ReadU32(uint32_t &value);

  bool ReadFloat(float &value);

  const uint8_t *Data() const;
  size_t Size() const;

private:
  std::array<uint8_t, NET_PACKET_MAX_SIZE> mData;
  size_t mWritePosition = 0;
  size_t mReadPosition = 0;
};
