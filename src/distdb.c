/*
 * distdb.c  - main implementation file
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

#include <sys/types.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/db_def.h"
#include "../include/communication.h"

void distdb_initalize()
{

}

/**
 * 这将导致监听 3721 端口，呵呵
 */
int distdb_enable_server(struct distdb_info *pdistdb_info, int retain)
{
	void* dbplugin;
	char * so_file;
	pthread_t pt;

	strncpy((char*)node_info.servername, pdistdb_info->servername, 33);
	if(pdistdb_info->node_file)
		node_info.node_file = strdup(pdistdb_info->node_file);
	node_info.groupid = pdistdb_info->groupid;
	node_info.backend = pdistdb_info->backend;

	switch(node_info.backend)
	{
	case 0:
		node_info.backend_info.sqlite3_backend_info.dbname = strdup(
				pdistdb_info->backend_info.sqlite3_backend_info.dbname);
		break;
	case 1:
		node_info.backend_info.mysql_backend_info.dbhost = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbhost);
		node_info.backend_info.mysql_backend_info.dbuser = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbuser);
		node_info.backend_info.mysql_backend_info.dbpass = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbpass);
		break;
	case 2:
		node_info.backend_info.occi_backend_info.dbhost = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbhost);
		node_info.backend_info.occi_backend_info.dbuser = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbuser);
		node_info.backend_info.occi_backend_info.dbpass = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbpass);
	}


	static char backend[][20] =
	{
	{ "sqlite" },
	{ "mysql" },
	{ "oracle" } };
	//加载本地数据库后端
	so_file = strdup(PLUINGDIR);
	so_file = realloc(so_file, strlen(so_file) + strlen(
			backend[pdistdb_info->backend]) + 20);
	strcat(so_file, backend[pdistdb_info->backend]);
	strcat(so_file, ".so");
	dbplugin = dlopen(so_file, RTLD_NOW);
	free(so_file);
	if(!dbplugin)
	{
		return -1;
	}
	//绑定监听端口
	if(open_nodes_socket())
		return -1;
	//建立监听服务
	return pthread_create(&pt,0,service_loop,0);
}

int distdb_execute_sql_str(DISTDB_NODE * nodes,struct DISTDB_SQL_RESULT ** out,const char *sql,int executeflag)
{
	return distdb_execute_sql_bin(nodes,out,sql,strlen(sql) +1 ,executeflag);
}

int distdb_execute_sql_bin(DISTDB_NODE * nodes,struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag)
{
	struct db_exchange_header * db_hdr;
	struct DISTDB_SQL_RESULT * res;

	void * db_private_ptr;

	int ret;

	*out = 0;

	res = (typeof(res)) malloc(sizeof(struct DISTDB_SQL_RESULT));
	pthread_mutex_init(&res->lock,0);


	if (node_type) //哈哈，client-only 模式是不可以进行本地操作的啦
		executeflag |= DISTDB_EXECSQL_NOLOCAL;



	//制作步骤: 首先，您需要在本地查找(如果允许的话，呵呵) :)
	if (!(executeflag & DISTDB_EXECSQL_NOLOCAL))
	{
		//本地查找
		db.db_open(res, executeflag & DISTDB_RPC_EXECSQL_ALLOWRECURSIVE);

		ret = db.db_exec_sql(res, sql, length);

		if (ret)
		{
			db.db_close(res);
			free(res);
			return ret;
		} // 本地找都会出错，就不必麻烦远程电脑了
	}

	// DISTDB_EXECSQL_NOSERVER 标志打开,但是，却是 direct call,也得执行！
	//用这种形式是最好的啦
	switch(executeflag & (DISTDB_EXECSQL_NOSERVER|DISTDB_EXECSQL_NOTDIRECTCALL|DISTDB_EXECSQL_NOBROADCAST))
	{
	case  DISTDB_EXECSQL_NOSERVER:
	case  DISTDB_EXECSQL_NOTDIRECTCALL: //自然是要处理的啦
		executeflag |= DISTDB_EXECSQL_NOBROADCAST;
	case 0:
		// 还要到远程电脑上整啊.. 真是的.
		// 简单的发送一下命令就好了吧，接着记录，以便收到的时候进行合理的插入。
		db_hdr = malloc(db_exchange_header_size + length + 2);

		memset(db_hdr, 0, db_exchange_header_size + length + 2);

		db_hdr->restptr = res;
		db_hdr->length = length;
		db_hdr->type = db_exchange_type_exec_sql;

		//远程的电脑不需要再次查找远程的远程电脑吧，嘿嘿
		db_hdr->exec_sql.execflag = executeflag | DISTDB_EXECSQL_NOTDIRECTCALL;
		memcpy(db_hdr->exec_sql.sql_command,sql,length);

		pthread_mutex_lock(&res->lock);
		send_all(nodes,db_hdr,db_exchange_header_size + length + 1,0);
		free(db_hdr);
		pthread_mutex_unlock(&res->lock);
		break;
	}

	if (executeflag & DISTDB_EXECSQL_NORESULT)
	{
		if(res)
			distdb_free_result(res);
		*out = NULL;
	}
	else
	{
		//LIST_ADDTOTAIL(&results, &res->resultlist);
		*out = res;
	}
	return ret;

}



