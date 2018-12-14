/*************************************************************************
	> File Name: sdlist.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Tue 11 Mar 2014 03:13:12 PM CST
 ************************************************************************/
#ifndef __SKTLIST__H__
#define __SKTLIST__H__
#include <stdio.h>
#include  <pthread.h>
#include "list.h"

struct sktnode{
	struct list_head list;
	char *name;
	void *rever;
};

struct sktlist{
	struct list_head  list;
	pthread_mutex_t  mutex;
	pthread_cond_t cond;
};


void sktlist_init(void);
void *alloc_sktnode(char *n);
int send_to_sktlist(void *cn);
char *get_from_sktlist(void);

#endif


