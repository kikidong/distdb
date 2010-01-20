/*
 * distdb.c  - main implementation file
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
#include <sys/socket.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/global_var.h"
#include "../include/distdb.h"

void distdb_initalize()
{

}

/**
 * 这将导致监听 3721 端口，呵呵
 */
int distdb_enable_server(struct distdb_info *pdistdb_info, int retain)
{
	void* dbplugin;
	char * so_file;
	pthread_t pt;

	strncpy((char*)node_info.servername, pdistdb_info->servername, 33);
	if(pdistdb_info->node_file)
		node_info.node_file = strdup(pdistdb_info->node_file);
	node_info.groupid = pdistdb_info->groupid;
	node_info.backend = pdistdb_info->backend;

	switch(node_info.backend)
	{
	case 0:
		node_info.backend_info.sqlite3_backend_info.dbname = strdup(
				pdistdb_info->backend_info.sqlite3_backend_info.dbname);
		break;
	case 1:
		node_info.backend_info.mysql_backend_info.dbhost = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbhost);
		node_info.backend_info.mysql_backend_info.dbuser = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbuser);
		node_info.backend_info.mysql_backend_info.dbpass = strdup(
				pdistdb_info->backend_info.mysql_backend_info.dbpass);
		break;
	case 2:
		node_info.backend_info.occi_backend_info.dbhost = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbhost);
		node_info.backend_info.occi_backend_info.dbuser = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbuser);
		node_info.backend_info.occi_backend_info.dbpass = strdup(
				pdistdb_info->backend_info.occi_backend_info.dbpass);
	}


	static char backend[][20] =
	{
	{ "sqlite" },
	{ "mysql" },
	{ "oracle" } };
	//加载本地数据库后端
	so_file = strdup(PLUINGDIR);
	so_file = realloc(so_file, strlen(so_file) + strlen(
			backend[pdistdb_info->backend]) + 20);
	strcat(so_file, backend[pdistdb_info->backend]);
	strcat(so_file, ".so");
	dbplugin = dlopen(so_file, RTLD_NOW);
	free(so_file);
	if(!dbplugin)
	{
		return -1;
	}
	//绑定监听端口
	if(open_nodes_socket())
		return -1;
	//建立监听服务
	pthread_create(&pt,0,service_loop,0);
}

