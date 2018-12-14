/*************************************************************************
	> File Name: cmd.h
	> Author:
	> Mail:
	> Created Time: Wed 05 Mar 2014 05:01:38 PM CST
 ************************************************************************/
#ifndef __CMD__H
#define __CMD__H

#include <time.h>
#include "types.h"

#define BE 1
#define LE 0
static bool big_end_test(void)  
{
        const u_int16_t n = 1;
        if(*(char *)&n)
                return LE;
        return BE;
}

#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))  
#define Swap32(l) (((l) >> 24) | \
                (((l) & 0x00ff0000) >> 8)  | \
                (((l) & 0x0000ff00) << 8)  | \
                ((l) << 24))  
#define Swap64(ll) (((ll) >> 56) |\
                (((ll) & 0x00ff000000000000) >> 40) |\
                (((ll) & 0x0000ff0000000000) >> 24) |\
                (((ll) & 0x000000ff00000000) >> 8)    |\
                (((ll) & 0x00000000ff000000) << 8)    |\
                (((ll) & 0x0000000000ff0000) << 24) |\
                (((ll) & 0x000000000000ff00) << 40) |\
                (((ll) << 56)))

#define BE_16(s) big_end_test() ? s : Swap16(s)
#define LE_16(s) big_end_test() ? Swap16(s) : s
#define BE_32(l) big_end_test() ? l : Swap32(l)
#define LE_32(l) big_end_test() ? Swap32(l) : l
#define BE_64(ll) big_end_test() ? ll : Swap64(ll)
#define LE_64(ll) big_end_test() ? Swap64(ll) : ll

time_t t;
char time_buf[1024];

void show_time(void)
{
        time(&t);
        ctime_r(&t, time_buf);
        printf("%s\n", time_buf);
}

void cmd_exec_login(struct iscsi_hdr *bhs, void *handle,
                char *buf, int *len);
void cmd_exec_text(struct iscsi_hdr *bhs, void *handle,
                char *buf, int *len);
void cmd_exec_scsi(struct iscsi_hdr *bhs,
                void *handle, char *buf, int *len, char *dir);
void cmd_exec_scsi_rsp(struct iscsi_hdr *bhs,
                void *handle, char *buf, int *len, char *dir);
void cmd_exec_scsi_data_in(struct iscsi_hdr *bhs,
                void *handle, char *buf, int *len, char *dir);
void cmd_exec_login_rsp(struct iscsi_hdr *bhs,
                void *handle, char *buf, int *len);
void cmd_exec_text_rsp(struct iscsi_hdr *bhs,
                void *handle, char *buf, int *len);
void cmd_exec_data_out(struct iscsi_hdr *bhs, void *name,
                char *buf, int *len);

int cmd_send(char *nn, char *data, int len);
struct iscsi_hdr* cmd_recv(char *nn);
#endif

