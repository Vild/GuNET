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
 * Name        : GuNET_Server.c
 * Author(s)   : Dan "WildN00b" Printzell
 * Copyright   : FreeBSD
 * Description : 
 * ============================================================================ */

#include "GuNET_Server.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/* For fcntl */
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 16384

#ifndef OUTPUTERROR
#define OUTPUTERROR 1
#endif

static int check_ = 0;

#define check(statement, errorcode) if ((check_ = (statement))) {\
										if (OUTPUTERROR) fprintf(stderr, "[GuNET_Server] [CHECK] Errorcode: %s Statement returned: %i", GuNET_Server_Error_ToString(errorcode), check_);\
										return errorcode;}

static void GuNET_Server_Client_free(GuNET_Server_Client_t * client);
static void GuNET_Server_Client_onConnect(evutil_socket_t fd, short event,
		void * serv);
static void GuNET_Server_Client_onRead(struct bufferevent * bev, void * serv);
static void GuNET_Server_Client_onError(struct bufferevent * bev, short error,
		void * serv);
static void GuNET_Server_Client_onSignal(evutil_socket_t sig, short events,
		void * user_data);

static GuNET_Server_Client_t * GuNET_Server_getLastClient(
		GuNET_Server_t * server) {
	GuNET_Server_Client_t * cur;
	if (!server || !server->clients)
		return NULL ;
	cur = server->clients;
	while (cur->next != NULL )
		cur = cur->next;
	return cur;
}

static GuNET_Server_Client_t * GuNET_Server_getClient(GuNET_Server_t * server,
		int fd) {
	GuNET_Server_Client_t * cur;
	if (!server)
		return NULL ;
	cur = server->clients;
	while (cur != NULL ) {
		if (cur->fd == fd)
			return cur;
		cur = cur->next;
	}
	return NULL ;
}

