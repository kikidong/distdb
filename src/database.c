/*
 * database.c - wapper for many database back-end
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
#endif

#include <limits.h>
#include <dlfcn.h>

#define __DISTDB_SERVER_SIDE_H
#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/inifile.h"
#include "../include/db_def.h"
#include "../include/rpc.h"
#include "../include/communication.h"

static LIST_SLOT_DEFINE(results);
static FILE * cf;

extern void * getbase()
{
	static struct {
		struct db_ops *pdb;
		FILE ** file;
	}ret={&db,&cf};
	return &ret;
}

int	load_plugins(const char * configfile)
{
//	dbplugin = dlopen("sqlite.so",RTLD_NOW);

	cf = fopen(configfile,"r");
	if(!cf)
	{
		return -1;
	}

	char backend[128] = "sqlite";
	void * dbplugin;
	char * so_file;

	get_profile_string(cf,"global","backend",backend,sizeof(backend));

	so_file = strdup(PLUINGDIR);
	so_file = realloc(so_file,strlen(so_file)+ strlen(backend) + 1 );
	dbplugin = dlopen(so_file, RTLD_NOW);
	free(so_file);
	if(!dbplugin)
	{
		fprintf(stderr,dlerror());
		close(cf);
		return -1;
	}
	close(cf);
	return 0;
}

extern void * getbase()
{
	return &db;
}

int	load_plugins(const char * configfile)
{
//	dbplugin = dlopen("sqlite.so",RTLD_NOW);

	FILE *cf = fopen(configfile,"r");
	if(!cf)
	{
		return -1;
	}

	char backend[128] = "sqlite";
	void * dbplugin;
	char * so_file;

	get_profile_string(cf,"global","backend",backend,sizeof(backend));

	so_file = strdup(PLUINGDIR);

	so_file = realloc(so_file,strlen(so_file)+ strlen(backend) + 20 );
	strcat(so_file,backend);
	strcat(so_file,".so");
	dbplugin = dlopen(so_file, RTLD_NOW);
	free(so_file);
	if(!dbplugin)
	{
		fprintf(stderr,dlerror());
		close(cf);
		return -1;
	}
	close(cf);
	return 0;
}

void close_node(struct nodes* n)
{
	close(n->sock_peer);
	n->sock_peer = 0;
	LIST_DELETE_AT(&n->connectedlist);
	LIST_ADDTOTAIL(&node_unconnectedlist,&n->unconnectedlist);
}

int send_all(void* buff,size_t size,int flag)
{
	struct list_node * n;
	int ret = 0;
	struct nodes * node;

	pthread_mutex_lock(&nodelist_lock);
	for(n=node_connectedlist.head;n!=node_connectedlist.tail->next;n=n->next)
	{
		node = LIST_HEAD(n,nodes,connectedlist);
		if(send(node->sock_peer,buff,size,flag)<0);
		{
			close_node(node);
		}
	}
	pthread_mutex_unlock(&nodelist_lock);
	return ret;
}



/*
 * Server side :)
 */
int distdb_rpc_execute_sql_bin(struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag)
{

	struct DISTDB_SQL_RESULT * res;

	void * db_private_ptr;

	int ret;

	*out = 0;

	res = (typeof(res)) malloc(sizeof(struct DISTDB_SQL_RESULT));

	if (!(executeflag & DISTDB_RPC_EXECSQL_NOLOCAL))
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

	if (! ( executeflag & DISTDB_RPC_EXECSQL_NOSERVER))
	{
		// 还要到远程电脑上整啊.. 真是的.
		//TODO 远程查找
		// 简单的发送一下命令就好了吧
		struct db_exchange_header * db_hdr = malloc(db_exchange_header_size
				+ length + 2);

		memset(db_hdr, 0, db_exchange_header_size + length + 2);

		db_hdr->restptr = res;
		db_hdr->length = length;
		db_hdr->type = db_exchange_type_exec_sql;

		//远程的电脑不需要再次查找远程的远程电脑吧，嘿嘿
		db_hdr->exec_sql.execflag = executeflag | DISTDB_RPC_EXECSQL_NOSERVER;
		memcpy(db_hdr->exec_sql.sql_command,sql,length);

		send_all(db_hdr,db_exchange_header_size + length + 1,0);

		free(db_hdr);
	}

	if (executeflag & DISTDB_RPC_EXECSQL_NORESULT)
	{
		if(res)
			distdb_rpc_free_result(res);
		*out = NULL;
	}
	else
	{
		LIST_ADDTOTAIL(&results, &res->resultlist);
		*out = res;
	}
	return ret;
}

int distdb_rpc_fetch_result(struct DISTDB_SQL_RESULT * in,char ** result[])
{
	return db.db_fetch_row(in,result);
}

int distdb_fetch_result(struct DISTDB_SQL_RESULT * reslt,char * data, size_t * retsize)
{
	int i,retval;

	struct db_sql_result * srst = (typeof(srst))data;

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


int distdb_rpc_free_result(struct DISTDB_SQL_RESULT * p)
{
	db.db_free_result(p);
	db.db_close(p);
	free(p);
}
