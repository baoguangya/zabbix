/*
** Zabbix
** Copyright (C) 2001-2019 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#ifndef ZABBIX_ZBXIPCSERVICE_H
#define ZABBIX_ZBXIPCSERVICE_H

#include "common.h"
#include "zbxalgo.h"

#define ZBX_IPC_SOCKET_BUFFER_SIZE	4096

#define ZBX_IPC_RECV_IMMEDIATE	0
#define ZBX_IPC_RECV_WAIT	1
#define ZBX_IPC_RECV_TIMEOUT	2

#define ZBX_IPC_WAIT_FOREVER	-1

typedef struct
{
	/* the message code */
	zbx_uint32_t	code;

	/* the data size */
	zbx_uint32_t	size;

	/* the data */
	unsigned char	*data;
}
zbx_ipc_message_t;

/* Messaging socket, providing blocking connections to IPC service. */
/* The IPC socket api is used for simple write/read operations.     */
typedef struct
{
	/* socket descriptor */
	int		fd;

	/* incoming data buffer */
	unsigned char	rx_buffer[ZBX_IPC_SOCKET_BUFFER_SIZE];
	zbx_uint32_t	rx_buffer_bytes;
	zbx_uint32_t	rx_buffer_offset;
}
zbx_ipc_socket_t;

typedef struct zbx_ipc_client zbx_ipc_client_t;

/* IPC service */
typedef struct
{
	/* the listening socket descriptor */
	int			fd;

	struct event_base	*ev;
	struct event		*ev_listener;
	struct event		*ev_timer;

	/* the unix socket path */
	char			*path;

	/* the connected clients */
	zbx_vector_ptr_t	clients;

	/* the clients with messages */
	zbx_queue_ptr_t		clients_recv;
}
zbx_ipc_service_t;

/* IPC client, providing nonblocking connections to IPC service.  */
/* The IPC client api is used to support read timeouts or to send */
/* large amount of data without blocking write operations.        */
struct zbx_ipc_client
{
	zbx_ipc_socket_t	csocket;
	zbx_ipc_service_t	*service;

	struct event_base	*ev;
	struct event		*ev_timer;

	zbx_uint32_t		rx_header[2];
	unsigned char		*rx_data;
	zbx_uint32_t		rx_bytes;
	zbx_queue_ptr_t		rx_queue;
	struct event		*rx_event;

	zbx_uint32_t		tx_header[2];
	unsigned char		*tx_data;
	zbx_uint32_t		tx_bytes;
	zbx_queue_ptr_t		tx_queue;
	struct event		*tx_event;

	zbx_uint64_t		id;
	unsigned char		state;

	zbx_uint32_t		refcount;
};

int	zbx_ipc_service_init_env(const char *path, char **error);
void	zbx_ipc_service_free_env(void);
int	zbx_ipc_service_start(zbx_ipc_service_t *service, const char *service_name, char **error);
int	zbx_ipc_service_recv(zbx_ipc_service_t *service, int timeout, zbx_ipc_client_t **client,
		zbx_ipc_message_t **message);
void	zbx_ipc_service_close(zbx_ipc_service_t *service);

int	zbx_ipc_client_open(zbx_ipc_client_t *client, const char *service_name, int timeout, char **error);
void	zbx_ipc_client_close(zbx_ipc_client_t *client);
int	zbx_ipc_client_send(zbx_ipc_client_t *client, zbx_uint32_t code, const unsigned char *data, zbx_uint32_t size);
int	zbx_ipc_client_flush(zbx_ipc_client_t *client, int timeout);
int	zbx_ipc_client_check_unsent_data(zbx_ipc_client_t *client);
int	zbx_ipc_client_recv(zbx_ipc_client_t *client, int timeout, zbx_ipc_message_t **message);

void	zbx_ipc_client_addref(zbx_ipc_client_t *client);
void	zbx_ipc_client_release(zbx_ipc_client_t *client);
int	zbx_ipc_client_connected(zbx_ipc_client_t *client);

int	zbx_ipc_socket_open(zbx_ipc_socket_t *csocket, const char *service_name, int timeout, char **error);
void	zbx_ipc_socket_close(zbx_ipc_socket_t *csocket);
int	zbx_ipc_socket_write(zbx_ipc_socket_t *csocket, zbx_uint32_t code, const unsigned char *data,
		zbx_uint32_t size);
int	zbx_ipc_socket_read(zbx_ipc_socket_t *csocket, zbx_ipc_message_t *message);

void	zbx_ipc_message_free(zbx_ipc_message_t *message);
void	zbx_ipc_message_clean(zbx_ipc_message_t *message);
void	zbx_ipc_message_init(zbx_ipc_message_t *message);
void	zbx_ipc_message_format(const zbx_ipc_message_t *message, char **data);
void	zbx_ipc_message_copy(zbx_ipc_message_t *dst, const zbx_ipc_message_t *src);

#endif

