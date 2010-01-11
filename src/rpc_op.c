/*
 * rpc_op.c - rpc operation
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define __DISTDB_SERVER_SIDE_H

#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/rpc.h"
#include "../include/db_def.h"

static int rpc_call_exec_sql(char * data, size_t * retsize)
{
	int ret;

	struct execute_sql_bin * pdata = (typeof(pdata)) data;

	ret = distdb_rpc_execute_sql_bin((DISTDB_SQL_RESULT**)data,pdata->data,pdata->length,pdata->flag);

	ret = sizeof(void*);

	return ret ;

}

static int rpc_free_result(char * data, size_t * retsize)
{
	*retsize = 0;
	return distdb_rpc_free_result((DISTDB_SQL_RESULT*)data);
}

static int rpc_fetch_result(char * data, size_t * retsize)
{
	int i,retval;
	DISTDB_SQL_RESULT * reslt = *(DISTDB_SQL_RESULT**)data;
	struct rpc_sql_result * srst = (typeof(srst))data;
	srst->number = reslt->columns;

	char ** res;

	retval = distdb_rpc_fetch_result(reslt,&res);

	if (retval==-1)
	{
		*retsize = 0;
		return -1;
	}

	char * real_result = (char*)(srst->offsets + reslt->columns);

	*retsize = reslt->columns +1 ;

	for( i = 0; i < srst->number ; ++i )
	{
		strcpy(real_result,res[i]);
		srst->offsets[i] = real_result - data;
		real_result += strlen(res[i])+1;
		*retsize += strlen(res[i])+1;
	}
	return retval;
}

static int rpc_stub(char * data, size_t * ret){	*ret = 0;return -1;}
static int (* rpc_call_table[20])(char * data, size_t * ret)  =
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

static void rpc_dispatch(size_t * len,char * recv)
{
	size_t return_size;

	struct rpc_packet_call * pc = (typeof(pc))recv;
	struct rpc_packet_ret * pr = (typeof(pr))recv;
	return_size = *len - sizeof(*pc);
	if( pc->rpc_call_id < 20)
		pr->ret = (rpc_call_table[pc->rpc_call_id])(pc->data, &return_size);
	else
		{
			pr->ret = -1;
			return_size = 0;
		}
	* len = return_size + sizeof(*pr);
}

/*
 * The big massive loop than handles RPC call.
 * Since it is UDP, we can use one socket to serve many clients
 */
static void * rpc_loop_thread(void*p)
{
	char * buffer = malloc(4096*3); // MORE THAN ONE PACKET
	size_t	recv_len;
	struct sockaddr_in	addr={0};
	socklen_t addr_len=INET_ADDRSTRLEN;
	do
	{
		addr_len = INET_ADDRSTRLEN;
		recv_len = recvfrom(g_rpc_socket,buffer,4096*3,0,(struct sockaddr*)&addr,&addr_len);
		rpc_dispatch(&recv_len,buffer);
		sendto(g_rpc_socket,buffer,recv_len,0,(struct sockaddr*)&addr,addr_len);
	}while(1);
}

int rpc_loop()
{
	pthread_t pt;
	return pthread_create(&pt,0,rpc_loop_thread,0);
}

int open_rpc_socket()
{
	int opt = 1;
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = RPC_DEFAULT_PORT;
	g_rpc_socket = socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(g_rpc_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bind(g_rpc_socket,(struct sockaddr*)&addr,INET_ADDRSTRLEN);
	listen(g_rpc_socket,2);
	return -1;
}
