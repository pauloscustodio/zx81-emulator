//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32
using socket_t = SOCKET;
using socket_size_t = int;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
using socket_t = int;
using socket_size_t = ssize_t;
constexpr socket_t invalid_socket = -1;
#endif

void init_terminal();
void sleep_ms(Milliseconds ms);
void init_socket_runtime();
void close_socket(socket_t sock);
void set_socket_non_blocking(socket_t sock);
std::string find_file_case(const std::string& path);
