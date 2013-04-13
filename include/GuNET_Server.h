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
 * Name        : GuNET_Server.h
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : FreeBSD
 * Description : 
 * ============================================================================ */

#ifndef GUNET_SERVER_H_
#define GUNET_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GuNET_Server_Error {
	GuNET_SERVER_ERROR_NONE = 0,
	GuNET_SERVER_ERROR_INVALID_ARGS,
	GuNET_SERVER_ERROR_MEMORY_ERROR,
	GuNET_SERVER_ERROR_SOCKET_ERROR,
	GuNET_SERVER_ERROR_NEED_MORE_DATA
} GuNET_Server_Error_t;

#define GuNET_Server_Error_ToString(x) #x

typedef struct GuNET_Server_Client GuNET_Server_Client_t;

typedef void (*GuNET_Server_Client_OnConnect_t)(GuNET_Server_Client_t * client);
typedef void (*GuNET_Server_Client_OnDisconnect_t)(
		GuNET_Server_Client_t * client);
typedef void (*GuNET_Server_Client_OnData_t)(GuNET_Server_Client_t * client);

typedef struct GuNET_Server {
	void * private;
} GuNET_Server_t;

struct GuNET_Server_Client {
	void * private;
};

GuNET_Server_Error_t GuNET_Server_Init(GuNET_Server_t ** server, int port,
		GuNET_Server_Client_OnConnect_t onConnect,
		GuNET_Server_Client_OnDisconnect_t onDisconnect,
		GuNET_Server_Client_OnData_t onData);
GuNET_Server_Error_t GuNET_Server_Free(GuNET_Server_t * server);
GuNET_Server_Error_t GuNET_Server_RunLoop(GuNET_Server_t * server);

GuNET_Server_Error_t GuNET_Server_Client_SetEncryptionKey(
		GuNET_Server_Client_t * client, const char * key, int length);
GuNET_Server_Error_t GuNET_Server_Client_SetUserData(
		GuNET_Server_Client_t * client, void * userdata);

GuNET_Server_Error_t GuNET_Server_Client_GetUserData(
		GuNET_Server_Client_t * client, void ** userdata);

GuNET_Server_Error_t GuNET_Server_Client_Disconnect(
		GuNET_Server_Client_t * client);

GuNET_Server_Error_t GuNET_Server_Client_Send(GuNET_Server_Client_t * client,
		const void * buffer, int size);
GuNET_Server_Error_t GuNET_Server_Client_Receive(GuNET_Server_Client_t * client,
		void * buffer, int size);

#ifdef __cplusplus
}
#endif

#endif /* GUNET_SERVER_H_ */
