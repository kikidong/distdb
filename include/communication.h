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
	uint32_t					pad:8;
	uint32_t					execflag;
	char						sql_command[0];
};


#pragma pack(pop)




#endif /* COMMUNICATION_H_ */
