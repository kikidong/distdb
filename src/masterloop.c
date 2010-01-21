/*
 * masterloop.c - home to the loop
 *
 * Copyright (C) 2009-2010 microcai
 *
 * This software is Public Domain
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 *
 */

/**
 * Master loop --
 * listen on local service port and accept other nodes' connection
 */

#include <sys/poll.h>
#include <pthread.h>
#include "../include/global_var.h"
#include "../include/communication.h"

static int accepts(struct nodes ** pn)
{
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					sock;
	struct list_node *	n;

	sock = accept(g_socket,(__SOCKADDR_ARG)&addr,&addr_len);

	if (sock <= 0)
		return 0; //忽略错误就可以了

//	pthread_mutex_lock(&nodelist_lock);
	if(peer_lookup_same(addr.sin_addr.s_addr))
		close(sock);
	*pn = nodes_new();
	LIST_ADDTOTAIL(&nodelist,&(*pn)->nodelist);
	pthread_mutex_unlock(&nodelist_lock);
	return 0;
}


int service_exec_sql(struct nodes* client, char * data, size_t * ret)
{
	//收到！ 哈哈
	struct DISTDB_SQL_RESULT	* result;
	struct db_exchange_header * db_hdr = (typeof(db_hdr)) data;
	if(distdb_execute_sql_bin(NULL, &result, db_hdr->exec_sql.sql_command,
			db_hdr->exec_sql.execflag));
	{
		*ret = 0;
		return 0;
	}
	//fetch result and send back!


	// send EOF




static int service_stub(struct nodes* client, char * data, size_t * ret){*ret = 0;return -1;}
static int (* service_call_table[20])(struct nodes*, char * data, size_t * ret)  =
{
		service_exec_sql,
		service_stub

};


void * massive_loop( struct nodes * client)
{
	char *buffer;
	struct db_exchange_header	db_hdr;
	size_t recv_len;

	const int buffersize = 8192;

	buffer = valloc(buffersize);
	//从这里接受连接到的节点的请求或者应答

	while(recv(client->sock_peer,&db_hdr,db_exchange_header_size,MSG_NOSIGNAL|MSG_PEEK))
	{

		recv_len = recv(client->sock_peer, buffer, db_hdr.length, MSG_NOSIGNAL);
//		rpc_dispatch(&client,&recv_len,buffer);
		(service_call_table[db_hdr.type])(client,buffer,&recv_len);
//		if(send(sock,buffer,recv_len,MSG_NOSIGNAL) < 0)
//			break;
		memset(buffer,0,buffersize);
	};
	free(buffer);

	//释放这个连接掌握的所有资源



}


void* service_loop(void*P)
{
	pthread_t	thread;

	struct pollfd pfd[1];
	pfd[0].fd = g_socket ;
	pfd[0].events = POLLIN ;

	struct nodes * pn;

	while (poll(pfd, 1, -1) == 1)
	{
		if(accepts(&pn))
			continue;
		pthread_create(&thread,0,(void *(*)(void *))massive_loop,pn);
	}

	return 0;
}
