/*
 * Copyright (C) 2012-2015 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STATIC_SOCKET_ADDRESS_HPP
#define STATIC_SOCKET_ADDRESS_HPP

#include "SocketAddress.hpp"
#include "Compiler.h"

#include <assert.h>
#include <stdint.h>

#ifdef HAVE_POSIX
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif

/**
 * An OO wrapper for struct sockaddr_storage.
 */
class StaticSocketAddress {
  size_t size;
  struct sockaddr_storage address;

public:
  StaticSocketAddress() = default;

  StaticSocketAddress &operator=(SocketAddress other);

  operator SocketAddress() const {
    return SocketAddress(reinterpret_cast<const struct sockaddr *>(&address),
                         size);
  }

#if defined(HAVE_POSIX) && !defined(__BIONIC__)
  /**
   * Make this a "local" address (UNIX domain socket).
   */
  void SetLocal(const char *path);
#endif

  /**
   * Creates a #StaticSocketAddress with the specified IPv4 address and
   * port.
   *
   * @parm ip the IPv4 address in host byte order
   */
  gcc_const
  static StaticSocketAddress MakeIPv4Port(uint32_t ip, unsigned port);

  gcc_const
  static StaticSocketAddress MakeIPv4Port(uint8_t a, uint8_t b, uint8_t c,
                                          uint8_t d, unsigned port) {
    uint32_t ip = (a << 24) | (b << 16) | (c << 8) | d;
    return MakeIPv4Port(ip, port);
  }

#ifdef __GLIBC__
  /**
   * Returns a StaticSocketAddress for the specified device. Caller
   * should check for validity of returned StaticSocketAddress.
   *
   * @param device is the device name f.i. "eth0"
   * @return StaticSocketAddress, use IsDefined() to check valid result
   */
  gcc_pure
  static StaticSocketAddress GetDeviceAddress(const char *device);

  /**
   * Converts StaticSocketAddress to human readable string
   *
   * @param buffer is the result buffer
   * @param buffer_size is the buffer size
   * @return IP address on success, else NULL
   */
  gcc_pure
  const char *ToString(char *buffer, size_t buffer_size) const;
#endif

  /**
   * Creates a #StaticSocketAddress with the IPv4 a wildcard address and the
   * specified port.
   */
  gcc_const
  static StaticSocketAddress MakePort4(unsigned port);

  operator struct sockaddr *() {
    return reinterpret_cast<struct sockaddr *>(&address);
  }

  operator const struct sockaddr *() const {
    return reinterpret_cast<const struct sockaddr *>(&address);
  }

  constexpr size_t GetCapacity() const {
    return sizeof(address);
  }

  size_t GetSize() const {
    return size;
  }

  void SetSize(size_t _size) {
    assert(_size > 0);
    assert(_size <= sizeof(address));

    size = _size;
  }

  int GetFamily() const {
    return address.ss_family;
  }

  bool IsDefined() const {
    return GetFamily() != AF_UNSPEC;
  }

  void Clear() {
    address.ss_family = AF_UNSPEC;
  }

  gcc_pure
  bool operator==(const StaticSocketAddress &other) const;

  bool operator!=(const StaticSocketAddress &other) const {
    return !(*this == other);
  }

#ifndef _WIN32_WCE
  bool Lookup(const char *host, const char *service, int socktype);
#endif
};

#endif
