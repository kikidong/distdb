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
#include "../include/db_def.h"
#include "../include/communication.h"

extern void * getbase()
{
	static struct {
		struct db_ops *dbops;
		struct distdb_info * info ;
	}ret={&db,&node_info};
	return &ret;
}

//void close_node(struct nodes* n)
//{
//	close(n->sock_peer);
//	n->sock_peer = 0;
//	LIST_DELETE_AT(&n->connectedlist);
//	LIST_ADDTOTAIL(&node_unconnectedlist,&n->unconnectedlist);
//}
//
void send_all(DISTDB_NODE * nodes, void* buff, size_t size, int flag)
{
	struct nodes * node;

	if (nodes)
	{
		node = (struct nodes *)(nodes[0]);
		while (node)
		{
			send(node->sock_peer, buff, size, flag);
			node++;
		}
	}
	else
	{
		struct list_node * n;
		pthread_mutex_lock(&nodelist_lock);

		for (n = nodelist.head; n != nodelist.tail->next; n = n->next)
		{
			node = LIST_HEAD(n,nodes,nodelist);
			send(node->sock_peer, buff, size, flag);
		}
		pthread_mutex_unlock(&nodelist_lock);
	}
}

int distdb_fetch_result(struct DISTDB_SQL_RESULT * in,char ** result[])
{
	return db.db_fetch_row(in,result);
}

int distdb_dfetch_result(struct DISTDB_SQL_RESULT * reslt,char * data, size_t * retsize)
{
	int i,retval;

	struct db_sql_result * srst = (typeof(srst))data;

//	srst->number = reslt->columns;

	char ** res;

	//retval = distdb_rpc_fetch_result(reslt,&res);

	if (retval==-1)
	{
		*retsize = 0;
		return -1;
	}

	//char * real_result = (char*)(srst->offsets + reslt->columns);

	*retsize = reslt->columns +1 ;
//
//	for( i = 0; i < srst->number ; ++i )
//	{
//		strcpy(real_result,res[i]);
//		srst->offsets[i] = real_result - data;
//		real_result += strlen(res[i])+1;
//		*retsize += strlen(res[i])+1;
//	}
	return retval;
}


int distdb_free_result(struct DISTDB_SQL_RESULT * p)
{
	db.db_free_result(p);
	db.db_close(p);
	free(p);
}
