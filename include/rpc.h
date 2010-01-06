#ifndef __PACKET__H
#define __PACKET__H

#ifndef __BEGIN_DECLS
#include <features.h>
#endif
__BEGIN_DECLS


struct rpc_packet_call{
	int	rpc_call_id; // number of the call
	int	call_seq; // sequence number of the call
	char data[0];
};

struct rpc_packet_ret{
	int ret ;// return code
	int	call_seq; // sequence number of the call
	char data[0]; // return data
};

__END_DECLS
#endif // __PACKET__H

