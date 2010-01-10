/*
 * db_def.h - define db related structure
 *
 * Copyright (C) 2009-2010 Kingstone, Ltd.
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
 * If you have any question with law suite, please contract 黄小克 , the owner of
 * this company.
 */

#ifndef DB_DEF_H_
#define DB_DEF_H_
#include <time.h>

__BEGIN_DECLS

struct DISTDB_SQL_RESULT{
	time_t	time;	// The time that request the execution
	DEFINE_LIST(resultlist); // The results must be linked together. :)
	struct	the_db_ops * db; // the db opetator
	void*	db_private_ptr;
	// the severs that receives the same requeset
	int		columns; // columns that may return
};

__END_DECLS

#endif /* DB_DEF_H_ */