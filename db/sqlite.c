/*
 * sqlite.c
 *
 *  Created on: 2010-1-2
 *      Author: cai
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <sqlite3.h>
#include "../init/inifile.h"
#include "../include/distdb.h"

int onload()
{
//	sqlite3_open()
	printf("called\n");
	char v[200];
 	get_profile_string(0,0,"sqlite",v,200);
	return 0;
}

int main()
{
	printf("loaded!");

}

static char * dbfile;
static sqlite3	* db;


/*
 * Open database
 */
int opendb()
{
	return sqlite3_open(dbfile,&db);
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
int execsql(const char * sql,int byte)
{
	const char * tail;
	sqlite3_stmt* stml;
	sqlite3_prepare_v2(db,sql,byte,&stml,&tail);
}
