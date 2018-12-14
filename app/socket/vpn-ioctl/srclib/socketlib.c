/*************************************************************************
	> File Name: socketlib.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Mon 23 Dec 2013 03:42:00 PM CST
 ************************************************************************/
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
 #include <arpa/inet.h>

#include "apue.h"
#include "list.h"
#include "hear.h"
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

#define SOCKIN_CLEN 10
#define DEFAULT_LISTENMAX  10
#define SKT_SERVER_PORT         3260

struct skt_list_head{
	char *name;
	struct list_head hnode;//node head
	pthread_mutex_t nmutex;
	struct list_head hattr;// attr head
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

struct skt_attr{ //descript socket server
	char *name;//attr name
	int damain;//af_inet
	int type;//
	int protocol;//0
	int listenmax;
	char ip[16];
	int port;
	struct list_head attr_list;// attr list
	struct list_head cnode_list;//cli node list
	struct sockopt sopt;	
	
	pthread_mutexattr_t *mutex_attr;
	pthread_mutex_t mutex;//lock attr
	
	int fd;//socket fd
	struct sockaddr_in saddr;//server listen addr
	socklen_t  saddrlen;
};

struct skt_node{
	char *name;
	int damain;//AF_INET
	int type;//SOCK_STREAM OR SOCK DGRAM
	int protocol;//0
	char ip[16];
	int port;
	void *parent;//point to server
	int fd;//socket fd
	int flags;
	struct sockaddr_in saddr;//dest addr
	socklen_t  saddrlen;//dest addr leng
	
