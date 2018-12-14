/*************************************************************************
	> File Name: sdlist.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Tue 11 Mar 2014 03:11:52 PM CST
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>


#include "list.h"
#include "sktlist.h"
#include "hear.h"

struct sktlist slist;

void sktlist_init(void)
{
	slist.list.prev = &slist.list;
	slist.list.next = &slist.list;
	pthread_mutex_init(&slist.mutex, NULL);
	pthread_cond_init(&slist.cond, NULL);	
	return ;
}

void *alloc_sktnode(char *n)
{
	struct sktnode *p = NULL;
	if (!n) return NULL;
	p = malloc(sizeof(struct sktnode));
	if (!p) return NULL;
	memset(p, 0, sizeof(struct sktnode));
	p->list.prev = &p->list;
	p->list.next = &p->list;
	p->name=malloc(strlen(n)+2);
	if (!p->name) {
		free(p);
		return NULL;
	}
	memset(p->name, 0, strlen(n)+2);
	memcpy(p->name, n, strlen(n));
	return p;
}

int send_to_sktlist(void *c)
{
	struct sktnode *cn = c;
	pthread_mutex_lock(&slist.mutex);
	list_add_tail(&cn->list,&slist.list);
	pthread_mutex_unlock(&slist.mutex);
	pthread_cond_signal(&slist.cond);
	return 0;
}	

char* get_from_sktlist(void)
{
	struct sktnode *np = NULL;
	char *p = NULL;
	
	pthread_mutex_lock(&slist.mutex);
	while (list_empty(&slist.list))
	pthread_cond_wait(&slist.cond, &slist.mutex);
	
	np = list_entry(slist.list.next,struct sktnode, list);
	list_del_init(&np->list);
	pthread_mutex_unlock(&slist.mutex);
	
	p = malloc(strlen(np->name)+2);
	memset(p, 0, strlen(np->name)+2);
	memcpy(p, np->name, strlen(np->name));
	
	return p;
}


