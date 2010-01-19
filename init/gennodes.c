/*
 * gennodes.c - generate nodes from node file
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
 *
 */
#ifdef HAVE_CONFIG_H
#include  "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>

#define __DISTDB_SERVER_SIDE_H

#include "../include/nodes.h"
#include "../include/global_var.h"
#include "../include/distdb.h"


/*
 * seek to a chunk {}
 */
static int seek_chunk(FILE*file, const char * block)
{
	char	tmplate[1024];
	size_t	n = 1024;
	char*	linebuf = malloc(n);
	char*	p;

	size_t bytesread;

	while (!feof(file) && (bytesread=getline(&linebuf, &n, file)) != -1)
	{
		p = linebuf;
		while (*p == ' ' || *p == '\t' || *p=='\n')
			++p;

		if(*p=='#') continue;

		if(strncmp(p, block, MIN(strlen(block),n- (p - linebuf))))
			continue;
		fseek(file, strlen(block) -bytesread , SEEK_CUR);
		break;
	}
	free(linebuf);
	return 0;
}

/*
 * return 1 if broken file
 */
int check_node_file(const char * szfile)
{
	FILE * file = fopen(szfile,"r");

	if (!file)
		return 1; // cannot open ? broken!

	// TODO : check the format


	return 0;
}

/*
 * scan nodes from line
 */
static int nodesscanf( const char* line , void * ptr)
{
	int		s1,s2,s3,s4 ,port;
	struct nodes * new_node = nodes_new();

	switch(sscanf(line, "%d.%d.%d.%d:%d", &s1, &s2, &s3, &s4, &port))
	{
	case 4:
		port = RPC_DEFAULT_PORT;
	case 5:
		new_node->peer.sin_family = AF_INET;
		new_node->peer.sin_port = port;
		new_node->peer.sin_addr.s_addr = MAKEINET(s1,s2,s3,s4);
		ZEROWITHSIZE(new_node->peer.sin_zero);
		break;
	default:
		free(new_node);
		return -1;
	}
	new_node->refcount = 2; // referenced by 2 lists
	LIST_ADDTOHEAD(&nodelist,&new_node->nodelist);
	LIST_ADDTOTAIL(&node_unconnectedlist,&new_node->unconnectedlist);
	return 0;
}

/*
 * chunk_read
 */
static int chunk_read(FILE* file,int (*_readline)(const char * line,void * userptr ), const char * userptr )
{
	size_t n = 4096;
	int ret;
	char * linebuf = malloc(4096);
	char *p;
	int found=0;

	while (memset(linebuf, 0, 4096) && !feof(file) && getline(&linebuf, &n, file) != -1)
	{
		p = linebuf;
		while (*p == ' ' || *p == '\t' || *p == '\n')
			++p;
		if (*p && *p!='#')
		{
			if(found == 0)
			{
				// seek "{"
				if (*p != '{')
				{
					ret = -1;
					break;
				}else
				{
					found = 1;
				}
			}
			else if (found == 1 && *p == '}')
			{
				ret = 0;
				break;
			}
			else
			{
				if((ret = _readline(p,(void*)userptr)))
					break;
			}
		}
	}
	free(linebuf);
	return ret;

}

static int sscan_maps(const char * line , void * ptr)
{
	char * cuptr = * (char**) ptr;
	int id;
	char * name = cuptr;
	if(sscanf(line,"%[^ \t]%d",name,&id)!=2)
		return -1;
	cuptr += strlen(name) + 1;
	* (char**) ptr = cuptr ;

	groupmap--;
	groupmap->id = id;
	groupmap->name = name;
	groupcount++;
	return 0;
}

/*
 * read nodes into memory
 */
int read_nodes(const char * nodes_file)
{
	static char*	  groupmap_back_store;

	int ret;
	size_t n = 1024;
	char * linebuf;
	char * p;
	int found;

	char * cuptr ;

		// open the file
	FILE * nf = fopen(nodes_file, "r");
	if (!nf)
		return -1;
	linebuf = malloc(n);

	//read the group id to name map
	seek_chunk(nf, "groupmap");
	//

	groupmap_back_store = malloc(4096);
	cuptr = groupmap_back_store;
	groupmap = (typeof(groupmap)) (groupmap_back_store+4096);

	chunk_read(nf,sscan_maps,(void*)&cuptr);
				// groupid to name map;
				//char * p = groupmap_back_store;
	// for every groupname, read the groups
	int i;

	for (i = 0; i < groupcount; ++i)
	{
		char	chunk[500];
		int		id = groupmap[i].id ;
		fseek(nf,0,0);

		snprintf(chunk,499,"group %s",groupmap[i].name);

		seek_chunk(nf,chunk);
		chunk_read(nf,nodesscanf,0);
	}

	free(linebuf);
	return ret;
}


/*
 * connet to ip and return the fd
 */
static int connectto(struct sockaddr_in * peer)
{
	int ret;
	int sk = socket(peer->sin_family,SOCK_STREAM,0);

	fcntl(sk,F_SETFL,fcntl(sk,F_GETFL)|O_NONBLOCK);

	if(sk > 0)
		ret =  connect(sk,(struct sockaddr*)peer,INET_ADDRSTRLEN);
	else
		return -1;
	if(ret && errno != EINPROGRESS)
		close(sk);
	return ret;
}

/*
 * connect to peers
 */
static int connect_peer(struct nodes * node)
{
	pthread_t pt;
	//connect
	node->sock_peer = connectto(&node->peer);
	//link to connected list

	node->refcount ++;
	LIST_ADDTOTAIL(&node_connectedlist,& node->connectedlist);
	LIST_DELETE_AT(&node->unconnectedlist);
	node->refcount --;
	pthread_create(&pt,0,(void *(*) (void *))service_loop,node);
}

int connect_nodes()
{
	//struct nodes * n;
	pthread_t pt;

	struct list_node * n;

	//listen on local ports
	int opt = 1;
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = RPC_DEFAULT_PORT;
	g_socket = socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(g_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bind(g_socket,(struct sockaddr*)&addr,INET_ADDRSTRLEN);
	listen(g_socket,20);

//	pthread_create(&pt, 0, accepts, 0);

	for (n = node_unconnectedlist.head ; n  != node_unconnectedlist.tail->next  ; n = n->next)
	{
	//	pthread_mutex_lock(&lock);
		if(! LIST_HEAD(n,nodes,unconnectedlist)->sock_peer) // only connect unconnected.
			connect_peer(LIST_HEAD(n,nodes,unconnectedlist));
		//else n has been off link. Thank good ness, the n->next still works
	//	pthread_mutex_unlock(&lock);
	}
	return 0;
}
