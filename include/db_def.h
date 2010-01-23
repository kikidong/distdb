/*
 * db_def.h - define db related structure
 *
 * Copyright (C) 2009-2010 Kingstone, Ltd.
 *
 * Written by microcai in 2009-2010
 *
 * This software is licensed under the Kingstone mid-ware License.
 *
 * For more information see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 *
 * If you have any question with law suite, please contract 黄小克 , the owner of
 * this company.
 */

#ifndef DB_DEF_H_
#define DB_DEF_H_
#include <time.h>
#include <pthread.h>
#include "list.h"

__BEGIN_DECLS

struct text_slot{
	uint32_t	offset;
	uint32_t	length;
};

#pragma pack(push,1)

struct sql_result_plain_text{ // record for incomming result by other nodes
	DEFINE_LIST(resultlist);
	size_t		size;
	union
	{
		struct
		{
			int colums;
			struct text_slot strings[0]; // == colums offset count from plaindata
		};
		char plaindata[0];
	};
	// and then is the data.
};
#pragma pack(pop)

#define size_sql_result_plain_text ( sizeof(size_t) + sizeof(struct list_node) + sizeof(int) )
struct sql_result_plain_text* convert_strtable2plain( int colum, char * tables[]);

struct DISTDB_SQL_RESULT{
	time_t	time;	// The time that request the execution
	DEFINE_LIST(resultlist); // The results must be linked together. :)
	struct nodes	* client ; // The client that request the result
	int		needclose;		 // set 1 if need close the db, set -1 if have closed local db
	int		ref;
	// the severs that receives the same requeset
	int		columns; // columns that may return
	void*	db_private_ptr;
	pthread_mutex_t	lock; // protect this struct
	pthread_cond_t	waitcond;
	list_slot sql_result;
	struct sql_result_plain_text * last; // 记录上次使用的。记得释放
	struct DISTDB_SQL_RESULT * old_res; // old result, NON-NULL means we should not save the incoming result, but re-send it
};

static inline struct DISTDB_SQL_RESULT * DISTDB_SQL_RESULT_NEW()
{
	struct DISTDB_SQL_RESULT * ret ;
	ret = (typeof(ret))malloc(sizeof(struct DISTDB_SQL_RESULT));
	memset(ret,0,sizeof(struct DISTDB_SQL_RESULT));
	ret->sql_result.head = ret->sql_result.tail = (struct list_node*) & ret->sql_result ;
	pthread_mutex_init(&ret->lock,0);
	pthread_cond_init(&ret->waitcond,0);
}

__END_DECLS

#endif /* DB_DEF_H_ */
