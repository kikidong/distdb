/**
 * @defgroup RPC 函数调用.
 * @author	 microcai.
 * @{
 * @name distdb.h
 * @{
 */

/* distdb.h  - RPC 调用定义
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



#ifndef __DISTDB_H_
#define __DISTDB_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HAVE_GETLINE

static inline size_t getline (char ** __lineptr,size_t * __n,FILE * __stream)
{
	int malloc_called = 1024;
	if(!*__lineptr)
		malloc(malloc_called++);
	if(fgets(*__lineptr,1024,__stream))
		return (*__n = strlen(*__lineptr));
	else if(malloc_called>1024)
		free(*__lineptr);
	return * __n = 0;
}

#endif

#ifdef __DISTDB_SERVER_SIDE_H

#ifndef MIN
#define MIN(x,y)	( (x) < (y) ? (x):(y) )
#endif
#define ZEROWITHSIZE(structure)	memset(&structure,0,sizeof(structure));

#endif

/**
 * @brief 用4个int参数构造出ip地址类型，而不用管CPU大端小端
 */
static int inline MAKEINET(s1,s2,s3,s4)
{
	union {
		unsigned char saddr[4];
		unsigned int  addr;
	}ip;

	ip.saddr[0] = s1 ;
	ip.saddr[1] = s2 ;
	ip.saddr[2] = s3 ;
	ip.saddr[3] = s4 ;

	return ip.addr;
}


/**
 * @brief 本地回环地址 127.0.0.1
 *
 * 使用  MAKEINET(127,0,0,1) 构建本机地址
 */
#define LOCALHOST	MAKEINET(127,0,0,1)

__BEGIN_DECLS


struct DISTDB_SQL_RESULT;

/**
 * @brief 用来标识一个SQL查询操作结果
 *
 * 由 distdb_rpc_execute_sql_str 和 distdb_rpc_execute_sql_bin 返回，并由
 * distdb_rpc_free_result 进行释放。内部结构在下一个版本中不保证是否会相同，所以
 * 不要尝试解析其内部意义
 */
typedef struct DISTDB_SQL_RESULT DISTDB_SQL_RESULT;


/**
 * @brief 返回distdb库的版本号
 * @return 描述版本号的字符串。
 *
 * 注意，不可以释放返回的指针，因为内部实现可能是这样的
 * @code
 * const char* distdb_version(){
 * 	return "1.0.0";
 * }
 * @endcode
 */
const char* distdb_version();

/**
 * @brief 连接到指定的RPC服务器
 * @param[in] server 服务器的字符串
 * @retval 0 成功连接到远程服务器
 * @retval 1 无法连接，要查询具体错误，请调用 distdb_lasterror()
 *
 * 如果 server 指针为　NULL 表示连接本地服务。server 字符串可以使用
 * 	SERVERNAME:[PORT] 的形式指定端口。SERVERNAME 可以是　IP 地址，也可以是计算机名。
 * @see distdb_lasterror
 */
int distdb_rpc_connectto(const char * server);

/**
 * @brief 断开RPC连接
 * @return 无
 *
 * 断开与 RPC 服务器的连接，一般在退出应用程序的时候使用。
 */
void distdb_rpc_disconnect();

/**
 * @brief 进行 distdb_rpc_execute_sql_* 调用时使用的旗标
 */
enum executeflag{


	DISTDB_RPC_EXECSQL_NOLOCAL	= 0x00000001,
	/**< force distdb to send request to other computers*/


	DISTDB_RPC_EXECSQL_NOSERVER = 0x00000002,
	/**< force distdb to lookup locally*/

	DISTDB_RPC_EXECSQL_NORESULT = 0x00000004 ,
	/**<force distdb to discard results*/

	DISTDB_RPC_EXECSQL_ALLOWRECURSIVE = 0x00010000
	/**< 允许在前一个SQL查询句柄还没有释放的情况下再次进行SQL操作*/

	/** @see DISTDB_RPC_EXECSQL_NOLOCAL */
#define DISTDB_RPC_EXECSQL_NOLOCAL DISTDB_RPC_EXECSQL_NOLOCAL
	/**< force distdb to send request to other computers*/

#define DISTDB_RPC_EXECSQL_SERVERONLY DISTDB_RPC_EXECSQL_NOLOCAL
#define DISTDB_RPC_EXECSQL_NOSERVER	DISTDB_RPC_EXECSQL_NOSERVER
#define DISTDB_RPC_EXECSQL_LOCALONLY	DISTDB_RPC_EXECSQL_NOSERVER
#define DISTDB_RPC_EXECSQL_NORESULT DISTDB_RPC_EXECSQL_NORESULT
#define DISTDB_RPC_EXECSQL_ALLOWRECURSIVE DISTDB_RPC_EXECSQL_ALLOWRECURSIVE
};

/**
 * @brief 执行SQL语句
 * @param[out] out 查询结果句柄
 * @param[in] sql 查询语句 \n
 * 				必须是NULL结束的字符串 see distdb_rpc_execute_sql_bin
 * @param[in] executeflag 查询模式，see executeflag
 *
 * 执行SQL语句,SQL语句
 */
int distdb_rpc_execute_sql_str(struct DISTDB_SQL_RESULT ** out,const char * sql, int executeflag);

/**
 * @brief distdb_rpc_execute_sql_bin 执行SQL语句
 * @param[out] out 查询结果句柄
 * @param[in] sql 查询语句 允许包含NULL \n
 * 					@see distdb_rpc_execute_sql_str
 * @param[in] length 长度
 * @param[in] executeflag 查询模式，@see executeflag
 */
int distdb_rpc_execute_sql_bin(struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag);

// One call , one row, more row ? call more
/**
 * @brief 获得一行查询结果
 * @param[in] in 查询结果句柄
 * @param[out] result 放回的结果
 * @retval 0	成功执行。
 * @retval -1	执行失败。
 * @retval 1	没有更多的结果了
 *
 * 对于没一行 SQL 查询结果，都要运行一次 distdb_rpc_fetch_result 。
 *
 * @par 示例:
 *
 * @code
 * DISTDB_SQL_RESULT	*res;
 * char			*table[];
 * if(!distdb_rpc_execute_sql_str(&res,"select * from testdb",0))
 * {
 * 	while(distdb_rpc_fetch_result(res,&table)==0)
 * 	{
 * 	  // DO SOME THIME
 * 	}
 * 	distdb_rpc_free_result(res);
 * }
 * @endcode *
 */
int distdb_rpc_fetch_result(struct DISTDB_SQL_RESULT * in ,char ** result[]);

/**
 * @brief 释放一次查询操作
 * @param[in]	in
 * @retval	0	操作成功完成，所有的服务器端和本地端资源都被成功释放
 * @retval	-1	操作失败。要获得详细信息，请使用 distdb_lasterror()
 *
 * distdb_rpc_execute_sql_bin 返回的查询结果必须被释放，否者将引起服务器端和本地库的内存泄漏
 * 将来的实现将会避免没有释放的句柄耗尽服务器内存。但是，强烈建议你释放掉它.
 */
int distdb_rpc_free_result(struct DISTDB_SQL_RESULT *in);

__END_DECLS

#endif /* __DISTDB_H_ */

/**//** @}*/ //

/**//** @}*/ //
