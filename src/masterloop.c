/*
 * masterloop.c - home to the loop
 *
 * Copyright (C) 2009-2010 Kingstone, ltd
 *
 * Written by microcai in 2009-2010
 *
 * This software is lisenced under the Kingstone mid-ware Lisence.
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 *
 * If you have any question with law suite, please contract 黄小克, the owner of
 * this company.
 */


/**
 * Master loop --
 * listen on local service port and accept other nodes' connection
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/poll.h>
#include <pthread.h>
#include "../include/global_var.h"
#include "../include/distdb.h"
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
	char 					**	  table;
	struct sql_result_plain_text* ptext;

	struct db_exchange_header * db_hdr = (typeof(db_hdr)) data;
	result = db_hdr->restptr;
	if (distdb_execute_sql_bin(NULL, &result, db_hdr->sql_command,
			db_hdr->length - db_exchange_header_size, db_hdr->execflag))
	{
		*ret = 0;
		return 0;
	}
	if(result==NULL) //不要结果，偶这里也没有结果
		return 0;
	//fetch result and send back! 只获得本地结果，远程结果直接转发
	while (distdb_fetch_result_local(result, &table) == 0)
	{
		ptext = convert_strtable2plain(result->colums, table);
		db_hdr->type = db_exchange_type_return_result;
		db_hdr->length = ptext->size + db_exchange_header_size;
		send(client->sock_peer, db_hdr, db_exchange_header_size, MSG_NOSIGNAL);
		send(client->sock_peer, ptext->plaindata, ptext->size, MSG_NOSIGNAL);
		free(ptext);
	}
	// send EOF
	db_hdr->type = db_exchange_type_end_result ;
	send(client->sock_peer,db_hdr,db_exchange_header_size,MSG_NOSIGNAL);
	distdb_free_local_result(result);
	return 0;
}

static int service_accpet_result(struct nodes* client, char * data, size_t * ret)
{
	struct DISTDB_SQL_RESULT	* result;
	char 					**	  table;
	struct sql_result_plain_text * res_plain;

	struct db_exchange_header * db_hdr = (typeof(db_hdr)) data;

	result = db_hdr->restptr ;

	if(db_hdr->execflag & DISTDB_EXECSQL_NORESULT) // 没要结果的，偶这里也不会有结果的
		return *ret = 0;

	pthread_mutex_lock(&result->lock);
	//记录下来，呵呵 :)
	//看是否需要挂入列队。
	if(result->old_res)
	{ // 看来是一个中间人啊，呵呵
		db_hdr->restptr = result->old_res ;
		pthread_mutex_unlock(&result->lock);
		send(result->client->sock_peer,data,*ret,0);
		return 0;
	}


	res_plain = malloc(*ret);
	res_plain->size = db_hdr->length - db_exchange_header_size;
	result->colums = res_plain->colums;
	memcpy(res_plain->plaindata,db_hdr->data,res_plain->size);
	//挂入列队 ：）
	LIST_ADDTOTAIL(&result->sql_result,&res_plain->resultlist);
	pthread_mutex_unlock(&result->lock);
	return pthread_cond_signal(&result->waitcond);
	//唤醒可能沉睡的主线程
}

static int service_result_close(struct nodes* client, char * data, size_t * ret)
{
//	service_
	struct DISTDB_SQL_RESULT	* result;
	char 					**	  table;
	struct sql_result_plain_text * res_plain;

	struct db_exchange_header * db_hdr = (typeof(db_hdr)) data;

	//Add 验证机制
	distdb_free_remote_result(db_hdr->restptr);
}

static int service_stub(struct nodes* client, char * data, size_t * ret){*ret = 0;return -1;}
static int (* service_call_table[20])(struct nodes*, char * data, size_t * ret)  =
{
		service_exec_sql,
		service_accpet_result,
		service_result_close,
		service_stub,
		service_stub,
		service_stub,
		service_stub,
		service_stub,
		service_stub,
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
