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
#include "../include/communication.h"

extern void * getbase()
{
	static struct
	{
		struct db_ops *dbops;
		struct distdb_info * info;
	} ret =
	{ &db, &node_info };
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
int send_all(DISTDB_NODE * nodes, void* buff, size_t size, int flag)
{
	int count;
	struct nodes * node;
	count = 0;

	if (nodes)
	{
		node = (struct nodes *) (nodes[0]);
		while (node)
		{
			send(node->sock_peer, buff, size, flag);
			node++;
			count++;
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
			count++;
		}
		pthread_mutex_unlock(&nodelist_lock);
	}
	return count;
}

int distdb_fetch_result_local(struct DISTDB_SQL_RESULT * in, char ** result[])
{
	return db.db_fetch_row(in, result);
}

int distdb_fetch_result(struct DISTDB_SQL_RESULT * in, char ** result[])
{
	//获得一个结果，而不管所在地.
	char *data;
	int ret;
	ret = db.db_peek_row(in, result);
	if (ret == 0)
		return 0;
	//没有数据,看远程的有没有哈.
	return distdb_fetch_result_remote(in, result);
}

/**
 * 获得远程返回的结果，
 */
int distdb_fetch_result_remote(struct DISTDB_SQL_RESULT * reslt,
		char ** result[])
{
	struct sql_result_plain_text * ptext;
	pthread_mutex_lock(&reslt->lock);
	if (LIST_ISEMPTY(reslt->sql_result))
	{
		if (reslt->ref) //还有结果会陆续进来，呵呵
			pthread_cond_wait(&reslt->waitcond, &reslt->lock);
	}
	if (reslt->ref == 0)
		return -1;

	if (reslt->last)
		free(reslt->last);

	ptext = LIST_HEAD(reslt->sql_result.head,sql_result_plain_text,resultlist);
	LIST_DELETE_AT(&ptext->resultlist);
	reslt->last = ptext;
	pthread_mutex_unlock(&reslt->lock);
	return 0;
}

int distdb_free_local_result(struct DISTDB_SQL_RESULT * p)
{
	db.db_free_result(p);
	db.db_close(p);
	p->needclose = -1;
}

int distdb_free_remote_result(struct DISTDB_SQL_RESULT * p)
{
	struct list_node *n;
	pthread_mutex_lock(&p->lock);
	p->ref--;
	if (p->ref <= 0)
	{ //free 掉链表
		for (n = p->sql_result.head; n != p->sql_result.tail->next; n++)
		{
			free(LIST_HEAD(n,sql_result_plain_text,resultlist));
		}
		free(p);
	}
	pthread_mutex_unlock(&p->lock);
}

int distdb_free_result(struct DISTDB_SQL_RESULT*p)
{
	pthread_mutex_lock(&p->lock);
	if (p->needclose != -1)
		distdb_free_local_result(p);
	p->ref = 1;
	pthread_mutex_unlock(&p->lock);
	distdb_free_remote_result(p);
}

