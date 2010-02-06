/*
 * inifile.c
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
#include <string.h>
#include "inifile.h"

int get_profile_string(FILE *fp, char *AppName,const char const *KeyName, char *KeyValue,size_t KEYVALLEN)
{

	char appname[KEYVALLEN], keyname[KEYVALLEN];
	char buf[KEYVALLEN], *c;
	int found = 1; /* 1 AppName 2 KeyName */

	fseek(fp, 0, SEEK_SET);

	if (appname)
	{
		sprintf(appname, "[%s]", AppName);
		memset(keyname, 0, sizeof(keyname));
		found = 0;
	}

	while (!feof(fp) && fgets(buf, KEYVALLEN, fp) != NULL)
	{
		if (found == 0)
		{
			if (buf[0] != '[')
			{
				continue;
			}
			else if (strncmp(buf, appname, strlen(appname)) == 0)
			{
				found = 1;
				continue;
			}
		}
		else if (found == 1)
		{
			if (buf[0] == '#')
			{
				continue;
			}
			else if (buf[0] == '[')
			{
				break;
			}
			else
			{
				if ((c = (char*) strchr(buf, '=')) == NULL)
					continue;
				memset(keyname, 0, sizeof(keyname));
				sscanf(buf, "%[^= ]", keyname);
				if (strcmp(keyname, KeyName) == 0)
				{
					sscanf(++c, "%*[ \t]%[^\n]",KeyValue);
					found = 2;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	if (found == 2)
		return (0);
	else
		return (-1);
}




