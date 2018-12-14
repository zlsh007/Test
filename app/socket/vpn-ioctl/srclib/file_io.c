/*************************************************************************
  > File Name: file_io.c
  > Author: guotao
  > Mail: guozengqiang163@163.com 
  > Created Time: Thu 06 Mar 2014 08:57:42 AM CST
 ************************************************************************/
#include <stdio.h>
 #include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "getenv.h"
#include <time.h>
#include "sktlist.h"
//#include "cmd.h"

#if 0
int open(const char *pathname, int flags, mode_t mode);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int access(const char *pathname, int mode);
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
#include <stdarg.h>
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

#if 0
#include "apue.h"
int lock_reg(int fd, int cmd, int ype, off_t offset, int whence, off_t len)
{
	struct flock lock;

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock_l_len = len;
	return (fcntl(fd, cmd, &lock));
}
pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock  lock;
	lock.l_type =type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	if (fcntl(fd, F_GETLK, &lock)<0) {
		perror("fcntl:");
		return -1;
	}
	if (lock.l_type == F_UNLOCK) {
		return (0);
	}
	return (lock.l_pid);
	
}
#endif
int mkfilename(char *str, const char *format, ...)
{
//	sprintf(str, format);

	return 0;
}
int create_filename(char *d, char *ip, int port, char *fn, char *buf, int len)
{
	char tbuf[50];
	struct tm *tmp;
	time_t timep;
	time(&timep);
	tmp = gmtime(&timep);
	memset(tbuf, 0, 50);
	//strftime(tbuf, 50, "%Y_%m%d_%H%M%S", tmp);
	strftime(tbuf, 50, "%Y_%m%d_%H", tmp);
	sprintf(d, "%.*s/%.*s_%d/%.*s", strlen(get_data_dir()), get_data_dir(), strlen(ip), ip, port, strlen(fn), fn);
	strcat(d,tbuf);
	return 0;
}

int writefile_append(const char *name, char *buf, int len)
{
	int fd;
	char tmpdir[100], tmpcmd[100];
	int wlen=0, ret = 0;
	
	memset(tmpdir, 0, 100);
	memset(tmpcmd, 0, 100);
	strcpy(tmpdir, name);
	tmpdir[strlen(name)-strlen(strrchr(name, '/'))]=0;
	sprintf(tmpcmd, "mkdir -p  %s", tmpdir);
	system(tmpcmd);	
	sync();
	chmod(name, S_IRWXU);
	sync();
	fd = open(name, O_WRONLY | O_APPEND |  O_CREAT, 0x644);
	if (fd<0) {
		perror("open file:");
		return 0;
	}
	while (wlen<len) {
		ret  = write(fd, buf+wlen, len-wlen);
		if (ret<0) {
			perror("Write file:");
			return -3;
		}
		wlen+=ret;
	}	
	close(fd);
	return len;
}
int writefile_new(const char *name, char *buf, int len)
{
	int fd;
	char tmpdir[100];
	char tmpcmd[100];
	int wlen=0, ret = 0;
	memset(tmpdir, 0, 100);
	memset(tmpcmd, 0, 100);
	strcpy(tmpdir, name);
	tmpdir[strlen(name)-strlen(strrchr(name, '/'))]=0;
	sprintf(tmpcmd, "mkdir -p  %.*s",strlen(tmpdir),  tmpdir);
	system(tmpcmd);	
	sync();
	
//	if (access(name, W_OK) && chmod(name, O_WRONLY) && access(name, W_OK)){
//		perror("access file:");
//	}
	chmod(name, S_IRWXU);
	sync();
	fd = open(name, O_WRONLY | O_CREAT, 0x644);
	if (fd<0) {
		perror("Open file:");
		return -2;
	}
	while (wlen<len) {
		ret  = write(fd, buf+wlen, len-wlen);
		if (ret<=0) {
			perror("Write file:");
			return -3;
		}
		wlen+=ret;
	}	
	close(fd);
	return len;
}
int readfile(const char *name, char *buf, int len)
{
	int fd;
	int ret;
	int leng = 0;
	if (access(name, O_RDONLY) && chmod(name, O_RDONLY) && access(name, O_RDONLY)){
		perror("access file:");
		return -1;
	}
	
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		perror("Open file:");
		return -2;
	}
	while(leng<len) {
		ret = read(fd, buf+leng, len-leng);
		if (ret<=0) break;
		leng+=ret;
	}
	
	close(fd);
	return leng;
}
int readfile_where(const char *name, char *buf, int len, char *ip, int port, int offset)
{
	int fd;
	int ret;
	int leng = 0;
	char tmp[100] = {0};
	sprintf(tmp, "%.*s/%.*s_%d/%.*s", strlen(get_data_dir()), get_data_dir(), strlen(ip), ip, port, strlen(name), name);
	if (access(tmp, O_RDONLY) && chmod(tmp, O_RDONLY) && access(tmp, O_RDONLY)){
		perror("access file");
		return -1;
	}
	
	fd = open(tmp, O_RDONLY);
	if (fd < 0) {
		perror("Open file");
		return -2;
	}
	printf("fd = %d, len = %d\n", fd, len);
	if (offset>0) {
		lseek(fd, offset, SEEK_SET);
	}
#if 1
	while(leng<len) {
		ret = read(fd, buf+leng, len-leng);
		if (ret<=0) break;
		leng+=ret;
	}
#endif
	//dump_data(buf, 5);
	
	close(fd);
	return leng;
}

int getfilesize(const char *name,  char *ip, int port)
{
	int ret = 0;
	struct stat s;
	char tmp[100] = {0};
	sprintf(tmp, "%.*s/%.*s_%d/%.*s", strlen(get_data_dir()), get_data_dir(), strlen(ip), ip, port, strlen(name), name);
//	printf("get file name:%s\n", tmp);
	ret = stat(tmp, &s);
	if (ret) {
		perror("stat error:");
		return -1;
	}
	return s.st_size;
}


