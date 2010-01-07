/*
 * nodes.h - node defines
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

#ifndef NODES_H_
#define NODES_H_

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "list.h"

__BEGIN_DECLS

struct nodes{
	size_t		refcount;		 // refcount
	void (*freeer)();			 // should call when free.
	DEFINE_LIST(ticklist); // update the tick every beat
	int			lastactive;
	int			effectiveresponse;
	DEFINE_LIST(nodelist);
	DEFINE_LIST(connectedlist);
	DEFINE_LIST(unconnectedlist);
	int					sock_peer;   // end point socket describe
 	struct sockaddr_in	peer;			// end point
 	int			groupid;		// groupid
 	DEFINE_LIST(grouplist); // linked list that binds same group together.
};

static inline struct nodes * nodes_new()
{
	struct nodes * ret = (typeof(ret))malloc(sizeof(*ret));
	memset(ret,0,sizeof(*ret));
	ret->freeer = free;
	return ret;
}

static inline int SAME_PEER(struct sockaddr_in	*peer1,struct sockaddr_in *	peer2)
{
	return peer1->sin_addr.s_addr == peer2->sin_addr.s_addr;
}

__END_DECLS
#endif /* NODES_H_ */
