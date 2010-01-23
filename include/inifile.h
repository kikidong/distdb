/*
 * inifile.h
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

#ifndef INIFILE_H_
#define INIFILE_H_

#include <stdio.h>


#ifndef HAVE_GETLINE

static inline size_t getline (char ** __lineptr,size_t * __n,FILE * __stream)
{
	int malloc_called = 1024;
	if(!*__lineptr)
		malloc(malloc_called++);
	if(fgets(*__lineptr,1024,__stream))
		return (*__n = strlen(*__lineptr));
	else if(malloc_called>1024)
		free(*__lineptr);
	return * __n = 0;
}

#endif

extern int get_profile_string(FILE *fp, char *AppName,const char const *KeyName, char *KeyValue,size_t KEYVALLEN);

#endif /* INIFILE_H_ */
