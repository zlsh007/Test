/*************************************************************************
	> File Name: srclib/bversion.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Sun 25 May 2014 06:35:44 PM PDT
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#if 0
#define VERSION 1
#define PATCHLEVEL 0
#define SUBLEVEL 0
#define VERSION_RAND 1234
#endif
#define VERSION_FIX_SIZE  1024
//static char *vsp = NULL;
int sys_version_log_mode_init(char *name, char *str)
{
	char *vsp = malloc(VERSION_FIX_SIZE+strlen(name) +strlen(str));
	if (!vsp) return -1;
	memset(vsp, 0, VERSION_FIX_SIZE+strlen(name)+strlen(str));
	openlog(name, LOG_NDELAY, LOG_USER);
//	sprintf(vsp, "%s VERSION V%d.%d.%d_%d  (%s %s:%s) %s\n%s:%s\n%s:%s\n", name, VERSION, PATCHLEVEL, SUBLEVEL, EXTRAVERSION, __DATE__,__TIME__, USERNAME, str, name, MAKEVERSION, name, CCVERSION);
	sprintf(vsp, "%s VERSION V%d.%d.%d_%d  (%s %s:%s) %s\n%s:%s\n%s:%s\n", name, VERSION, PATCHLEVEL, SUBLEVEL, EXTRAVERSION,__DATE__,__TIME__, USERNAME, str, name, MAKEVERSION, name, CCVERSION);
	printf("%s\n", vsp);
	syslog(LOG_INFO, "%s", vsp);
	
	closelog();
	free(vsp);
	return 0;
}


void sys_log_mode_init(const char *ident, int option, int facility)
{
	openlog(ident, option, facility);
	return ;
}

void sys_log_mode_free(void)
{
	return closelog();
}
#if 0
int main(void)
{
	sys_version_log_mode_init("APP", "test version log");
	return 0;
}
#endif
