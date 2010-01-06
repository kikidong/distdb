/*
 * version.c
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

#include "vcconfig.h"


#define __DISTDB_SERVER_SIDE_H

#include "../include/distdb.h"

const char* distdb_version()
{
	return PACKAGE_VERSION;
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2,1),&wsadata);

	}else if (fdwReason == DLL_PROCESS_DETACH)
	{
		WSACleanup();
	}
}
#endif
