/*************************************************************************
	> File Name: cmd.c
	> Author: guotao
	> Mail: guozengqiang163@163.com :
	> Created Time: Wed 05 Mar 2014 04:48:06 PM CST
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"
#include "socketlib.h"
#include "hear.h"
void dump_data(const char *buf, int len)
{
	int i;
//	printf("---------------------------------dump data------------------------------------\n");
	for (i = 0; i< len; i++){
		if (i % 10 == 0) printf("\n");
		printf("%d   ", buf[i]);
	}
	printf("\n");
	return ;
}
static char checkcrc(char *buf, int len)
{
	int i;
	char sum = 0;

	for (i = 0; i< len; i++) {
		sum ^= buf[i];
	}
	return sum & 0xff;
}
int cmd_send(const char *nn, char cmd, char *data, int len)
{
	int ret;
	struct gpiocmd *cmdp = NULL;
	if (len<0) return len;
	cmdp = malloc(sizeof(struct gpiocmd)+len+2);
	if (!cmdp) {
		return -1;
	}
	memset(cmdp, 0, sizeof(struct gpiocmd)+len+2);
	cmdp->start = CMD_START;
	cmdp->cmd = cmd;
	cmdp->subcmd = 0;
	cmdp->crc = 0;
	cmdp->len = len;
	cmdp->crc = checkcrc((char *)cmdp, 8);
	if (len>0) {
		memcpy(cmdp->data, data, len);
	}
	ret = skt_send(nn, cmdp, sizeof(struct gpiocmd)+len);
	if (ret<0) {free(cmdp);return -1;}
	free(cmdp);
	return ret;
}

struct gpiocmd* cmd_recv(const char *nn)
{
	int ret;
	struct gpiocmd cmd;
	struct gpiocmd *cmdp = NULL;
	char crc = 0;
	HEAR("\n");
cmd_recv_start:
	while(1) {
		if (skt_recv(nn, &cmd, 1)<0) {
			return NULL;
		}
		if (cmd.start == CMD_START) break;
	};
	
	ret = skt_recv(nn, &cmd.cmd, sizeof(struct gpiocmd)-1);
	if (ret<0) {
		return NULL;
	}
	crc = cmd.crc;
	cmd.crc = 0;
	if (crc != checkcrc(&cmd.start, 8)) goto cmd_recv_start;

	if (cmd.len<0) {
		return NULL;
	}

	cmdp = malloc(sizeof(struct gpiocmd)+cmd.len);
	if (!cmdp) {
		perror("MALLOC");
		return NULL;
	}
	memset(cmdp, 0, sizeof(struct gpiocmd)+cmd.len);
	memcpy(cmdp, &cmd, sizeof(struct gpiocmd));	
	if (0==cmd.len) return cmdp;
	ret = skt_recv(nn, cmdp->data, cmd.len);
	if (ret< 0) {
		free(cmdp);
		return NULL;
	}
	
	return cmdp;
}


