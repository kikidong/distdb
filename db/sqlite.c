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
#include "../include/inifile.h"
#include "../include/distdb.h"
#include "../include/global_var.h"

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
int execsql(void**out,const char * sql,int byte)
{
	const char * tail;
	sqlite3_stmt* stml;
	sqlite3_prepare_v2(pdb,sql,byte,&stml,&tail);
	*out = stml;
}


void __init()
{
	printf(":P\n");
//	get_profile_string(0,0,0,0,0);
	struct db_ops * getbase();
	struct db_ops * db = getbase();

	db->db_exec_sql = execsql;
//	db->db_fetch_row =

	printf("%p\n",db);
	printf(":D\n");
	return;
}
