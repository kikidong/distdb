/*
 * communication.h - defines for communication between distdbs
 *
 * COPYRIGHT NOTES SEE COPYING
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_
#include <features.h>
#include "list.h"
#include "db_def.h"
#include "nodes.h"

#pragma pack(push,8)


struct db_exchange_header{
	union{
	struct DISTDB_SQL_RESULT * restptr;
	char					restptr_pad[8];
	};
	uint32_t					length:24;
	uint32_t					type:8; // @see db_exchange_type
	uint32_t					execflag;
	union
	{
		char sql_command[0];
		char data[0];
	};
};

#define db_exchange_header_size 20

#pragma pack(pop)

enum db_exchange_type
{
	db_exchange_type_exec_sql = 0,
	db_exchange_type_return_result = 1,
	db_exchange_type_end_result =2 ,  // use this to declare that there is no more result



};


#endif /* COMMUNICATION_H_ */
