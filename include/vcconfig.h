/*
 * vcconfig.h - config file used on vc(WIN32) platform
 *
 *  Created on: 2009-12-31
 *      Author: cai
 */

#ifndef VCCONFIG_H_
#define VCCONFIG_H_

#ifdef _WIN32
#include <windows.h>
#endif

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define getpagesize()	(4096)

#endif /* VCCONFIG_H_ */
