/*************************************************************************
	> File Name: sersion.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Sun 25 May 2014 09:44:02 PM PDT
 ************************************************************************/
#ifndef __BDVERSION__H__
#define __BDVERSION__H__


int sys_version_log_mode_init(char *name, char *str);
void sys_log_mode_init(const char *ident, int option, int facility);
void sys_log_mode_free(void);
#define sys_log(priority, fmt, args...) syslog(priority, fmt, ##args)
#endif
