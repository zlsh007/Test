/*************************************************************************
	> File Name: client.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Wed 12 Mar 2014 05:45:30 PM CST
 ************************************************************************/
#ifndef CLIENT__H__
#define CLIENT__H__



int dp_client_sysinit(int);
int dp_client_sysclose(void);
int dpcreatdump(int ch,char *filename, char *buf, int len);
int dpappdump(int ch,char *filename, char *buf, int len);
int dpfilesize(int ch, char *filename);
int dploadfile(int ch, char *filename, char *buf, int len, int where);




#endif

