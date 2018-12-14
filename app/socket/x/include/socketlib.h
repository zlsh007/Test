/*************************************************************************
	> File Name: netmsglib.h
	> Author: suhua
	> Mail:
	> Created Time: 2014年12月06日 星期六 09时47分11秒
 ************************************************************************/
#ifndef NETMSG_LIB__H
#define NETMSG_LIB__H
#include <sys/socket.h>
#include <arpa/inet.h>

#include "list.h"
//error number
#define ESKTMLC                         1
#define ESKTATTRREGCHECHNAME            2
#define ESKTATTRREGSOCKET               3
#define ESKTATTRREGBIND                 4
#define ESKTATTRREGLISTEN               5
#define ESKTNODEREGCHECKNAME            6
#define ESKTNODEREGSOCKET               7
#define ESKTNODEREGCONNECT              8
#define ESKTACCEPTSEARCHATTR  9
#define ESKTACCEPTALLOCNODE   10
#define ESKTACCEPTACCEPT      11
#define ESKTACCEPTCHECKNODENAME  12
#define ESKTSENDSEARCHNODE    13
#define ESKTRECVSEARCHNODE   14

#define DEFAULT_REUSE  1
#define DEFAULT_RCVBUFSIZE 0x10000
#define DEFAULT_RCVLOWAT  0x1
#define DEFAULT_RTOUTSEC  0x10
#define DEFAULT_RTOUTUSEC  0x0

#define DEFAULT_SNDBUFSIZE 0x10000
#define DEFAULT_SNDLOWAT  0x1
#define DEFAULT_STOUTSEC  0x10
#define DEFAULT_STOUTUSEC  0x0

#define SOCKIN_CLEN                     10
#define DEFAULT_LISTENMAX               10
#define SKT_SERVER_PORT                 3260
#define STORAGE_SERVER_PORT             3262
#define STORAGE_SERVER_IP	        "10.10.118.116"
#define SKT_ATTR                        "SKTATTRSERVER"
#define SKT_NODE_NAME                   "NODE"
#define FIFO_SS_NAME                    "FIFO_SS"
#define FIFO_SR_NAME                    "FIFO_SR"
#define SKT_CLIENT_MAX                  1
#define SKT_NAME_LEN                    7
#define FIFO_NAME_LEN                   10

struct pthread_skt {
        int thread_num[SKT_CLIENT_MAX];
        pthread_attr_t pattr;
        pthread_t stid;
        pthread_t ctid[SKT_CLIENT_MAX];
};

struct skt_list_head{
	char *name;
	struct list_head hnode;
	pthread_mutex_t nmutex;
	struct list_head hattr;
	pthread_mutex_t amutex;
};

struct sockopt{
	int reuse;
	int rcvbufsize;
	int rcvlowat;
	struct timeval rtout;
	int sndbufsize;
	int sndlowat;
	struct timeval stout;
};

struct skt_attr{                        //descript socket server
	char *name;                     //attr name
	int damain;                     //af_inet
	int type;
	int protocol;
	int listenmax;
	char ip[16];
	int port;
	struct list_head attr_list;     // attr list
	struct list_head cnode_list;    //cli node list
	struct sockopt sopt;	
	
	pthread_mutexattr_t *mutex_attr;
	pthread_mutex_t mutex;          //lock attr
	
	int fd;                         //socket fd
	struct sockaddr_in saddr;       //server listen addr
	socklen_t saddrlen;
};

struct skt_node{
	char *name;
	int damain;                     //AF_INET
	int type;                       //SOCK_STREAM OR SOCK DGRAM
	int protocol;
	char ip[16];
	int port;
	void *parent;                   //point to server
	int fd;                         //socket fd
	int flags;
	struct sockaddr_in saddr;       //dest addr
	socklen_t saddrlen;             //dest addr leng
	
	struct list_head list;
	struct list_head plist;         //list head start of attr,client list
	struct sockopt sopt;	
	pthread_mutexattr_t *mutex_attr;
	pthread_mutex_t mutex;          //lock node
};

int skt_free_attr(void *attrp);
int skt_free_node(void *nodp);
void print_attr(void);
void print_node(void);
char *get_node_ip(const char *name);
int skt_sys_init(const char *sys_name);
void skt_sys_free(const char *sysname);
void *skt_alloc_attr(const int type, const int port);
struct skt_node *skt_alloc_node(const int type,
                const char *sip, const int port);
int skt_attr_register(const void *attr, const char *name);
int skt_node_register(void *node, char *name);
int skt_accept(char *attrname, char *nodename);
int skt_send(void *name, void *sbuf, size_t slen);
int skt_recv(char *name, void *rbuf, size_t rlen);
void *skt_shutdown(char *name, int how);
void *skt_close_attr(char *name);
void *skt_close_node(char *name);
void skt_close_free_node(char *name);

#endif
