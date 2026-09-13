#include "net.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>

// ---------------------------------------------------------------------
// socket lifecycle
// ---------------------------------------------------------------------

netsocket_t net_socket_create(netprot_t prot) {
  int type = (prot == NET_PROT_TCP) ? SOCK_STREAM : SOCK_DGRAM;
  netsocket_t sock = socket(AF_INET, type, 0);

  if (sock < 0) {
    fprintf(stderr, "net_socket_create: socket() failed: %s\n", strerror(errno));
    return NET_INVALID_SOCKET;
  }

  // Not strictly OS wrapping, but every socket we hand out should be
  // nonblocking by default — the game loop polls, it never wants to stall.
  if (net_socket_set_nonblocking(sock) != 0) {
    net_socket_close(sock);
    return NET_INVALID_SOCKET;
  }

  if (prot == NET_PROT_UDP) {
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
  } else {
    int reuseaddr = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    int nodelay = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
  }

  return sock;
}

void net_socket_close(netsocket_t sock) {
  if (sock == NET_INVALID_SOCKET) return;
  close(sock);
}

int32_t net_socket_bind(netsocket_t sock, netaddr_t addr) {
  struct sockaddr_in sa;
  net_addr_to_sockaddr(&addr, &sa);

  if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    fprintf(stderr, "net_socket_bind: bind() failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int32_t net_socket_set_nonblocking(netsocket_t sock) {
  int flags = fcntl(sock, F_GETFL, 0);
  if (flags < 0) {
    fprintf(stderr, "net_socket_set_nonblocking: fcntl(GET) failed: %s\n", strerror(errno));
    return -1;
  }
  if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
    fprintf(stderr, "net_socket_set_nonblocking: fcntl(SET) failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int32_t net_socket_connect(netsocket_t sock, netaddr_t addr) {
  struct sockaddr_in sa;
  net_addr_to_sockaddr(&addr, &sa);

  if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    // EINPROGRESS is expected on a nonblocking connect — caller should
    // poll writability (select/poll) rather than treat this as failure.
    if (errno != EINPROGRESS) {
      fprintf(stderr, "net_socket_connect: connect() failed: %s\n", strerror(errno));
      return -1;
    }
  }
  return 0;
}

int32_t net_socket_listen(netsocket_t sock, int32_t backlog) {
  if (listen(sock, backlog) < 0) {
    fprintf(stderr, "net_socket_listen: listen() failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

netsocket_t net_socket_accept(netsocket_t sock, netaddr_t *out_addr) {
  struct sockaddr_in sa;
  socklen_t salen = sizeof(sa);

  netsocket_t client = accept(sock, (struct sockaddr *)&sa, &salen);
  if (client < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      fprintf(stderr, "net_socket_accept: accept() failed: %s\n", strerror(errno));
    }
    return NET_INVALID_SOCKET;
  }

  if (out_addr) net_sockaddr_to_addr(&sa, out_addr);
  net_socket_set_nonblocking(client);
  return client;
}

// ---------------------------------------------------------------------
// send / receive
// ---------------------------------------------------------------------

int32_t net_send(netsocket_t sock, const netaddr_t *to, const byte *data, size_t len) {
  ssize_t sent;

  if (to) {
    struct sockaddr_in sa;
    net_addr_to_sockaddr(to, &sa);
    sent = sendto(sock, data, len, 0, (struct sockaddr *)&sa, sizeof(sa));
  } else {
    sent = send(sock, data, len, 0);
  }

  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return 0; // would've blocked, try again next tick
    fprintf(stderr, "net_send: failed: %s\n", strerror(errno));
    return -1;
  }
  return (int32_t)sent;
}

int32_t net_recv(netsocket_t sock, netaddr_t *from, byte *buf, size_t buflen) {
  ssize_t received;

  if (from) {
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    received = recvfrom(sock, buf, buflen, 0, (struct sockaddr *)&sa, &salen);
    if (received >= 0) net_sockaddr_to_addr(&sa, from);
  } else {
    received = recv(sock, buf, buflen, MSG_DONTWAIT);
  }

  if (received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return 0; // nothing waiting
    fprintf(stderr, "net_recv: failed: %s\n", strerror(errno));
    return -1;
  }
  if (received == 0 && from == NULL) {
    // TCP peer closed the connection cleanly.
    return -1;
  }
  return (int32_t)received;
}

// ---------------------------------------------------------------------
// addr helpers
// ---------------------------------------------------------------------

void net_addr_to_sockaddr(const netaddr_t *addr, struct sockaddr_in *out) {
  memset(out, 0, sizeof(*out));
  out->sin_family = AF_INET;
  out->sin_addr.s_addr = htonl(addr->ip);
  out->sin_port = htons(addr->port);
}

void net_sockaddr_to_addr(const struct sockaddr_in *in, netaddr_t *out) {
  out->ip = ntohl(in->sin_addr.s_addr);
  out->port = ntohs(in->sin_port);
}

const char *net_addr_to_string(const netaddr_t *addr, char *buf, size_t buflen) {
  struct in_addr ia;
  ia.s_addr = htonl(addr->ip);
  snprintf(buf, buflen, "%s:%u", inet_ntoa(ia), addr->port);
  return buf;
}

uint8_t net_addr_from_string(const char *str, uint16_t port, netaddr_t *out) {
  struct in_addr ia;
  if (inet_pton(AF_INET, str, &ia) != 1) {
    fprintf(stderr, "net_addr_from_string: bad address '%s'\n", str);
    return 0;
  }
  out->ip = ntohl(ia.s_addr);
  out->port = port;
  return 1;
}

uint8_t net_addr_eq(const netaddr_t *a, const netaddr_t *b) {
  return a->ip == b->ip && a->port == b->port;
}
