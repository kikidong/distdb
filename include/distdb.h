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
static int inline MAKEINET(int s1,int s2,int s3,int s4)
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


struct _DISTDB_NODE;

/**
 * @brief 用来标识一个SQL查询操作结果
 *
 * 由 distdb_rpc_execute_sql_str 和 distdb_rpc_execute_sql_bin 返回，并由
 * distdb_rpc_free_result 进行释放。内部结构在下一个版本中不保证是否会相同，所以
 * 不要尝试解析其内部意义
 */
typedef struct DISTDB_SQL_RESULT DISTDB_SQL_RESULT;

/**
 * @brief 用来标识一个distdb节点。
 *
 * 由 distdb_connectto() 创建并返回。由 distdb_disconnect() 释放。不要尝
 * 试解析其内部意义，内部意义将在无通知的情况下任意变动。
 */

typedef struct _DISTDB_NODE *DISTDB_NODE;


/**
 * @brief 转化到 server 模式的时候需要传入的参数。
 *
 * 包含了进行角色转化的所有信息
 */
struct distdb_info{
	const char		servername[32]; /**<可选设，服务器名字，不能包含空格*/
	int				backend;		  /**<数据库后端类型 0 sqlite 1 mysql 2 oracle */
	int 			groupid;		/**<该节点组号*/

	union{
		struct {
			const char	* dbname;	/**<指向数据库文件的文件名*/
		}sqlite3_backend_info;
		struct {
			const char  * dbhost;  /**<mysql数据库名称*/
			const char  * dbuser;  /**<连接到mysql数据库所使用的用户名*/
			const char  * dbpass;  /**<数据库密码*/
		}mysql_backend_info;
		struct {
			const char	* dbhost;  /**<oracle数据库*/
			const char  * dbuser;  /**<oracle数据库用户*/
			const char  * dbpass;  /**<oracle数据库密码*/
		}occi_backend_info;
	}backend_info;
	const char *	node_file; /**<节点数据库文件名。
							 如果提供了的话，distdb 将只接受节点数据库里面列出来的节点*/
};



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
 * @brief 初始化distdb节点
 * @return void
 *
 * 在使用任何 distdb_* (distdb_version除外) 函数前调用他进行显示初始化。
 */
void distdb_initalize();


/**
 * @brief 设置节点角色
 * @param[in] __pdistdb_info 指向 distdb_info 结构的指针
 * @param[in] retain 见详解
 * @return 成功变成 server 角色就成 0，失败成 -1
 *
 * distdb 节点默认角色是 client-only
 *
 * client-only角色是那样的一种角色，它没有本地数据端，只能请求其他节点服务，
 * 也不能接受其他节点的服务请求，只能发出请求，而无法做到进行服务。
 *
 * server 角色是那样的一种角色，它连接到本地数据端，能接受其他服务器的请求
 * 并应答，同时，其自身也能要求其他节点进行服务。
 *
 * 当调用此函数，distdb 将转入server模式。
 *
 * 当调用 distdb_enable_server 的时候，默认会断开所有目前已经连接到的节点。要想连接更多节点，请调用
 * distdb_connect 手工连接。如果 retain 不为 0 ， 则会重新连接  distdb_enable_server 调用时
 * 还在线的节点，使用新身份登录。
 *
 * distdb直接运行 libdistdb.so(Linux平台由脚本 distdbd 执行，Windows平台为 distdb.exe )
 * 的时候，其入口点将调用  distdb_enable_server 进入 server 模式。并间歇搜索新的节点，连接上去
 *
 * distdb 作为 共享库 libdistdb.so (Windows 下是 distdb.dll ， 和 distdb.exe 只是最后连接的时候参数不同 ) 运行的时候，如果调用者没有
 * 调用 distdb_enable_server ， distdb 就进入了 client-only 模式。
 *
 * @see distdb_info
 *
 */
int distdb_enable_server(struct distdb_info *__pdistdb_info,int retain);

