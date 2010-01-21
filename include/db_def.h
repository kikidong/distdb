/*
 * db_def.h - define db related structure
 *
 * This software is Public Domain
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 */

#ifndef DB_DEF_H_
#define DB_DEF_H_
#include <time.h>
#include <pthread.h>
__BEGIN_DECLS

struct DISTDB_SQL_RESULT{
	time_t	time;	// The time that request the execution
	DEFINE_LIST(resultlist); // The results must be linked together. :)
	struct	the_db_ops * db; // the db opetator
	int		needclose;		 // set 1 if need close the db
	// the severs that receives the same requeset
	int		columns; // columns that may return
	void*	db_private_ptr;
	pthread_mutex_t	lock; // protect this struct
};

__END_DECLS

#endif /* DB_DEF_H_ */
