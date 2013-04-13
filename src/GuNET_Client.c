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
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{} LOSS
 * OF USE, DATA, OR PROFITS{} OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ============================================================================
 * Name        : GuNET_Client.c
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : GPLv2, i think
 * Description : 
 * ============================================================================ */

#include "GuNET_Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef _WIN32
#define __USE_MISC
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#ifndef OUTPUTERROR
#define OUTPUTERROR 1
#endif

static int check_ = 0;

#define check(statement, errorcode) if ((check_ = (statement))) {\
										if (OUTPUTERROR) fprintf(stderr, "[GuNET_Client] [CHECK] Errorcode: %s Statement returned: %i", GuNET_Client_Error_ToString(errorcode), check_);\
										return errorcode;}

#define checkWinSock(statement, errorcode) if ((check_ = (statement))) {\
										if (OUTPUTERROR) fprintf(stderr, "[GuNET_Client] [CHECK] Errorcode: %s Statement returned: %i", GuNET_Client_Error_ToString(errorcode), check_);\
										closesocket(client->fd);WSACleanup();return errorcode;}

GuNET_Client_Error_t GuNET_Client_Init(GuNET_Client_t ** client) {
	GuNET_Client_t * newClient;
#ifdef _WIN32
	WSADATA wsaData;
#endif
	check(!client, GuNET_CLIENT_ERROR_INVALID_ARGS);
	newClient = malloc(sizeof(GuNET_Client_t));
	check(!newClient, GuNET_CLIENT_ERROR_MEMORY_ERROR);

#ifdef _WIN32
	check(WSAStartup(MAKEWORD(2,2), &wsaData), GuNET_CLIENT_ERROR_SOCKET_ERROR);
#endif

	newClient->fd = 0;
	newClient->key = NULL;
	newClient->length = 0;

	*client = newClient;

	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_Free(GuNET_Client_t * client) {
	check(!client, GuNET_CLIENT_ERROR_INVALID_ARGS);
	if (client->key)
		free(client->key);
	free(client);
	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_SetEncryptionKey(GuNET_Client_t * client,
		const char * key, int length) {
	check(!client, GuNET_CLIENT_ERROR_INVALID_ARGS);

	if (client->length)
		free(client->key);

	if (length) {
		client->key = malloc(length);
		strncpy(client->key, key, length);
		client->length = length;
	}

	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_Connect(GuNET_Client_t * client,
		const char * address, int port) {
#ifdef _WIN32
	struct addrinfo * ips = NULL;
	struct addrinfo * ptr = NULL;
	struct addrinfo hints;
	int result = 0;
	char portC[16];
	snprintf(portC, 16,"%d", port);
#else
	struct sockaddr_in sin;
	struct hostent * h;
#endif
	check(!client || !address, GuNET_CLIENT_ERROR_INVALID_ARGS);

#ifdef _WIN32
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	checkWinSock(getaddrinfo(address, portC, &hints, &ips), GuNET_CLIENT_ERROR_UNABLE_TO_RESOLVE_HOSTNAME);

	for (ptr = (struct addrinfo *) ips; ptr != NULL; ptr = ptr->ai_next) {

		client->fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		checkWinSock(client->fd == INVALID_SOCKET, GuNET_CLIENT_ERROR_COULD_NOT_CONNECT);

		result = connect(client->fd, ptr->ai_addr, (int) ptr->ai_addrlen);
		if (result != SOCKET_ERROR)
		break;

		closesocket(client->fd);
		client->fd = INVALID_SOCKET;
	}

	freeaddrinfo(ips);
	checkWinSock(client->fd == INVALID_SOCKET, GuNET_CLIENT_ERROR_COULD_NOT_CONNECT);
#else
	h = gethostbyname(address);
	check(!h, GuNET_CLIENT_ERROR_UNABLE_TO_RESOLVE_HOSTNAME);
	check(h->h_addrtype != AF_INET, GuNET_CLIENT_ERROR_NO_IPV6);

	client->fd = socket(AF_INET, SOCK_STREAM, 0);
	check(client->fd < 0, GuNET_CLIENT_ERROR_SOCKET_ERROR);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr = *(struct in_addr *) h->h_addr;

	check(connect(client->fd, (struct sockaddr * )&sin, sizeof(sin)),
			GuNET_CLIENT_ERROR_COULD_NOT_CONNECT);
#endif
	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_Disconnect(GuNET_Client_t * client) {
#ifdef _WIN32
	int result;
	char recvbuf[512];
#endif

	check(!client || !client->fd, GuNET_CLIENT_ERROR_INVALID_ARGS);

#ifdef _WIN32
	checkWinSock(shutdown(client->fd, SD_SEND) == SOCKET_ERROR, GuNET_CLIENT_ERROR_NONE);
	do {
		result = recv(client->fd, recvbuf, 512, 0);
	}while( result > 0 );
	closesocket(client->fd);
	WSACleanup();
#else
	close(client->fd);
#endif
	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_Send(GuNET_Client_t * client,
		const void * buffer, int size) {
	char * data = NULL;
	int i;
	int j;
#ifndef _WIN32
	const char * tmpdata;
	ssize_t n_written, remaining;
#endif

	check(!client || !size || !client->fd, GuNET_CLIENT_ERROR_INVALID_ARGS);

	data = malloc(size);
	strncpy(data, buffer, size);
	if (client->length) {
		for (i = 0, j = 0; i < size; i++, j++) {
			if (j == client->length - 1)
				j %= (j * 8) / client->length;
			data[i] ^= (client->key[j] + i * j) % 255;
		}
	}

#ifdef _WIN32
	if ((check_ = (send(client->fd, data, size, 0) == SOCKET_ERROR))) {
		if (OUTPUTERROR)
		fprintf(stderr, "[GuNET_Client] [CHECK] Errorcode: %s Statement returned: %i",
				GuNET_Client_Error_ToString(GuNET_CLIENT_ERROR_SOCKET_ERROR),
				check_);
		closesocket(client->fd);
		WSACleanup();
		return GuNET_CLIENT_ERROR_SOCKET_ERROR;
	}
#else
	tmpdata = data;
	remaining = size;
	while (remaining) {
		n_written = send(client->fd, tmpdata, remaining, 0);
		if (n_written <= 0)
			return GuNET_CLIENT_ERROR_NONE;
		remaining -= n_written;
		tmpdata += n_written;
	}
#endif
	free(data);

	return GuNET_CLIENT_ERROR_NONE;
}

GuNET_Client_Error_t GuNET_Client_Receive(GuNET_Client_t * client,
		void * buffer, int size) {
	int i;
	int j;
	ssize_t read = 0;
#ifdef _WIN32
	int result = 0;
#else
	ssize_t result = 0;
#endif

	check(!client || !client->fd || !buffer || !size,
			GuNET_CLIENT_ERROR_INVALID_ARGS);
#ifdef _WIN32
	while (read < size) {
		result = recv(client->fd, buffer, size, 0);
		read += result;
		if (result == 0)
		break;
		if (result < 0)
		return GuNET_CLIENT_ERROR_SOCKET_ERROR;
	}
#else
	while (read < size) {
		result = recv(client->fd, buffer, size, 0);
		read += result;
		if (result == 0)
			break;
		if (result < 0)
			return GuNET_CLIENT_ERROR_SOCKET_ERROR;
	}
#endif

	if (client->length) {
		for (i = 0, j = 0; i < size; i++, j++) {
			if (j == client->length - 1)
				j %= (j * 8) / client->length;
			((char *) buffer)[i] ^= (client->key[j] + i * j) % 255;
		}
	}

	return GuNET_CLIENT_ERROR_NONE;
}

#undef check
#undef checkWinSock
