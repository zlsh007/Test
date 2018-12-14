/*************************************************************************
	> File Name: log.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Thu 13 Mar 2014 09:21:05 AM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "getenv.h"
#include "file_io.h"

const char *logcmd[]={"a", "--Connected---", "--Writecreat--", "--Writeappend-", "--Readfilesize","--Readfile----","--Disconnected", "--Errorcmd----", "--DPMSTART---", "--DPSRCVDATA--", "--DPSSNDDATA--"};

#define LOGFILENAMEMAX 255
pthread_mutex_t  logmutex;
int logsys_init(void)
{
	pthread_mutex_init(&logmutex, NULL);
	return 0;
}

int logwrite(char *lname, int cmd, char *ip, int port, char *buf)
{
	char tv[512] = {0};
	char logname[LOGFILENAMEMAX];
	char ipport[50] = {0};
	struct tm *tmp;
	time_t timep;
//	printf("logcmd[%d] = %s\n", cmd, logcmd[cmd]);
	
	time(&timep);
	tmp = gmtime(&timep);
	strftime(tv, 20, "%Y_%m%d_%H%M%S", tmp);
	
	memset(logname, 0, LOGFILENAMEMAX);
	//sprintf(logname, "%.*s/%.*s", strlen(get_data_dir()), get_data_dir(), strlen(lname), lname);
//	sprintf(logname, "%.*s/%.*s", strlen(get_logfilename()), get_logfilename(), strlen(lname), lname);
	sprintf(logname, "%.*s", strlen(lname), lname);

	sprintf(ipport, "---%.*s--%d---", strlen(ip), ip, port);
	strcat(tv, logcmd[cmd]);
	strcat(tv, ipport);
	strcat(tv, buf);
//	printf("----%s\n",  tv);
	strcat(tv, "\r\n");
	pthread_mutex_lock(&logmutex);
	writefile_append(logname, tv, strlen(tv)+1);
	pthread_mutex_unlock(&logmutex);
	return 0;
}


