/*
 * global_var.h
 *
 *  Created on: 2009-12-26
 *      Author: cai
 */

#ifndef GLOBAL_VAR_H_
#define GLOBAL_VAR_H_

#include "list.h"
#include "nodes.h"
#include "db_def.h"

__BEGIN_DECLS


struct _groupmap{
	int	id;
	char * name;
};

struct db_ops{
	int (*db_open)(struct DISTDB_SQL_RESULT * res,int reopen);
	int (*db_exec_sql)(struct DISTDB_SQL_RESULT*,const char*,int len);
	int (*db_get_result)(struct DISTDB_SQL_RESULT*);
	int (*db_fetch_row)(struct DISTDB_SQL_RESULT*,char ***); // 0 for ok, -1 for no more
	int (*db_peek_row)(struct DISTDB_SQL_RESULT*,char ***); // 0 for ok, -1 for no more, 1 for time out
	int (*db_free_result)(struct DISTDB_SQL_RESULT*);
	int (*db_close)(struct DISTDB_SQL_RESULT*);
};

LIST_SLOT_DECLARE(nodelist);
extern pthread_mutex_t nodelist_lock;
extern int				  groupid; //本组号

extern	void*			  zeropage;
extern int				  g_rpc_socket;
extern	int				  g_socket;
extern int				  groupcount;
extern struct _groupmap *groupmap;
extern struct db_ops	  db;

extern int node_type;
extern struct distdb_info	node_info;

__END_DECLS
#endif /* GLOBAL_VAR_H_ */
