#ifndef __PACKET__H
#define __PACKET__H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

__BEGIN_DECLS

#pragma pack(push,1)

struct rpc_packet_call{
	uint32_t	rpc_call_id; // number of the call
	uint32_t	call_seq; // sequence number of the call
	char data[0];
};

struct rpc_packet_ret{
	uint32_t ret ;// return code
	uint32_t	call_seq; // sequence number of the call
	char data[0]; // return data
};



#define SIZE_RPC_HEADER	8

struct execute_sql_bin{
	uint32_t	length;
	uint32_t	flag;
	char		data[0];
};
#define SIZE_EXECUTE_SQL_BIN 8

struct rpc_sql_result{
	uint8_t	number;
	uint8_t offsets[0];
};

#pragma pack(pop)

#define DISTDB_RPC_EXECUTE_SQL_BIN	1
#define DISTDB_RPC_FREE_RESLUT		2
#define DISTDB_RPC_FETCH_RESULT		3

__END_DECLS
#endif // __PACKET__H

