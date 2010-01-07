/*
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

#include <fcntl.h>
#include <sqlite3.h>
#include "../include/inifile.h"
#include "../include/distdb.h"
#include "../include/global_var.h"

int main(int argc,char* argv[])
{
	printf("=============================================================\n");
	printf("%s -- The SQLite backend for distdb Version:%s\n",argc?argv[0]:"sqlite.so",VERSION);
	printf("Copyright (C) 2009-2010 microcai. All rights reserved\n");
 	printf("For more infomation see COPYING file shipped with this software.\n");
	printf("Copyright (C) 2009-2010 Kingstone, Ltd. All rights reserved\n");
	printf("Written by %s\n",AUTHOR);
	printf("For more infomation see COPYING file shipped with this software.\n");
	printf("=============================================================\n");
}

static char * dbfile;
static sqlite3	* pdb;

struct sqlite{
	char ** result;
	int row;
	int col;
	int current;
};

/*
 * Open database
 */
int opendb()
{
	return sqlite3_open(dbfile,&pdb);
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
static int execsql(void**out,const char * sql,int byte)
{
	char * errmsg;
	char ** result;
	int col;
	int row;
	struct sqlite	* forout;

	forout = NULL;

	if (sqlite3_get_table(pdb, sql, &result, &row, &col, &errmsg) == SQLITE_OK)
	{
		forout = malloc(sizeof(struct sqlite));
		forout->result = result;
		forout->col = col;
		forout->row = row;
		forout->current = 1;
	}
	if(errmsg)
	{
		sqlite3_free(errmsg);
	}
	*out = forout;
	if(forout)
		return 0;
	return -1;
}

static int get_result(void * ptr)
{
	return sqlite3_step((sqlite3_stmt*)ptr);
}

static int fetch_row(void* ptr,char** reslut)
{
	int i;
	struct sqlite * sp = (struct sqlite*) ptr;

	if( sp->current > sp->row)
		return -1;

	for(i=0;i<sp->col;i++)
	{
		strcpy(reslut[i],sp->result[sp->current*sp->col + i ]);
	}
	sp->current ++;
	return 0;
}

static int free_reslut(void*ptr)
{
	struct sqlite * sp = (struct sqlite*) ptr;
	sqlite3_free_table(sp->result);
	return 0;
}

void __init()
{
	printf(":P\n");
	/*
	 * Can't use global var, or it will failed to start
	 * when execute as separate program
	 */
	extern void * getbase();

	struct {
		struct db_ops *pdb;
		FILE ** file;
	} * base = getbase();

	struct db_ops * db = base->pdb;

	FILE * cf = *base->file;

	db->db_exec_sql = execsql;

	db->db_get_result = get_result;

	db->db_fetch_row = fetch_row;

	db->db_free_result = free_reslut;

	printf("sqlite backend loaded");
	sqlite3_initialize();
	return;
}
