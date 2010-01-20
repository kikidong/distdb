/*
 * nodes_op - ops for nodes
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
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define __DISTDB_SERVER_SIDE_H

#include "../include/global_var.h"
#include "../include/distdb.h"
#include "../include/rpc.h"
#include "../include/db_def.h"


DISTDB_NODE distdb_connect(const char* server)
{
	int sock_peer;
	struct sockaddr_in peer;
	struct nodes * newnode;
	//连接并登录
	memset(peer.sin_zero, 0, sizeof(peer.sin_zero));

	peer.sin_family = AF_INET;
	peer.sin_port = DISTDB_DEFAULT_PORT;
	peer.sin_addr.s_addr = resoveserver(server, &peer.sin_port);
	peer.sin_port = htons(peer.sin_port);

	pthread_mutex_lock(&nodelist_lock);
	if (peer_lookup_same(peer.sin_addr.s_addr)) //已经连接的就不再重复连接
	{
		pthread_mutex_unlock(&nodelist_lock);
		return 0;
	}
	sock_peer = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_peer < 0)
		return 0;

	if (connect(sock_peer, (const struct sockaddr *) &peer, INET_ADDRSTRLEN)
			< 0)
	{
		close(sock_peer);
		pthread_mutex_unlock(&nodelist_lock);
		return 0;
	}

	newnode = nodes_new();
	newnode->peer = peer;
	LIST_ADDTOTAIL(&nodelist, &newnode->nodelist);
	pthread_mutex_unlock(&nodelist_lock);

	if (distdb_login(newnode))
	{
		//登录失败!从列表中移除
		LIST_DELETE_AT(&newnode->nodelist);
		newnode->freeer(newnode);
	}
	return (DISTDB_NODE) newnode;
}

int start_connect_nodes()
{
	do
	{
		// 随机 休息 0 - 20 秒
		sleep(rand() % 20);
		//struct nodes * n;
		pthread_t pt;

		struct list_node * n;

		pthread_mutex_lock(&nodelist_lock);

//		for (n = node_unconnectedlist.head; n
//				!= node_unconnectedlist.tail->next; n = n->next)
//		{
//			pthread_t pt;
//			if (!LIST_HEAD(n,nodes,unconnectedlist)->sock_peer) // only connect unconnected.
//				pthread_create(&pt, 0, (void *(*)(void *)) connect_peer,
//						LIST_HEAD(n,nodes,unconnectedlist));
//			//else n has been off link. Thank good ness, the n->next still works
//		}
		pthread_mutex_unlock(&nodelist_lock);
	} while (1);
	return 0;
}

int open_nodes_socket()
{
	int opt = 1;
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = DISTDB_DEFAULT_PORT;
	g_socket = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(g_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(g_socket, (struct sockaddr*) &addr, INET_ADDRSTRLEN) < 0)
	{
		close(g_socket);
		g_socket = -1;
		return -1;
	}
	return listen(g_socket, 20);
}

int peer_lookup_same(in_addr_t ip)
{
	struct list_node * n;

	pthread_mutex_lock(&nodelist_lock);

	for (n = nodelist.head; n
			!= nodelist.tail->next; n = n->next)
	{
		if(LIST_HEAD(n,nodes,nodelist)->peer.sin_addr.s_addr == ip)
		{
			pthread_mutex_unlock(&nodelist_lock);
			return 1;
		}
	}
	pthread_mutex_unlock(&nodelist_lock);
	return 0;
}

/**
 * @brief 执行登录
 *
 * 成功登录返回 0，登录失败返回 -1
 */
int distdb_login(struct nodes * node)
{
	char sendbuf[1024];
	static char type[][16] = { { "client-only" },{ "server" } };
	int len;

	char ret_ok[16];
	char ret_name[33];
	char ret_type[32];
	int ret_groupid;


	len = sprintf(sendbuf, "ACTION:LOGIN NAME:%s TYPE:%s GROUP:%d\n\n",
			node_info.servername, type[node_type], node_info.groupid);
	send(node->sock_peer, sendbuf, len, 0);

	//FIXME 加入超时机制
	recv(node->sock_peer,sendbuf,sizeof(sendbuf),0);

	if (sscanf(sendbuf, "LOGIN:%15[^ ] NAME:%32[^ ] TYPE:%31[^ ] GROUP:%d\n\n",
			ret_ok, ret_name, ret_type, &ret_groupid) != 4)
		return -1;
	if(ret_ok[0]!='O' || ret_ok[1]!='K')
		return -1;

	if (strcmp(ret_type, "server") == 0)
		node->type = 0;
	else
		node->type = 1;

	node->groupid = ret_groupid;
	node->lastactive = time(0);
	return 0;
}
