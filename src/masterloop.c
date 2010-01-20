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

static int accepts(struct nodes ** pn)
{
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					sock;
	struct list_node *	n;

	sock = accept(g_socket,(__SOCKADDR_ARG)&addr,&addr_len);

	if (sock <= 0)
		return 0; //忽略错误就可以了

//	pthread_mutex_lock(&nodelist_lock);
	if(peer_lookup_same(addr.sin_addr.s_addr))
		close(sock);
	*pn = nodes_new();
	LIST_ADDTOTAIL(&nodelist,&(*pn)->nodelist);
	pthread_mutex_unlock(&nodelist_lock);
	return 0;
}

void * massive_loop( struct nodes * client)
{



}


void* service_loop(void*P)
{
	pthread_t	thread;

	struct pollfd pfd[1];
	pfd[0].fd = g_socket ;
	pfd[0].events = POLLIN ;

	struct nodes * pn;

	while (poll(pfd, 1, -1) == 1)
	{
		if(accepts(&pn))
			continue;
		pthread_create(&thread,0,(void *(*)(void *))massive_loop,pn);
	}

	return 0;
}
