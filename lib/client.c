/*
 * client.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#include "./vcconfig.h"
#endif

#include "../include/distdb.h"
#include "../include/rpc.h"

static int rpc_socket = -1 ;
static struct sockaddr_in server_addr = {0};

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
	struct rpc_packet_call * buff = malloc(sizeof(struct rpc_packet_call) + length );

	buff->rpc_call_id =

	sendto(rpc_socket,0,0,0,(struct sockaddr*)&server_addr,INET_ADDRSTRLEN);

}

int distdb_rpc_execute_sql_str(struct DISTDB_SQL_RESULT ** out,const char *sql,int executeflag)
{
	return distdb_rpc_execute_sql_bin(out,sql,strlen(sql) +1 ,executeflag);
}

int main()
{
	printf("libdistdb -- The rpc call wrapper for distdb\n");
	printf("Copyright (C) 2009-2010 microcai %s\n",PACKAGE_BUGREPORT);
	printf("version %s compiled on %s %s\n",VERSION, __DATE__,__TIME__);
	exit(0);
}

