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

extern int get_profile_string(FILE *fp, char *AppName,const char const *KeyName, char *KeyValue,size_t KEYVALLEN);

#endif /* INIFILE_H_ */