	struct list_head list;
	struct list_head plist;//list head start of attr,client list
	struct sockopt sopt;	
	pthread_mutexattr_t *mutex_attr;
	pthread_mutex_t mutex;//lock node
};


static struct skt_list_head skt_head={NULL};

int skt_free_attr(void *attrp);
int skt_free_node(void *nodp);

static inline void *list_for_search_attr(const char *name)
{
	struct skt_attr  *cp = NULL;
	struct list_head *pos, *n;
	pthread_mutex_lock(&skt_head.amutex);
	list_for_each_safe(pos, n, &skt_head.hattr) {
		cp = list_entry(pos, struct skt_attr, attr_list);	
		if (!strcmp(cp->name, name)) {
			pthread_mutex_unlock(&skt_head.amutex);
			HEAR("return cp\n");
			return cp;
		}
	}	
	pthread_mutex_unlock(&skt_head.amutex);
	HEAR("return null\n");
	return NULL;	
}

static inline void *list_for_search_node(const char *name)
{
	struct skt_node  *np = NULL;
	struct list_head *pos, *n;
	pthread_mutex_lock(&skt_head.nmutex);
	list_for_each_safe(pos, n, &skt_head.hnode) {
		np = list_entry(pos, struct skt_node, list);	
		if (!strcmp(np->name, name)) {
			pthread_mutex_unlock(&skt_head.nmutex);
			return np;
		}
	}	
	pthread_mutex_unlock(&skt_head.nmutex);
	return NULL;	
}

static char *check_node_name(const char *name)
{
	char *p = NULL;
	int len = strlen(name);
	p = list_for_search_node(name);
	if (p) return NULL;
	p = malloc(len+4);
	if (!p) {
		printf("MALLOC NODE name");
		return NULL;
	}
	memset(p, 0, len+4);
	memcpy(p, name, len+1);
	return p;
}

static char *check_attr_name(const char *name)
{
	char *p = NULL;
	int len = strlen(name);
	p = list_for_search_attr(name);
	if (p) return NULL;
	HEAR("file the name\n");
	p = malloc(len+4);
	if (!p) {
		printf("MALLOC ATTR name");
		return NULL;
	}
	memset(p, 0, len+4);
	memcpy(p, name, len+1);
	//strcpy(p, name);
	return p;
}

char *get_node_ip(const char *name)
{
	struct skt_node *nodp = NULL;

	nodp = (struct skt_node *)list_for_search_node(name);
	if (!nodp) {
		return NULL;
	}
	return nodp->ip;	
}
#if 0
//typedef void (*sighandler_t)(int);
//sighandler_t signal(int signum, sighandler_t handler);
void sigpig_skt_fun(int sig)
{
	printf("sig:%d, pthreadid:%d\n", sig, (int)pthread_self());
	send_to_list(datap->slistp, rcvbuf, ret);
	return ;
}
#endif

int skt_sys_init(const char *sys_name)
{
	struct skt_list_head *sktp = &skt_head;
	if (skt_head.name) return 0;
	int len = strlen(sys_name)+1;
	sktp->name = malloc(len);
	if (!sktp->name) {
		printf("MALLOC SKT NAME");
		return -ESKTMLC;
	}
	memset(sktp->name, 0, len);
	strcpy(sktp->name, sys_name);
	
	sktp->hnode.next = &sktp->hnode;
	sktp->hnode.prev = &sktp->hnode;
	pthread_mutex_init(&sktp->nmutex, NULL);
	
	sktp->hattr.next = &sktp->hattr;
	sktp->hattr.prev = &sktp->hattr;
	pthread_mutex_init(&sktp->amutex, NULL);
//	signal(SIGPIPE, &sigpig_skt_fun);

	return 0;
}

void skt_sys_free(const char *sysname)
{
	struct skt_node *nodp = NULL;
	struct skt_attr *attrp =NULL;
	struct list_head *pos, *n;
	if(strcmp(skt_head.name, sysname)) {
		return ;
	}
	pthread_mutex_lock(&skt_head.amutex);
	pthread_mutex_lock(&skt_head.nmutex);
	free(skt_head.name);
	skt_head.name = NULL;
	
	list_for_each_safe(pos, n, &skt_head.hattr) {
		attrp = list_entry(pos, struct skt_attr, attr_list);	
		list_del_init(&attrp->attr_list);
		skt_free_attr(attrp);
	}
	list_for_each_safe(pos, n, &skt_head.hnode) {
		nodp = list_entry(pos, struct skt_node, list);	
		list_del_init(&nodp->list);
		skt_free_node(nodp);
	}
	pthread_mutex_unlock(&skt_head.nmutex);
	pthread_mutex_unlock(&skt_head.amutex);
	pthread_mutex_destroy(&skt_head.nmutex);	
	pthread_mutex_destroy(&skt_head.amutex);	
	return ;
}
void *skt_alloc_attr(const int type, const int port)
{
	struct skt_attr *attrp = NULL;
	struct sockopt *s = NULL;
	attrp = malloc(sizeof(struct skt_attr));
	if (!attrp) {
		printf("MALLOC ATTR ERROR\n");
		return NULL;
	}
	memset(attrp, 0, sizeof(struct skt_attr));
	attrp->name = NULL;
	attrp->damain = AF_INET;
	attrp->type = type;
	attrp->protocol = 0;
	
	attrp->listenmax = DEFAULT_LISTENMAX;
	memset(attrp->ip, 0, 16);
//	strcpy(attrp->ip, ip);
	attrp->port = port;
	attrp->fd = -1;//socket fd
	attrp->cnode_list.next = &attrp->cnode_list;
	attrp->cnode_list.prev = &attrp->cnode_list;
	attrp->attr_list.next = &attrp->attr_list;
	attrp->attr_list.prev = &attrp->attr_list;
	
	attrp->mutex_attr = NULL;
	pthread_mutex_init(&attrp->mutex, NULL);
	memset(&attrp->saddr, 0, sizeof(struct sockaddr_in));
	attrp->saddr.sin_family = attrp->damain;
	attrp->saddr.sin_addr.s_addr = INADDR_ANY;
	attrp->saddr.sin_port = htons(attrp->port);
	attrp->saddrlen = sizeof(struct sockaddr_in);
	s = &attrp->sopt;	
	s->reuse = 1;
	s->rcvbufsize = DEFAULT_RCVBUFSIZE;
	s->rcvlowat = DEFAULT_RCVLOWAT;
	s->rtout.tv_sec = DEFAULT_RTOUTSEC;
	s->rtout.tv_usec =DEFAULT_RTOUTUSEC;
	s->sndbufsize = DEFAULT_SNDBUFSIZE;
	s->sndlowat = DEFAULT_SNDLOWAT;
	s->stout.tv_sec = DEFAULT_STOUTSEC;
	s->stout.tv_usec =DEFAULT_STOUTUSEC;
	return attrp;
}
//attr == node
void *skt_alloc_node(const int type, const char *sip, const int port)
{
	struct skt_node *nodp = NULL;
	unsigned int saddr;
	struct sockopt *s = NULL;
//	printf("sip:%s, port:%d\n", sip, port);
	nodp = malloc(sizeof(struct skt_node));
	if (!nodp) {
		printf("MALLOC NODE ERROR\n");
		return NULL;
	}
	memset(nodp, 0, sizeof(struct skt_node));
	nodp->name = NULL;
	nodp->damain = AF_INET;
	nodp->type = type;
	nodp->protocol = 0;
	
	memset(nodp->ip, 0, 16);
	strcpy(nodp->ip, sip);
	nodp->port=port;
	nodp->parent = NULL;
	nodp->fd=-1;//socket fd
	
	nodp->list.next = &nodp->list;
	nodp->list.prev = &nodp->list;
	
	nodp->plist.next = &nodp->plist;
	nodp->plist.prev = &nodp->plist;
	
	nodp->mutex_attr = NULL;
	pthread_mutex_init(&nodp->mutex, nodp->mutex_attr);

	inet_pton(AF_INET, sip, &saddr);
	nodp->saddrlen =  sizeof(struct sockaddr_in);
	memset(&nodp->saddr, 0, sizeof(struct sockaddr_in));
	nodp->saddr.sin_family = nodp->damain;
	nodp->saddr.sin_addr.s_addr = saddr;
	nodp->saddr.sin_port = htons(port);
	s =&nodp->sopt;
	s->reuse = 1;
	s->rcvbufsize = DEFAULT_RCVBUFSIZE;
	s->rcvlowat = DEFAULT_RCVLOWAT;
	s->rtout.tv_sec = DEFAULT_RTOUTSEC;
	s->rtout.tv_usec =DEFAULT_RTOUTUSEC;
	s->sndbufsize = DEFAULT_SNDBUFSIZE;
	s->sndlowat = DEFAULT_SNDLOWAT;
	s->stout.tv_sec = DEFAULT_STOUTSEC;
	s->stout.tv_usec =DEFAULT_STOUTUSEC;

	return nodp;
}


int skt_free_attr(void *attrp)
{
	struct skt_attr *p = attrp;

	if (p) {
		if (p->name) ;
			free(p->name);
		p->name=NULL;
		free(p);
		p = NULL;
		attrp = NULL;
	}
	return 0;
}

int skt_free_node(void *nodp)
{
	struct skt_node *p = nodp;
	if (p) {
		if (p->name) {
			free(p->name);
		}
		p->name=NULL;
		free(p);
	}
	return 0;
}

#if 0
int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
	int nsec;
	for (nsec = 1; nsec <= MAXSLEEP;nsec <<= 1) {
		if (0 == connect(sockfd, addr, alen)) {
			return 0;
		}
		if (nsec <= MAXSLEEP/2) 
			sleep(nsec);
	}
	return -1;
}
#endif
int skt_attr_regester(const void *attr, const char *name)
{
	struct skt_attr *attrp = (struct skt_attr *)attr;
	struct sockopt *s = &attrp->sopt;
	
	//pthread_mutex_lock(&skt_head.amutex);
	attrp->name = check_attr_name(name);
	if (!attrp->name) {
		printf("MALLOC ATTR NAME ERROR\n");
	//	pthread_mutex_unlock(&skt_head.amutex);
		return -ESKTATTRREGCHECHNAME;
	}
	if ((attrp->fd = socket(attrp->damain, attrp->type, attrp->protocol)) < 0) {
		printf("CREAT SOCKET ERROR\n");
		if (attrp->name) {
			free(attrp->name);
		}
		attrp->name = NULL;
	//	pthread_mutex_unlock(&skt_head.amutex);
		return -ESKTATTRREGSOCKET;
	}
	//set reuse
	setsockopt(attrp->fd, SOL_SOCKET, SO_REUSEADDR, &s->reuse, sizeof(int));
	setsockopt(attrp->fd, SOL_SOCKET, SO_RCVBUF, &s->rcvbufsize, sizeof(int));
	setsockopt(attrp->fd, SOL_SOCKET, SO_RCVLOWAT, &s->rcvlowat, sizeof(int));
//	setsockopt(attrp->fd, SOL_SOCKET, SO_RCVTIMEO, &s->rtout, sizeof(struct timeval));
	
	setsockopt(attrp->fd, SOL_SOCKET, SO_SNDBUF, &s->sndbufsize, sizeof(int));
	setsockopt(attrp->fd, SOL_SOCKET, SO_SNDLOWAT, &s->sndlowat, sizeof(int));
//	setsockopt(attrp->fd, SOL_SOCKET, SO_SNDTIMEO, &s->stout, sizeof(struct timeval));
	
	if (bind(attrp->fd, (struct sockaddr *)&attrp->saddr, attrp->saddrlen) < 0) {
		printf("BIND");
		if (attrp->name) {
			free(attrp->name);
		}
		attrp->name = NULL;
		if (-1 != attrp->fd) close(attrp->fd);
		attrp->fd = -1;
//		pthread_mutex_unlock(&skt_head.amutex);
		return -ESKTATTRREGBIND;
	}
	if (attrp->type == SOCK_STREAM || attrp->type == SOCK_SEQPACKET) {
		if (listen(attrp->fd, attrp->listenmax) < 0) {
			printf("LISTEN");
			if (attrp->name) {
				free(attrp->name);
			}
			attrp->name = NULL;
			close(attrp->fd);
			attrp->fd = -1;
//			pthread_mutex_unlock(&skt_head.amutex);
			return -ESKTATTRREGLISTEN;
		}
	}
	pthread_mutex_lock(&skt_head.amutex);
	list_add_tail(&attrp->attr_list, &skt_head.hattr);
	pthread_mutex_unlock(&skt_head.amutex);
	
	return 0;
}

