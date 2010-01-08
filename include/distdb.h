/*
 * distdb.h  -- massive include file
 *
 * Copyright (C) 2009-2010 Kingstone, Ltd.
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
 * If you have any question with law suite, please contract 黄小克 , the owner of
 * this company.
 */

#ifndef __DISTDB_H_
#define __DISTDB_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HAVE_GETLINE

static inline size_t getline (char ** __lineptr,size_t * __n,FILE * __stream)
{
	int malloc_called = 1024;
	if(!*__lineptr)
		malloc(malloc_called++);
	if(fgets(*__lineptr,1024,__stream))
		return (*__n = strlen(*__lineptr));
	else if(malloc_called>1024)
		free(*__lineptr);
	return * __n = 0;
}

#endif

#ifndef MIN
#define MIN(x,y)	( (x) < (y) ? (x):(y) )
#endif

#define ZEROWITHSIZE(structure)	memset(&structure,0,sizeof(structure));

#define MAKEINET(s1,s2,s3,s4)	(((s1)& 0xFF)<< 24 ) |(((s2)& 0xFF)<< 16 )|(((s3)& 0xFF)<< 8 )|((s4)& 0xFF)

#define LOCALHOST	MAKEINET(127,0,0,1)

__BEGIN_DECLS

struct DISTDB_SQL_RESULT;
typedef struct DISTDB_SQL_RESULT DISTDB_SQL_RESULT;

const char* distdb_version();
int distdb_rpc_connectto(const char * server);
int distdb_rpc_disconnect();

/*
 * executeflag
 */

enum executeflag{
	//force distdb to send request to other computers
	DISTDB_RPC_EXECSQL_NOLOCAL	= 0x00000001,
#define DISTDB_RPC_EXECSQL_NOLOCAL DISTDB_RPC_EXECSQL_NOLOCAL
#define DISTDB_RPC_EXECSQL_SERVERONLY DISTDB_RPC_EXECSQL_NOLOCAL

	//force distdb to lookup locally
	DISTDB_RPC_EXECSQL_NOSERVER = 0x00000002,
#define DISTDB_RPC_EXECSQL_NOSERVER	DISTDB_RPC_EXECSQL_NOSERVER
#define DISTDB_RPC_EXECSQL_LOCALONLY	DISTDB_RPC_EXECSQL_NOSERVER

	//force distdb to discard results
	DISTDB_RPC_EXECSQL_NORESULT = 0x00000004 ,
#define	DISTDB_RPC_EXECSQL_NORESULT DISTDB_RPC_EXECSQL_NORESULT

	//Make sure we can make multiple sql calls simultaneously
	DISTDB_RPC_EXECSQL_ALLOWRECURSIVE = 0x00010000
#define DISTDB_RPC_EXECSQL_ALLOWRECURSIVE DISTDB_RPC_EXECSQL_ALLOWRECURSIVE

};

int distdb_rpc_execute_sql_str(struct DISTDB_SQL_RESULT ** out,const char * sql, int executeflag);
int distdb_rpc_execute_sql_bin(struct DISTDB_SQL_RESULT ** out,const char *,size_t length,int executeflag);

// One call , one row, more row ? call more
int distdb_rpc_fetch_result(struct DISTDB_SQL_RESULT * in ,char ** result[]);
int distdb_rpc_free_result(struct DISTDB_SQL_RESULT *);

__END_DECLS

#endif /* __DISTDB_H_ */
