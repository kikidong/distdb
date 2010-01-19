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



	}

<<<<<<< HEAD
=======

>>>>>>> king
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

int distdb_rpc_free_result(struct DISTDB_SQL_RESULT * p)
{
	db.db_free_result(p);
	db.db_close(p);
	free(p);
}