int skt_node_regester(void *node, char *name)
{
	struct skt_node *nodp = (struct skt_node *)node;
	struct sockaddr_in addr;//dest addr
	socklen_t  addrlen;//dest addr leng
	int ret;
	if (nodp->name) {
		free(nodp->name);
	}
	nodp->name = check_node_name(name);
	if (!nodp->name) {
		printf("MALLOC NODE NAME");
		return -ESKTNODEREGCHECKNAME;
	}
	if (nodp->fd>=0) {
		close(nodp->fd);
		nodp->fd = -1;
	}
	if ((nodp->fd = socket(nodp->damain, nodp->type, nodp->protocol)) < 0) {
		printf("CREATE SOCKET");
		if (nodp->name) {
			free(nodp->name);
		}
		nodp->name = NULL;
		return -ESKTNODEREGSOCKET;
	}
	addrlen = sizeof(struct sockaddr_in);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	addr.sin_family = nodp->saddr.sin_family;
	addr.sin_addr.s_addr = nodp->saddr.sin_addr.s_addr;
	addr.sin_port = nodp->saddr.sin_port;
	
	if (nodp->type == SOCK_STREAM || nodp->type == SOCK_SEQPACKET) {
		if ((ret = connect(nodp->fd, (struct sockaddr *)&addr, addrlen)) < 0) {
			printf("CONNECT ERROR\n");
			if (nodp->name) {
				free(nodp->name);
				nodp->name = NULL;
			}
			close(nodp->fd);
			nodp->fd = -1;
			return -ESKTNODEREGCONNECT;	
		} else {
		}
	}

	pthread_mutex_lock(&skt_head.nmutex);
	list_add_tail(&nodp->list, &skt_head.hnode);
	pthread_mutex_unlock(&skt_head.nmutex);
	return 0;
}


