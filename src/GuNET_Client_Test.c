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
 * Name        : GuNET_Client_Test.c
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : FreeBSD
 * Description : 
 * ============================================================================ */

#include "../include/GuNET_Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static GuNET_Client_Error_t err;
#define _(x) if ((err = x)) {printf("ERROR: %i\n", (int)err); return -1;}

int main(int argc, char ** argv) {
	GuNET_Client_t * client;
	char * query = "TESTING123";
	char * got;
	uint16_t size = strlen(query) + 1;

	printf("GuNET_Client_Init\n");
	fflush(stdout);
	_(GuNET_Client_Init(&client));
	printf("GuNET_Client_Connect\n");
	fflush(stdout);
	_(GuNET_Client_Connect(client, "localhost", 6545));
	printf("GuNET_Client_Send\n");
	fflush(stdout);
	_(GuNET_Client_Send(client, &size, sizeof(uint16_t)));
	_(GuNET_Client_Send(client, query, size));
	printf("GuNET_Client_Receive\n");
	fflush(stdout);

	_(GuNET_Client_Receive(client, &size, sizeof(uint16_t)));
	got = malloc(size);
	_(GuNET_Client_Receive(client, got, sizeof(got)));
	printf("GuNET_Client_Disconnect\n");
	fflush(stdout);
	_(GuNET_Client_Disconnect(client));
	printf("Sent %s\n", query);
	printf("Got %s\n", got);
	free(got);
	return 0;
}
#undef _