GuNET_Server_Error_t GuNET_Server_Init(GuNET_Server_t ** server, int port,
		GuNET_Server_Client_OnConnect_t onConnect,
		GuNET_Server_Client_OnDisconnect_t onDisconnect,
		GuNET_Server_Client_OnData_t onData, void * userdata) {
	GuNET_Server_t * newServer;
	struct sockaddr_in addr;
	struct event * signal_event;

#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	WSAStartup(wVersionRequested, &wsaData);
#endif

	check(!server, GuNET_SERVER_ERROR_INVALID_ARGS);

	newServer = malloc(sizeof(GuNET_Server_t));
	check(!newServer, GuNET_SERVER_ERROR_MEMORY_ERROR);

	newServer->base = event_base_new();

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = htons(port);

	newServer->fd = socket(AF_INET, SOCK_STREAM, 0);
	evutil_make_socket_nonblocking(newServer->fd);
	evutil_make_listen_socket_reuseable(newServer->fd);

	check(bind(newServer->fd, (struct sockaddr * )&addr, sizeof(addr)) < 0,
			GuNET_SERVER_ERROR_SOCKET_ERROR);

	check(listen(newServer->fd, 16) < 0, GuNET_SERVER_ERROR_SOCKET_ERROR);

	newServer->listen = event_new(newServer->base, newServer->fd,
			EV_READ | EV_PERSIST, GuNET_Server_Client_onConnect,
			(void *) newServer);
	event_add(newServer->listen, NULL );
	newServer->onConnect = onConnect;
	newServer->onDisconnect = onDisconnect;
	newServer->onData = onData;
	newServer->userdata = userdata;

	signal_event =
			evsignal_new(newServer->base, SIGINT, GuNET_Server_Client_onSignal, (void *)newServer->base);

	if (!signal_event || event_add(signal_event, NULL ) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	*server = newServer;
	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Free(GuNET_Server_t * server) {
	GuNET_Server_Client_t * cur;
	check(!server, GuNET_SERVER_ERROR_INVALID_ARGS);

	cur = server->clients;
	while (cur != NULL ) {
		GuNET_Server_Client_free(cur);
		cur = cur->next;
	}

	event_base_free(server->base);
	event_free(server->listen);

	evutil_closesocket(server->fd);

	free(server);

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_RunLoop(GuNET_Server_t * server) {
	check(!server, GuNET_SERVER_ERROR_INVALID_ARGS);

	event_base_dispatch(server->base);

	return GuNET_SERVER_ERROR_NONE;
}

static void GuNET_Server_Client_free(GuNET_Server_Client_t * client) {
	GuNET_Server_t * server;
	if (!client)
		return;

	server = client->server;
	if (server->clients == client)
		server->clients = client->next;
	else {
		if (client->prev)
			client->prev->next = client->next;
		if (client->next)
			client->next->prev = client->prev;
	}

	/*evutil_closesocket(client->fd);
	 evbuffer_free(client->read);
	 evbuffer_free(client->write);*/
	bufferevent_free(client->bev);

	if (client->length)
		free(client->key);
	free(client);
}

static void GuNET_Server_Client_onConnect(evutil_socket_t fd, short event,
		void * serv) {
	GuNET_Server_Client_t * client;
	GuNET_Server_t * server = (GuNET_Server_t *) serv;
	struct sockaddr_storage ss;
#ifdef _WIN32
	int slen = sizeof(ss);
#else
	socklen_t slen = sizeof(ss);
#endif
	struct bufferevent * bev;
	int clientfd = accept(fd, (struct sockaddr *) &ss, &slen);
	if (clientfd < 0) {

	} else if (clientfd > FD_SETSIZE) {
		if (OUTPUTERROR)
			fprintf(stderr, "[GuNET_Server] clientfd > FD_SETSIZE");
		evutil_closesocket(clientfd);
		return;
	} else {
		client = malloc(sizeof(GuNET_Server_Client_t));
		if (!client) {
			if (OUTPUTERROR)
				fprintf(stderr,
						"[GuNET_Server] Failed to malloc for new client.");
			evutil_closesocket(clientfd);
			return;
		}
		evutil_make_socket_nonblocking(clientfd);
		bev = bufferevent_socket_new(server->base, clientfd,
				BEV_OPT_CLOSE_ON_FREE);
		if (!bev) {
			if (OUTPUTERROR)
				fprintf(stderr, "[GuNET_Server] Failed to malloc bufferevent.");
			free(client);
			evutil_closesocket(clientfd);
			return;
		}
		bufferevent_setcb(bev, GuNET_Server_Client_onRead, NULL,
				GuNET_Server_Client_onError, serv);
		bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
		bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
		client->fd = clientfd;
		client->bev = bev;
		client->server = server;
		client->length = 0;
		client->key = NULL;
		client->userdata = server->userdata;

		client->next = NULL;
		client->prev = GuNET_Server_getLastClient(server);
		if (!client->prev)
			server->clients = client;
		else
			client->prev->next = client;

		if (server->onConnect)
			server->onConnect(client, client->userdata);
	}
}

static void GuNET_Server_Client_onRead(struct bufferevent * bev, void * serv) {
	GuNET_Server_t * server = (GuNET_Server_t *) serv;
	GuNET_Server_Client_t * cur;
	if (!server->clients || !server->onData)
		return;
	cur = server->clients;
	while (cur != NULL ) {

		if (cur->bev == bev) {
			server->onData(cur, cur->userdata);
			return;
		}
		cur = cur->next;
	}
}

static void GuNET_Server_Client_onError(struct bufferevent * bev, short error,
		void * serv) {
	GuNET_Server_t * server = (GuNET_Server_t *) serv;
	GuNET_Server_Client_t * client = GuNET_Server_getClient(server,
			bufferevent_getfd(bev));

	if (server->onDisconnect)
		server->onDisconnect(client, client->userdata);
	GuNET_Server_Client_free(client);
}

static void GuNET_Server_Client_onSignal(evutil_socket_t sig, short events,
		void * user_data) {
	struct event_base *base = user_data;
	struct timeval delay = { 2, 0 };

	if (OUTPUTERROR)
		printf(
				"[GuNET_Server]Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

GuNET_Server_Error_t GuNET_Server_Client_SetEncryptionKey(
		GuNET_Server_Client_t * client, const char * key, int length) {
	check(!client, GuNET_SERVER_ERROR_INVALID_ARGS);

	if (client->length)
		free(client->key);

	if (length) {
		client->key = malloc(length);
		strncpy(client->key, key, length);
		client->length = length;
	}

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_SetUserData(
		GuNET_Server_Client_t * client, void * userdata) {
	check(!client, GuNET_SERVER_ERROR_INVALID_ARGS);

	client->userdata = userdata;

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_GetUserData(
		GuNET_Server_Client_t * client, void ** userdata) {
	check(!client, GuNET_SERVER_ERROR_INVALID_ARGS);

	*userdata = client->userdata;

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_Disconnect(
		GuNET_Server_Client_t * client) {
	check(!client, GuNET_SERVER_ERROR_INVALID_ARGS);

	if (client->server->onDisconnect)
		client->server->onDisconnect(client, client->userdata);

	GuNET_Server_Client_free(client);

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_Send(GuNET_Server_Client_t * client,
		const void * buffer, int size) {
	check(!client || !buffer, GuNET_SERVER_ERROR_INVALID_ARGS);

	bufferevent_write(client->bev, buffer, size);
	bufferevent_flush(client->bev, EV_WRITE, BEV_FINISHED);

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_Receive(GuNET_Server_Client_t * client,
		void * buffer, int size) {
	check(!client || !buffer, GuNET_SERVER_ERROR_INVALID_ARGS);

	bufferevent_read(client->bev, buffer, size);
	bufferevent_flush(client->bev, EV_READ, BEV_NORMAL);

	return GuNET_SERVER_ERROR_NONE;
}

GuNET_Server_Error_t GuNET_Server_Client_ReceiveSize(
		GuNET_Server_Client_t * client, int * size) {
	check(!client || !size, GuNET_SERVER_ERROR_INVALID_ARGS);

	*size = evbuffer_get_length(bufferevent_get_output(client->bev));

	return GuNET_SERVER_ERROR_NONE;
}

#undef check
