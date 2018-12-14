/*************************************************************************
	> File Name: cli.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Sat 01 Mar 2014 10:21:39 AM CST
 ************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include "socketlib.h"
#include "getenv.h"
#include "cmd.h"


#define DPCINNODENAME "DPCINNODESTR"
#define DPCOUTNODENAME "DPCOUTNODESTR"
#define SNDBUFSIZE 2036

struct dpc_channel_ctx{
	int maxchannel;
	int channel;
	char *sndnamep;
	char *rcvnamep;
	pthread_mutex_t smutex;
	pthread_mutex_t rmutex;
}; 

static struct dpc_channel_ctx *dpcctxp = NULL;
static char *getnodename(char *str, int num)
{
	char *p = malloc(strlen(str)+4);
	if (!p) {
		perror("GET NODE NAME MALLOC");
		return NULL;
	}
	memset(p, 0, strlen(str)+4);
	sprintf(p, "%s%d", str, num);
	return p;
}

int dp_client_sysinit(int max)
{
	void *nodp = NULL;
	int i;
	int ret=0;
	struct dpc_channel_ctx *ctxp = NULL;
	printf("DPM VERSSION %s\n", DPMS_VERSSION);
	dp_env_init();
	
	skt_sys_init("DPC_CLIENT_SYS");
	
	dpcctxp = malloc(sizeof(struct dpc_channel_ctx)*max);
	if (!dpcctxp) {
		perror("MALLOC");
		return -1;
	}
	memset(dpcctxp, 0, sizeof(struct dpc_channel_ctx)*max);
	for (ctxp = dpcctxp, i = 0; i < max; ctxp++, i++) {
		ctxp->maxchannel = max;
		ctxp->channel = i;
		pthread_mutex_init(&ctxp->smutex, NULL);
		pthread_mutex_init(&dpcctxp->rmutex, NULL);
		ctxp->rcvnamep = getnodename(DPCINNODENAME, i);
		ctxp->sndnamep = getnodename(DPCOUTNODENAME, i);
		
		nodp = skt_alloc_node(SOCK_STREAM, get_dpsin_ip(i), get_dpsin_port(i));
		if (!nodp) {
			printf("skt_alloc_node %d ret error\n", i);
			return -1;
		}
		ret = skt_node_regester(nodp, ctxp->sndnamep);
		skt_shutdown(ctxp->sndnamep, SHUT_RD);
	
		nodp = skt_alloc_node(SOCK_STREAM, get_dpsout_ip(i), get_dpsout_port(i));
		if (!nodp) {
			printf("skt_alloc_node %d ret error\n", i);
			return -1;
		}
		ret = skt_node_regester(nodp, ctxp->rcvnamep);
		skt_shutdown(ctxp->rcvnamep, SHUT_WR);
	}	
	
	return ret;
}

int dp_client_sysclose(void)
{
	skt_sys_free("DPC_CLIENT_SYS");
	return 0;
}
int dpcreatdump(int ch, char *filename, char *buf, int len)
{
	int slen = 0;
	int ret = 0;
	struct dpc_channel_ctx *ctxp = dpcctxp + ch;
	if (dpcctxp->maxchannel<=ch) {
		printf("ERROR CHANNEL:%d\n", ch);
		return -1;
	}
	if (len>SNDBUFSIZE) {
		pthread_mutex_lock(&ctxp->smutex);
		ret = dp_send(ctxp->sndnamep, CMDSIGNSTART,CREATWRFILEREQ, strlen(filename)+1, filename,  SNDBUFSIZE, buf+slen, 0);	
		pthread_mutex_unlock(&ctxp->smutex);
		if (ret<0) return ret;
		slen+=ret;
		slen-=(13+strlen(filename));
		while ((len-slen) > SNDBUFSIZE) {
			pthread_mutex_lock(&ctxp->smutex);
			ret = dp_send(ctxp->sndnamep, CMDSIGNMID,CREATWRFILEREQ, strlen(filename)+1, filename,  SNDBUFSIZE, buf+slen, 0);	
			pthread_mutex_unlock(&ctxp->smutex);
			if (ret<0) return ret;
			slen+=ret;
			slen-=(13+strlen(filename));
		}
		if (len-slen>0) {
			pthread_mutex_lock(&ctxp->smutex);
			ret = dp_send(ctxp->sndnamep, CMDSIGNEND,CREATWRFILEREQ, strlen(filename)+1, filename,  len-slen, buf+slen, 0);	
			pthread_mutex_unlock(&ctxp->smutex);
			if (ret<0) return ret;
			slen+=ret;
			slen-=(13+strlen(filename));
		}
	} else {
		pthread_mutex_lock(&ctxp->smutex);
		slen = dp_send(ctxp->sndnamep, CMDSIGNONLY,CREATWRFILEREQ, strlen(filename)+1, filename,  len-slen, buf+slen, 0);	
		pthread_mutex_unlock(&ctxp->smutex);
		slen-=(13+strlen(filename));
	}
	return slen;
}

int dpappdump(int ch, char *filename, char *buf, int len)
{
	int slen = 0;
	int ret = 0;
	struct dpc_channel_ctx *ctxp = dpcctxp+ch;
	if (dpcctxp->maxchannel<=ch) {
		printf("ERROR CHANNEL:%d\n", ch);
		return -1;
	}
	if (len>SNDBUFSIZE) {
		pthread_mutex_lock(&ctxp->smutex);
		ret = dp_send(dpcctxp[ch].sndnamep, CMDSIGNSTART,APPENDWRFILEREQ, strlen(filename)+1, filename,  SNDBUFSIZE, buf+slen, 0);	
		pthread_mutex_unlock(&ctxp->smutex);
		if (ret<0) return slen;
		slen+=ret;
		slen-=(13+strlen(filename));
		while ((len-slen) > SNDBUFSIZE) {
			pthread_mutex_lock(&ctxp->smutex);
			ret = dp_send(ctxp->sndnamep, CMDSIGNMID,APPENDWRFILEREQ, strlen(filename)+1, filename,  SNDBUFSIZE, buf+slen, 0);	
			pthread_mutex_unlock(&ctxp->smutex);
			if (ret<0) return slen;
			slen+=ret;
			slen-=(13+strlen(filename));
		}
		if (len-slen>0) {
			pthread_mutex_lock(&ctxp->smutex);
			ret = dp_send(ctxp->sndnamep, CMDSIGNEND,APPENDWRFILEREQ, strlen(filename)+1, filename,  len-slen, buf+slen, 0);	
			pthread_mutex_unlock(&ctxp->smutex);
			if (ret<0) return slen;
			slen+=ret;
			slen-=(13+strlen(filename));
		}
	} else {
		pthread_mutex_lock(&ctxp->smutex);
		slen = dp_send(ctxp->sndnamep, CMDSIGNONLY,CREATWRFILEREQ, strlen(filename)+1, filename,  len-slen, buf+slen, 0);	
		pthread_mutex_unlock(&ctxp->smutex);
		slen-=(13+strlen(filename));
	}
	return slen;
}

int dpfilesize(int ch, char *filename)
{
	int ret;
	struct dpcmd *cmdp = NULL;
	struct dpc_channel_ctx *ctxp = dpcctxp+ch;
	if (dpcctxp->maxchannel<=ch) {
		printf("ERROR CHANNEL:%d\n", ch);
		return -1;
	}
	pthread_mutex_lock(&ctxp->smutex);
	ret = dp_send(ctxp->sndnamep, CMDSIGNONLY,GETFILESIZEREQ, strlen(filename)+1, filename,  0, NULL, 0);	
	if (ret<=0) {
		pthread_mutex_unlock(&ctxp->smutex);
		return -1;
	}
	cmdp = dp_recv(ctxp->rcvnamep);
	pthread_mutex_unlock(&ctxp->smutex);
	if (!cmdp) {
		return -2;
	}
	
	return cmdp->offset;
}

int dploadfile(int ch, char *filename, char *buf, int len, int where)
{
	int ret = 0;
	struct dpcmd *cmdp = NULL;
	struct dpc_channel_ctx *ctxp = dpcctxp+ch;
	if (dpcctxp->maxchannel<=ch) {
		printf("ERROR CHANNEL:%d\n", ch);
		return -1;
	}
	if (len>2035) return -3;
	pthread_mutex_lock(&ctxp->smutex);
	ret = dp_send(ctxp->sndnamep, CMDSIGNSTART,GETFILEDATAREQ, strlen(filename)+1, filename,  len, NULL, where);	
	if (ret<=0) {
		pthread_mutex_unlock(&ctxp->smutex);
		return -1;
	}	
	cmdp = dp_recv(ctxp->rcvnamep);
	pthread_mutex_unlock(&ctxp->smutex);
	if (!cmdp) return -2;
	
	if ((unsigned char)cmdp->cmd != GETFILEDATAACK) {
		free(cmdp);
		return -1;
	}
	memcpy(buf, cmdp->buf+strlen(filename)+1, cmdp->dlen);
	free(cmdp);
	return cmdp->dlen;
}





