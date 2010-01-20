/*
 * rpc_op.c - RPC 操作抽象
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
 * @file roc_op.c
 * @author microcai
 *
 * rpc_op.c 负责抽象化 RPC 调用的网络传输。
 * 直接调用 src/database.c 里定义的同名 RPC 函数
 * RPC 函数在 src/database.c 实现 和 lib/client.c (暂时) 存根
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define __DISTDB_SERVER_SIDE_H

#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/rpc.h"
#include "../include/db_def.h"

static int rpc_call_exec_sql(struct rpc_clients* client,char * data, size_t * retsize)
{
	int ret;

	struct execute_sql_bin * pdata = (typeof(pdata)) data;
	struct DISTDB_SQL_RESULT * rest = 0;

	ret = distdb_rpc_execute_sql_bin(&rest,
			pdata->data,pdata->length,pdata->flag);

	memcpy(data,&(rest),8);

	*retsize = sizeof(void*);

	if(!ret) // 记录到结果链表中
	{
		LIST_ADDTOTAIL(& client->sql_results , & rest->resultlist );
	}

	return ret ;

}

static int rpc_free_result(struct rpc_clients* client, char * data, size_t * retsize)
{
	// 首先要查看有没有这样的东东吧？

	*retsize = 0;
	DISTDB_SQL_RESULT * reslt ;
	memcpy(&reslt,data,sizeof(reslt));
	LIST_DELETE_AT(&reslt->resultlist);
	return distdb_rpc_free_result(reslt);
}

static int rpc_fetch_result(struct rpc_clients*client, char * data, size_t * retsize)
{
	int i,retval;
	DISTDB_SQL_RESULT * reslt ;
	memcpy(&reslt,data,sizeof(reslt));

	return distdb_fetch_result(reslt,data,retsize);
}

static int rpc_stub(struct rpc_clients*client, char * data, size_t * ret){	*ret = 0;return -1;}
static int (* rpc_call_table[20])(struct rpc_clients*, char * data, size_t * ret)  =
{
		/*The first call.*/
		rpc_stub,
		/* DISTDB_RPC_EXECUTE_SQL_BIN = 1 */
		rpc_call_exec_sql,
		/*DISTDB_RPC_FREE_RESLUT = 2*/
		rpc_free_result,
		/*DISTDB_RPC_FETCH_RESULT = 3*/
		rpc_fetch_result,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub,
		rpc_stub
};

static void rpc_dispatch(struct rpc_clients * client,size_t * len,char * recv)
{
	size_t return_size;

	struct rpc_packet_call * pc = (typeof(pc))recv;
	struct rpc_packet_ret * pr = (typeof(pr))recv;
	return_size = *len - RPC_PACKET_HEADER_SIZE ;
	if( pc->rpc_call_id < 20)
		pr->ret = (rpc_call_table[pc->rpc_call_id])(client,pc->data, &return_size);
	else
		{
			pr->ret = -1;
			return_size = 0;
		}
	pr->len = return_size  + RPC_PACKET_HEADER_SIZE ;
	* len = pr->len ;
}

/*
 * The big massive loop than handles RPC call.
 */
void * rpc_loop_thread(void*__paramter)
{
	int					sock;
	char*				buffer;
	size_t				buffersize;
	size_t				recv_len;
	struct	rpc_packet_call * header;
	struct rpc_clients	client;

	struct	paramter{
		struct sockaddr_in addr;
		int 	sock;
		socklen_t	addr_len;
	}*paramter = (typeof(paramter)) __paramter;

	memset(&client,0,sizeof(client));
	client.sql_results.head = client.sql_results.tail = (struct list_node*)& (client.sql_results );
	client.addr = paramter->addr;
	sock = paramter->sock ;

	free(__paramter);

/*	recv_len = 1;
	SO_DEBUG
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *) &recv_len, sizeof(int));
*/

	//暂时不串起来所有的客户连接
	// not schedule for implementation
	//LIST_ADDTOTAIL()

	buffersize = 4096*3;// MORE THAN ONE PACKET

	buffer = malloc(buffersize);
	memset(buffer,0,buffersize);

	while(recv(sock,buffer,RPC_PACKET_HEADER_SIZE,MSG_NOSIGNAL|MSG_PEEK))
	{
		header = (typeof(header)) buffer ;
		recv_len = recv(sock,buffer , header->len , MSG_NOSIGNAL);
		rpc_dispatch(&client,&recv_len,buffer);
		if(send(sock,buffer,recv_len,MSG_NOSIGNAL) < 0)
			break;
		memset(buffer,0,buffersize);
	};
	free(buffer);

	/**
	 * 释放掉所有还没释放的数据
	 */
	struct list_node * n;
	for(n=client.sql_results.head ; n != client.sql_results.tail->next ; n = n->next)
	{
		distdb_rpc_free_result(LIST_HEAD(n,DISTDB_SQL_RESULT,resultlist));
	}

	//LIST_DELETE_AT( & client.clients);

	return 0;
}

int open_rpc_socket()
{
	int opt = 1;
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = RPC_DEFAULT_PORT;
	g_rpc_socket = socket(AF_INET,SOCK_STREAM,0);
	setsockopt(g_rpc_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bind(g_rpc_socket,(struct sockaddr*)&addr,INET_ADDRSTRLEN);
	return listen(g_rpc_socket,2);
}
