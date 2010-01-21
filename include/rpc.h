#ifndef __PACKET__H
#define __PACKET__H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "list.h"

__BEGIN_DECLS

#pragma pack(push,1)

struct rpc_packet_call{
	uint32_t	rpc_call_id; // number of the call
	uint32_t	call_seq; // sequence number of the call
	uint32_t	len;	  // add for TCP connection, decide the length
	char data[0];
};

struct rpc_packet_ret{
	uint32_t ret ;// return code
	uint32_t	call_seq; // sequence number of the call
	uint32_t	len;	  // add for TCP connection, decide the length
	char data[0]; // return data
};

#define RPC_PACKET_HEADER_SIZE 12


/**
 * 用这个结构来管理rpc客户端，追踪其内存使用。进行内存使用上限控制
 * 追踪其SQL查询结果，在连接断开没有主动释放的情况下释放掉其所占用的内存
 */
struct rpc_clients
{
	//链表，用来管理所有的rpc 客户连接
	DEFINE_LIST(clients);
	//链表，管理这个客户的所有 sql 查询结果，退出时释放
	list_slot	sql_results;
	//客户端 套接字地址
	struct sockaddr_in	addr;
};

struct execute_sql_bin{
	uint32_t	length;
	uint32_t	flag;
	char		data[0];
};
#define SIZE_EXECUTE_SQL_BIN 8


struct db_sql_result{
	uint8_t	number;
	uint8_t offsets[0];
};

#define rpc_sql_result db_sql_result

#pragma pack(pop)

#define DISTDB_RPC_EXECUTE_SQL_BIN	1
#define DISTDB_RPC_FREE_RESLUT		2
#define DISTDB_RPC_FETCH_RESULT		3

void * rpc_loop_thread(void*p);

__END_DECLS
#endif // __PACKET__H

