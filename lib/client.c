/*
 * client.c
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
#else
#include "./vcconfig.h"
#endif

#include "../include/distdb.h"
#include "../include/rpc.h"


static int rpc_socket = -1 ;
static struct sockaddr_in server_addr = {0};
long	seq;

int distdb_rpc_connectto(const char * server)
{
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = RPC_DEFAULT_PORT;
	server_addr.sin_addr.s_addr = LOCALHOST;
	if(server)
	{
		int s1,s2,s3,s4,port;
		switch(sscanf(server,"%d.%d.%d.%d:%d",&s1,&s2,&s3,&s4,&port))
		{
		case 5:
			server_addr.sin_port = port;
		case 4:
			server_addr.sin_addr.s_addr = MAKEINET(s1,s2,s3,s4);
			break;
		default:
			return -1;
		}
	}
	rpc_socket = socket(server_addr.sin_family,SOCK_DGRAM,0);
	if(rpc_socket <= 0)
		return -1;
	return connect(rpc_socket,(struct sockaddr*)&server_addr,INET_ADDRSTRLEN);
}

int distdb_rpc_disconnect()
{
	close(rpc_socket);
	rpc_socket = -1;
}

int distdb_rpc_execute_sql_bin(struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag)
{
	socklen_t	addrlen = INET_ADDRSTRLEN;
	char	buff[8192];
	struct rpc_packet_call * sbuff = (typeof(sbuff))buff;
	struct rpc_packet_ret * rbuff = (typeof(rbuff))buff;
	sbuff->rpc_call_id = DISTDB_RPC_EXECUTE_SQL_BIN;
	sbuff->call_seq = ++seq;
	struct execute_sql_bin * pdata = (typeof(pdata))(sbuff->data);
	pdata->length = length;
	pdata->flag = executeflag;
	memcpy(pdata->data, sql, length);

	if (sendto(rpc_socket, buff, length + SIZE_RPC_HEADER + SIZE_EXECUTE_SQL_BIN ,
			0,(struct sockaddr*) &server_addr, INET_ADDRSTRLEN) < 0)
		return -1;

	if (recvfrom(rpc_socket, buff, sizeof(buff), 0, (struct sockaddr*) &server_addr,&addrlen) < 0)
		return -1;

	if(rbuff->call_seq != seq)
		return -1;
	memcpy(out, rbuff->data, sizeof(void*)); // sir, this is the best way :)
	return rbuff->ret;
}

int distdb_rpc_execute_sql_str(struct DISTDB_SQL_RESULT ** out,const char *sql,int executeflag)
{
	return distdb_rpc_execute_sql_bin(out,sql,strlen(sql) +1 ,executeflag);
}

int distdb_rpc_free_result(struct DISTDB_SQL_RESULT *reslt)
{
	socklen_t	addrlen = INET_ADDRSTRLEN;
	char	buff[8192];
	struct rpc_packet_call * sbuff = (typeof(sbuff))buff;
	struct rpc_packet_ret * rbuff = (typeof(rbuff))buff;
	sbuff->rpc_call_id = DISTDB_RPC_FREE_RESLUT;
	sbuff->call_seq = ++seq;
	memcpy(sbuff->data,&reslt,sizeof(reslt));

	if (sendto(rpc_socket, buff, 8 + SIZE_RPC_HEADER ,
			0,(struct sockaddr*) &server_addr, INET_ADDRSTRLEN) < 0)
		return -1;

	if (recvfrom(rpc_socket, buff, sizeof(buff), 0, (struct sockaddr*) &server_addr,&addrlen) < 0)
		return -1;

	if(rbuff->call_seq != seq)
		return -1;
	return rbuff->ret;	 //:D
}

int main(int argc, char* argv[])
{
	printf("libdistdb -- The rpc call wrapper for distdb\n");
	printf("Copyright (C) 2009-2010 microcai %s\n",PACKAGE_BUGREPORT);
	printf("version %s compiled on %s %s\n",VERSION, __DATE__,__TIME__);
	exit(0);
}
