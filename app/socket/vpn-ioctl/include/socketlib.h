/*************************************************************************
	> File Name: socketlib.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Tue 24 Dec 2013 11:11:53 AM CST
 ************************************************************************/
#ifndef _SOCKETLIB_H
#define _SOCKETLIB_H

#include "apue.h"
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef __cplusplus
extern  "C" {
#endif

int skt_sys_init(const char *sys_name);

void skt_sys_free(const char *sysname);
void *skt_alloc_attr(const int type, const int port);
void *skt_alloc_node(const int type, const char *sip, const int port);
int skt_free_attr(const void *attrp);
int skt_free_node(const void *nodp);
int skt_attr_regester(const void *attr, const char *name);
int skt_node_regester(const void *node, const char *name);
void *list_for_search_attr(const char *name);
void *list_for_search_node(const char *name);
int skt_accept(const char *attrname, const char *nodename);
int skt_send(const void *name, void *sbuf, size_t slen);
int skt_recv(const char *name, void *rbuf, size_t rlen);

void *skt_shutdown(const char *name, int how);
void *skt_close_attr(const char *name);
void *skt_close_node(const char *name);

void skt_close_free_node(char *name);
char *get_node_ip(const char *name);
void print_node(void);
void print_attr(void);

#ifdef __cplusplus
}
#endif

#endif
