/*************************************************************************
	> File Name: file_io.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Thu 06 Mar 2014 09:54:11 AM CST
 ************************************************************************/
#ifndef FILE__IO__H
#define FILE__IO__H

int writefile_append(const char *name, char *buf, int len);
int writefile_new(const char *name, char *buf, int len);
int readfile(const char *name, char *buf, int len);
int readfile_where(const char *name, char *buf, int len, char *ip, int port, int offset);

int create_filename(char *d, char *ip, int port, char *fn);
//int getfilesize(const char *name);
int getfilesize(const char *name,  char *ip, int port);



#endif
