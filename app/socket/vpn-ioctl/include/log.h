/*************************************************************************
	> File Name: include/log.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Fri 28 Mar 2014 01:40:11 PM CST
 ************************************************************************/
#ifndef __LOG__H__
#define __LOG__H__

#define LOGCMD_CONNECT 1
#define LOGCMD_WRITECREATE 2
#define LOGCMD_WRITEAPPEND 3
#define LOGCMD_READFILESIZE 4
#define LOGCMD_READFILE 5
#define LOGCMD_DISCONNECT 6
#define LOGCMD_ERROR 7
#define LOGCMD_DPMSTART 8
#define LOGCMD_DPSRCVDATA 9
#define LOGCMD_DPSSNDDATA 10



int logsys_init(void);
int logwrite(char *lname, int cmd, char *ip, int port, char *buf);

#endif


