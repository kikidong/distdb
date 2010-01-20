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
	union
	{
		struct
		{
			uint32_t execflag;
			char sql_command[0];
		}exec_sql;
		struct {
			int pad;
		}ack_sql;
	};
};

#define db_exchange_header_size 16

#pragma pack(pop)

enum db_exchange_type
{
	db_exchange_type_exec_sql = 1,

};


#endif /* COMMUNICATION_H_ */
