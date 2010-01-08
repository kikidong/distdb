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

__BEGIN_DECLS


struct _groupmap{
	int	id;
	char * name;
};

struct db_ops{
	int (*db_open)();
	int (*db_exec_sql)(void ** db_private_ptr,const char*,int len);
	int (*db_get_result)(void*	db_private_ptr);
	int (*db_fetch_row)(void*	db_private_ptr,char ***);
	int (*db_free_result)(void*	db_private_ptr);
	int (*db_close)();
};

LIST_SLOT_DECLARE(nodelist);
LIST_SLOT_DECLARE(node_unconnectedlist);
LIST_SLOT_DECLARE(node_connectedlist);

extern int				  g_rpc_socket;
extern	int				  g_socket;
extern int				  groupcount;
extern struct _groupmap *groupmap;
extern struct db_ops	  db;

__END_DECLS
#endif /* GLOBAL_VAR_H_ */