/**
 * @brief 连接到指定的节点
 * @param[in] server 服务器的字符串
 *  		如果 server 指针为　NULL 表示连接本地服务。server 字符串可以使用
 *   	SERVERNAME:[PORT] 的形式指定端口。SERVERNAME 可以是　IP 地址，也可以是计算机名。
 * @return DISTDB_NODE *
 *
 * distdb_connect 连接到一个节点，并加入可用节点链表。这些节点将在您进行查询操作是被自动
 * 操作，除非显示指定了操作的节点，否者所有已经连接的节点都会被自动操作。
 *
 */
DISTDB_NODE distdb_connect(const char* server);

/**
 * @brief 断开某个节点的连接
 * @param[in] node 服务器节点描述
 * @return 无
 *
 * 断开与服务节点的连接
 */
void distdb_disconnect(DISTDB_NODE node);

/**
 * @brief 进行 distdb_execute_sql_* 调用时使用的旗标
 */
enum executeflag{


	DISTDB_EXECSQL_NOLOCAL	= 0x00000001,
	/**< force distdb to send request to other computers*/


	DISTDB_EXECSQL_NOSERVER = 0x00000002,
	/**< force distdb to lookup locally*/

	DISTDB_EXECSQL_NORESULT = 0x00000004 ,
	/**<force distdb to discard results*/

	DISTDB_EXECSQL_ALLOWRECURSIVE = 0x00000008,
	/**< 允许在前一个SQL查询句柄还没有释放的情况下再次进行SQL操作*/

	DISTDB_EXECSQL_NOTDIRECTCALL = 0x00010000,
	/**< 内部使用，表示不是用户调用的*/
	DISTDB_EXECSQL_NOBROADCAST = 0x00020000
	/**< 内部使用，表示不用继续发送到其他节点*/

#define DISTDB_EXECSQL_NOLOCAL DISTDB_EXECSQL_NOLOCAL
	/**< force distdb to send request to other computers*/

#define DISTDB_RPC_EXECSQL_SERVERONLY DISTDB_EXECSQL_NOLOCAL
#define DISTDB_RPC_EXECSQL_NOSERVER	DISTDB_EXECSQL_NOSERVER
#define DISTDB_RPC_EXECSQL_LOCALONLY	DISTDB_EXECSQL_NOSERVER
#define DISTDB_RPC_EXECSQL_NORESULT DISTDB_EXECSQL_NORESULT
#define DISTDB_RPC_EXECSQL_ALLOWRECURSIVE DISTDB_EXECSQL_ALLOWRECURSIVE
};

/**
 * @brief 执行SQL语句
 * @param[in] nodes 需要发送请求执行的节点列表
 * 				如果为 NULL 就是所有已经连接的节点
 * 				节点列表最后一项用 NULL 结尾
 *
 * @param[out] out 查询结果句柄
 * @param[in] sql 查询语句 \n
 * 				必须是NULL结束的字符串 see distdb_rpc_execute_sql_bin
 * @param[in] executeflag 查询模式，see executeflag
 *
 * 执行SQL语句,SQL语句
 */
int distdb_execute_sql_str(DISTDB_NODE * nodes,struct DISTDB_SQL_RESULT ** out,const char * sql, int executeflag);

/**
 * @brief distdb_rpc_execute_sql_bin 执行SQL语句
 * @param[in] nodes 需要发送请求执行的节点列表
 * 				如果为 NULL 就是所有已经连接的节点
 * 				节点列表最后一项用 NULL 结尾
 * @param[out] out 查询结果句柄
 * @param[in] sql 查询语句 允许包含NULL \n
 * 					@see distdb_rpc_execute_sql_str
 * @param[in] length 长度
 * @param[in] executeflag 查询模式，@see executeflag
 */
int distdb_execute_sql_bin(DISTDB_NODE * nodes,struct DISTDB_SQL_RESULT ** out,const char *sql,size_t length,int executeflag);

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
