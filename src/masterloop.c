/*
 * masterloop.c - home to the loop
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


/**
 * Master loop --
 * 1) listen on local rpc port and accept rpc connections
 * 2) listen on local service port and accept other nodes' connection
 */
#include <sys/poll.h>
#include <pthread.h>
#include "../include/global_var.h"
#include "../include/rpc.h"
#include "../include/communication.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER ;

static int accepts(struct nodes ** pn)
{
	//只许可接受定义的节点的连接，希望将来可以改变这一事实。
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					sock;
	struct list_node *	n;

	sock = accept(g_socket,(__SOCKADDR_ARG)&addr,&addr_len);

	if (sock <= 0)
		return 0; //忽略错误就可以了

	//lock
	pthread_mutex_lock(&lock);
	//look up the same peer node
	for (n = node_connectedlist.head; n != node_connectedlist.tail->next; n
			= n->next)
	{
		if (SAME_PEER(&addr, &(LIST_HEAD(n,nodes,connectedlist)->peer)))
		{
			LIST_HEAD(n,nodes,connectedlist)->peer = addr;
			close(LIST_HEAD(n,nodes,connectedlist)->sock_peer);
			LIST_HEAD(n,nodes,connectedlist)->sock_peer = sock;
			*pn = LIST_HEAD(n,nodes,connectedlist);
			pthread_mutex_unlock(&lock);
			return 0;
		}
	}

	//if not found in connectedlist, look in the unconnectedlist
	for (n = node_unconnectedlist.head; n != node_unconnectedlist.tail->next; n= n->next)
	{
		if (SAME_PEER(&addr, &(LIST_HEAD(n,nodes,unconnectedlist)->peer)))
		{
			LIST_DELETE_AT(n);

			*pn = LIST_HEAD(n,nodes,unconnectedlist);

			LIST_ADDTOTAIL(&node_connectedlist, &(*pn)->connectedlist);
			LIST_HEAD(n,nodes,unconnectedlist)->peer = addr;
			LIST_HEAD(n,nodes,unconnectedlist)->sock_peer = sock;

			pthread_mutex_unlock(&lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&lock);
	//if not found in both place
	write(sock, "Permission denied\n", 18, MSG_DONTWAIT);
	close(sock);
	return -1;
}

/**
 * 主要的进行服务器的函数
 */
void* service_loop(struct nodes * clientnode)
{
	int sock;
	sock = clientnode->sock_peer ;





	while(1){}
	return 0;
}


/**
 * 这个函数完成对到来的连接进行的。
 */
static void* master_service(void*not_used)
{
	struct nodes	*   clientnode;

	if (accepts(&clientnode))
		return 0;
	// ok ,开始进行服务吧
	return service_loop(clientnode);
}

int event_loop()
{
	int ret;
	struct pollfd pfd[2];
	// use poll
	pfd[0].fd = g_rpc_socket ;
	pfd[1].fd = g_socket ;
	pfd[0].events = pfd[1].events = POLLERR| POLLHUP | POLLIN ;
	while( (ret = poll(pfd,2,-1)) )
	{
		while( ret--  )
		{
			if(pfd[ret].fd == g_socket)
			{
				//! 新开一个线程来处理
				pthread_t	thread;
				pthread_create(&thread,0,master_service,0);
			}else if (pfd[ret].fd == g_rpc_socket)
			{ //! 新开一个线程来处理新RPC客户的连接
				pthread_t	thread;

				struct	paramter{
					struct sockaddr_in addr;
					int 	sock;
					socklen_t	addr_len;
				}*paramter = (typeof(paramter)) malloc(sizeof(*paramter));

				paramter->addr_len = INET_ADDRSTRLEN ;

				paramter->sock = accept(g_rpc_socket,(__SOCKADDR_ARG)&paramter->addr,&paramter->addr_len);
				if(paramter->sock >0)
					pthread_create(&thread,0,rpc_loop_thread,paramter);
				else
					free(paramter);
			}
		}
		pfd[0].fd = g_rpc_socket ;
		pfd[1].fd = g_socket ;
		pfd[0].events = pfd[1].events = POLLERR| POLLHUP | POLLIN ;
	}

	return 0;
}