int skt_accept(char *attrname, char *nodename)
{
	struct skt_node *nodp = NULL;
	struct skt_attr *attrp = NULL;
	
	attrp = list_for_search_attr(attrname);
	if (!attrp) {
		printf("list for search attr ERROR sh\n");
		return -ESKTACCEPTSEARCHATTR;
	}
	nodp = skt_alloc_node(attrp->type, attrp->ip, attrp->port);
	if (!nodp) {
		return -ESKTACCEPTALLOCNODE;
	}
	nodp->fd = accept(attrp->fd, (struct sockaddr *)&nodp->saddr, &nodp->saddrlen);
	if (nodp->fd < 0) {
		printf("ACCEPT ERROR\n");
		if (nodp) {
			free(nodp);
		}
		nodp=NULL;
		return -ESKTACCEPTACCEPT;
	}
	nodp->name =check_node_name(nodename);
	if (!nodp->name) {
		printf("check node name error!");
		close(nodp->fd);
		nodp->fd = -1;
		free(nodp);
		nodp = NULL;
		return -ESKTACCEPTCHECKNODENAME;
	}
	nodp->parent = (void*)attrp;
	memset(nodp->ip, 0, 16);
	inet_ntop(AF_INET, &nodp->saddr.sin_addr.s_addr, nodp->ip, INET_ADDRSTRLEN);
	nodp->saddrlen =  sizeof(struct sockaddr_in);
	nodp->damain = nodp->saddr.sin_family = nodp->damain;
	nodp->port = attrp->port;
	pthread_mutex_lock(&skt_head.nmutex);
	list_add_tail(&nodp->list, &skt_head.hnode);
	list_add_tail(&nodp->plist, &attrp->cnode_list);
	pthread_mutex_unlock(&skt_head.nmutex);
	return 0;
}

