/*
 * Copyright (c) 2013, Dan "WildN00b" Printzell
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * [*] Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * [*] Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 *     other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ============================================================================
 * Name        : GuNET_Client.h
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : GPLv2, i think
 * Description : 
 * ============================================================================ */

#ifndef GUNET_CLIENT_H_
#define GUNET_CLIENT_H_

#include <sys/types.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GuNET_Client_Error {
	GuNET_CLIENT_ERROR_NONE = 0,
	GuNET_CLIENT_ERROR_INVALID_ARGS,
	GuNET_CLIENT_ERROR_MEMORY_ERROR,
	GuNET_CLIENT_ERROR_SOCKET_ERROR,
	GuNET_CLIENT_ERROR_COULD_NOT_CONNECT,
	GuNET_CLIENT_ERROR_UNABLE_TO_RESOLVE_HOSTNAME,
	GuNET_CLIENT_ERROR_NO_IPV6
} GuNET_Client_Error_t;

#define GuNET_Client_Error_ToString(x) #x

typedef struct GuNET_Client {
	char * key;
	int length;
#ifdef _WIN32
	SOCKET fd;
#else
	int fd;
#endif
} GuNET_Client_t;

GuNET_Client_Error_t GuNET_Client_Init(GuNET_Client_t ** client);
GuNET_Client_Error_t GuNET_Client_Free(GuNET_Client_t * client);

GuNET_Client_Error_t GuNET_Client_SetEncryptionKey(GuNET_Client_t * client, const char * key, int length);

GuNET_Client_Error_t GuNET_Client_Connect(GuNET_Client_t * client,
		const char * address, int port);
GuNET_Client_Error_t GuNET_Client_Disconnect(GuNET_Client_t * client);

GuNET_Client_Error_t GuNET_Client_Send(GuNET_Client_t * client,
		const void * buffer, ssize_t size);
GuNET_Client_Error_t GuNET_Client_Receive(GuNET_Client_t * client, void * buffer,
		ssize_t size);

#ifdef __cplusplus
}
#endif

#endif /* GUNET_CLIENT_H_ */
