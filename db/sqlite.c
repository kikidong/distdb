/*
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sqlite3.h>
#include "../include/distdb.h"
#include "../include/global_var.h"
#include "../include/db_def.h"

int main(int argc,char* argv[])
{
	printf("=============================================================\n");
	printf("%s -- The SQLite backend for distdb Version:%s\n",argc?argv[0]:"sqlite.so",VERSION);
	printf("Copyright (C) 2009-2010 Kingstone, Ltd. All rights reserved\n");
	printf("Written by %s\n",AUTHOR);
	printf("For more infomation see COPYING file shipped with this software.\n");
	printf("=============================================================\n");
}

static char * dbfile;
static sqlite3	* pdb;

struct sqlite{
	sqlite3	* pdb;
	char ** result;
	int row;
	int col;
	int current;
	char **currentline;
};

/**
 * Open database
 */
int opendb(struct DISTDB_SQL_RESULT * res,int reopen)
{
	res->db_private_ptr = malloc(sizeof(struct sqlite));
	struct sqlite * p =(typeof(p))(res->db_private_ptr);

	res->needclose = reopen;

	if(reopen)
		return sqlite3_open(dbfile,&p->pdb);
	p->pdb = pdb;
	return 0;
}

/**
 * @brief 关闭打开的一个连接
 */
int db_close(struct DISTDB_SQL_RESULT*res)
{
	struct sqlite * p =(typeof(p))(res->db_private_ptr);
	if(res->needclose)
		sqlite3_close(p->pdb);
	free(res->db_private_ptr);
	return 0;
}

/*
 * Check tables and other stuff
 */
int checkdb(char * tables[])
{

	return 0;
}

/*
 * 执行 sql 语句
 */
static int execsql(struct DISTDB_SQL_RESULT* res,const char * sql,int byte)
{
	char * errmsg;
	char ** result;
	int col;
	int row;
	struct sqlite	* forout =(typeof(forout))(res->db_private_ptr);

	if (sqlite3_get_table(forout->pdb, sql, &result, &row, &col, &errmsg) == SQLITE_OK)
	{
		forout->col = res->colums = col;
		forout->result = result;
		forout->row = row;
		forout->current = 1;
		forout->currentline = calloc(col,sizeof(char*));
		return 0;
	}
	if(errmsg)
	{
		sqlite3_free(errmsg);
	}
	return -1;
}

static int fetch_row(struct DISTDB_SQL_RESULT * res,char*** reslut)
{
	int i;
	struct sqlite * sp = (struct sqlite*) (res->db_private_ptr);
	*reslut = sp->currentline;

	if( sp->current > sp->row)
		return -1;

	for(i=0;i<sp->col;i++)
	{
		sp->currentline[i] = sp->result[sp->current*sp->col + i ];
	}
	sp->current ++;
	return 0;
}

static int free_reslut(struct DISTDB_SQL_RESULT * res)
{
	struct sqlite * sp = (struct sqlite*) res->db_private_ptr;
	sqlite3_free_table(sp->result);
	return 0;
}

void __init()
{
	/*
	 * Can't use global var, or it will failed to start
	 * when execute as separate program
	 */
	extern void * getbase();

	struct {
		struct db_ops *pdb;
		struct distdb_info * info;
	} * base = getbase();

	struct db_ops * db = base->pdb;

	db->db_exec_sql = execsql;

	db->db_peek_row = db->db_fetch_row = fetch_row; // 不需要，呵呵

	db->db_free_result = free_reslut;

	db->db_open = opendb ;

	db->db_close = db_close;

	dbfile = strdup(base->info->backend_info.sqlite3_backend_info.dbname);

	sqlite3_open(dbfile,&pdb);

	printf("sqlite backend loaded\n");
	return;
}
