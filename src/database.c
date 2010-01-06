/*
 * database.c - wapper for many database back-end
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

#include <limits.h>
#include <dlfcn.h>

#define __DISTDB_SERVER_SIDE_H
#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/inifile.h"

struct DISTDB_SQL_RESULT{
	time_t	time;	// The time that request the execution
	DEFINE_LIST(resultlist); // The results must be linked together. :)
	struct	the_db_ops * db; // the db opetator
	void*	db_private_ptr;
	// the severs that receives the same requeset
	int		columns; // columns that may return

};

static LIST_SLOT_DEFINE(results);

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

int opendb()
{

	return 0;
}

/*
 * Server side :)
 */
int distdb_rpc_execute_sql_bin(struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag)
{

	struct DISTDB_SQL_RESULT * res;

	void * db_private_ptr;

	*out = 0;

	db.db_exec_sql(&db_private_ptr,sql,length);

	res = (typeof(res))malloc(sizeof(struct DISTDB_SQL_RESULT));

	LIST_ADDTOTAIL(&results,&res->resultlist);

	*out = res;
	return 0;
}

int distdb_rpc_fetch_result(struct DISTDB_SQL_RESULT * in,char * result[])
{
	return db.db_fetch_row(in->db_private_ptr,result);
}

int distdb_rpc_free_result(struct DISTDB_SQL_RESULT * p)
{
	LIST_DELETE_AT(&p->resultlist);
	db.db_free_result(p->db_private_ptr);
	free(p);
}


