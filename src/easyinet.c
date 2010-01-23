/*
 * easyinet.c - make inet more easier
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
 * @defgroup HELPER 功能
 * @author	 microcai.
 * @{
 * @name easyinet.c
 * @{
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/distdb.h"
#include "../include/db_def.h"

/**
 * @brief 转化为 ip 地址,如果有的话，端口
 * @param[in] servername
 *
 * resoveserver 将一个 服务器名称转化为 ip 地址,名称可以是点分十进制的ip地址，可以是 netbios 地址
 * 可以是 DNS 地址。当然，NULL 字符串返回本机地址
 */
in_addr_t resoveserver(const char * servername,int * port)
{
	in_addr_t ip;
	struct hostent  host;
	struct hostent * phost;
	int h_erron;
	char buff[1024];

	if(servername)
	{
		//首先就是要解析出端口地址
		sscanf("*[^ :]%d",servername,port);
		//首先尝试是不是 点分十进制的格式
		ip = inet_addr(servername);
		if (ip == INADDR_NONE)
		{ // 尝试是否是 DNS 地址
			if (!gethostbyname_r(servername, &host,buff,sizeof(buff),&phost,&h_erron))
			{
				memcpy(&ip, phost->h_addr_list[0], sizeof(ip));
			}
			else
				ip = INADDR_NONE;
		}
	}
	else
		ip = LOCALHOST;
	return ip;
}

/**
 * @brief 将字符串表平坦化
 */
struct sql_result_plain_text* convert_strtable2plain( int colum, char * tables[])
{
	struct sql_result_plain_text* ret;
	int							i,offset;
	size_t						maxlen;
	//首先是计算所需要的大小
	size_t	sizes[colum];

	for (i = maxlen = 0; i < colum; i++)
	{
		sizes[i] = strlen(tables[i]);
		maxlen += sizes[i] + 1; // 包含结尾的 NULL 呵呵
	}

	ret = malloc(maxlen + size_sql_result_plain_text);
	ret->size = maxlen + size_sql_result_plain_text - sizeof(list_slot);
	ret->resultlist.next = ret->resultlist.prev = & ret->resultlist;
	ret->colums = colum;
	for (i = 0, offset = sizeof(struct text_slot) * colum + sizeof(int); i < colum; ++i)
	{
		ret->strings[i].length = sizes[i];
		ret->strings[i].offset = offset;
		strcpy(ret->plaindata + offset, tables[i]);
		offset += sizes[i] + 1;
	}
	return ret;
}
