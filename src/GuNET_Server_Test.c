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
 * Name        : GuNET_Server_Test.c
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : FreeBSD
 * Description : 
 * ============================================================================ */
#include "../include/GuNET_Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static GuNET_Server_Error_t err;
#define _(x) if ((err = x)) {printf("ERROR: %i\n", (int)err); return -1;}
#define __(x) if ((err = x)) {printf("ERROR: %i\n", (int)err);}

void onConnect(GuNET_Server_Client_t * client, void * userdata) {
	printf("Client connected\n");
	fflush(stdout);
}

void onDisconnect(GuNET_Server_Client_t * client, void * userdata) {
	printf("Client disconnect\n");
	fflush(stdout);
}

void onData(GuNET_Server_Client_t * client, void * userdata) {
	uint16_t size;
	char * got;
	printf("Client send data\n");
	fflush(stdout);

	__(GuNET_Server_Client_Receive(client, &size, sizeof(uint16_t)));
	got = malloc(size);
	__(GuNET_Server_Client_Receive(client, got, size))
	if (err == GuNET_SERVER_ERROR_NEED_MORE_DATA) {
		free(got);
		return;
	}
	printf("Got and sending... %s\n", got);
	__(GuNET_Server_Client_Send(client, &size, sizeof(uint16_t)));
	__(GuNET_Server_Client_Send(client, got, size));
	free(got);
}

int main(int argc, char ** argv) {
	GuNET_Server_t * server;

	printf("GuNET_Server_Init\n");
	fflush(stdout);
	_(GuNET_Server_Init(&server, 6545, onConnect, onDisconnect, onData, NULL));
	printf("GuNET_Server_RunLoop\n");
	fflush(stdout);
	_(GuNET_Server_RunLoop(server));
	printf("GuNET_Server_Free\n");
	fflush(stdout);
	_(GuNET_Server_Free(server));
	return 0;
}
#undef _
#undef __