int skt_send(void *name, void *sbuf, size_t slen)
{
	int ret = 0;
	struct skt_node *nodp = NULL;
//	printf("slen=%d\n", slen);
	nodp= list_for_search_node(name);
	if (!nodp) {
		printf("list for search node\n");
		return -ESKTSENDSEARCHNODE;
	}
	if (nodp->type==SOCK_DGRAM) {
		ret = sendto(nodp->fd, sbuf, slen, nodp->flags, (struct sockaddr *)&nodp->saddr, nodp->saddrlen);	
	} else {
		ret = send(nodp->fd, sbuf, slen, nodp->flags);
		if(ret<0) printf("SEND\n");
	}
	if (ret==0) ret = -3;
	return ret;
}
#if 0
static int recv_timeout(int fd, void *rbuf, size_t rlen, struct timeval *tout)
{
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = tout->tv_sec;
	tv.tv_usec = tout->tv_usec;

	retval = select(fd+1, &rfds, NULL, NULL, &tv);
	if (retval == -1){
		printf("select()");
		return -1;	
	} else if (retval) {
		if(FD_ISSET(fd, &rfds)) {
			return  recv(fd, rbuf, rlen, 0);
		}
	} else{
		printf("No data within five seconds.\n");
	}
	return 0;
}
#endif
int skt_recv(char *name, void *rbuf, size_t rlen)
{
	int ret = 0;
	struct skt_node *nodp = NULL;
	
	nodp= list_for_search_node(name);
	if (!nodp) {
		printf("list for search node\n");
		return -ESKTRECVSEARCHNODE;
	}


	if (nodp->type==SOCK_DGRAM) {
		ret = recvfrom(nodp->fd, rbuf, rlen, nodp->flags, (struct sockaddr *)&nodp->saddr, &nodp->saddrlen);	
		nodp->damain = nodp->saddr.sin_family ;
//		nodp->ip = nodp->saddr.sin_addr.s_addr;
		nodp->port = ntohs(nodp->saddr.sin_port);
	} else {
		ret = recv(nodp->fd, rbuf, rlen, nodp->flags);
		if (ret<0) printf("recverror\n");
	}
	if (ret==0) ret=-3;
	return ret;
}

void *skt_shutdown(char *name, int how)
{
	struct skt_node  *nodp = NULL;

	nodp = list_for_search_node(name);
	if (!nodp) {
		return NULL;
	}
	if (nodp->fd) shutdown(nodp->fd, how);
	return nodp;
}

void *skt_close_attr(char *name)
{
	struct skt_attr *attrp = NULL;
	struct skt_node *nodp = NULL;
	struct list_head *pos, *n;
	
	attrp = list_for_search_attr(name);
	if (!attrp) {
		return NULL;
	}
	if (attrp->fd != -1) close(attrp->fd);
	attrp->fd = -1;
	pthread_mutex_lock(&skt_head.nmutex);
	list_for_each_safe(pos, n, &attrp->cnode_list) {
		nodp = list_entry(pos, struct skt_node, plist);	
		list_del_init(&nodp->list);
		skt_free_node(nodp);
	}
	pthread_mutex_unlock(&skt_head.nmutex);
	return attrp;
}

void *skt_close_node(char *name)
{
	struct skt_node *nodp = NULL;

	nodp = list_for_search_node(name);
	if (!nodp) {
		return;
	}
	pthread_mutex_lock(&skt_head.nmutex);
	list_del_init(&nodp->list);
	pthread_mutex_unlock(&skt_head.nmutex);
	if (nodp->fd != -1) {
		close(nodp->fd);
	}
	nodp->fd = -1;
	if (nodp->name) {
		free(nodp->name);
		nodp->name = NULL;
	}
//	print_node();
	return nodp;
}


void skt_close_free_node(char *name)
{
	struct skt_node *nodp = NULL;

	nodp = skt_close_node(name);
	if (!nodp) {
		return;
	}
	skt_free_node(nodp);
	return ;
}


void print_attr(void)
{
	struct skt_attr  *cp = NULL;
	struct list_head *pos, *n;
	pthread_mutex_lock(&skt_head.amutex);
	list_for_each_safe(pos, n, &skt_head.hattr) {
		cp = list_entry(pos, struct skt_attr, attr_list);	
		printf("attr name:%s\n", cp->name);
	}	
	pthread_mutex_unlock(&skt_head.amutex);
	return ;
}

void print_node(void)
{
	struct skt_node  *cp = NULL;
	struct list_head *pos, *n;
	pthread_mutex_lock(&skt_head.nmutex);
	list_for_each_safe(pos, n, &skt_head.hnode) {
		cp = list_entry(pos, struct skt_node, list);	
		printf("node name:%s\n", cp->name);
	}	
	pthread_mutex_unlock(&skt_head.nmutex);
	return ;
}





