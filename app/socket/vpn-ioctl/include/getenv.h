/*************************************************************************
	> File Name: getenv.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Tue 04 Mar 2014 05:25:13 PM CST
 ************************************************************************/
#ifndef __GETENV__H__
#define __GETENV__H__


#define DPMS_VERSSION "V1.2 2014-05-26 10:32"
int dp_env_init(void);

int get_dpsout_port(int i);
int get_dpmout_port(int i);
int get_dpsin_port(int i);
int get_dpmin_port(int i);
char *get_dpmin_ip(int i);
char *get_dpmout_ip(int i);
char *get_dpsin_ip(int i);
char *get_dpsout_ip(int i);

char *get_data_dir(void);
char *get_logfilename(void);
int get_dpmchannel_max(void);
int get_dpschannel_max(void);
#endif
