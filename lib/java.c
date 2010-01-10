/*
 * java.c
 *
 *  Created on: 2010-1-10
 *      Author: cai
 */

#include "bindings/java/distdb.h"

JNIEXPORT jdouble JNICALL Java_distdb_testd(JNIEnv * jnie, jclass jc, jdouble i)
{
	return 5*5;
}
