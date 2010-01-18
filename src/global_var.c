/*
 * global_var.c - home to the global variable
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

#define __DISTDB_SERVER_SIDE_H

#include "../include/global_var.h"
#include "../include/distdb.h"

static char _zeropage[4096];


int	g_rpc_socket,g_socket;

LIST_SLOT_DEFINE(nodelist);
LIST_SLOT_DEFINE(node_unconnectedlist);
LIST_SLOT_DEFINE(node_connectedlist);

int				  groupcount;
struct _groupmap *groupmap;

void*			  zeropage = (void*)_zeropage;

struct db_ops db;
