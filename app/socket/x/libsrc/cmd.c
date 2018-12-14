/*************************************************************************
	> File Name: cmd.c
	> Author: guotao
	> Mail: guozengqiang163@163.com :
	> Created Time: Wed 05 Mar 2014 04:48:06 PM CST
 ************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "types.h"
#include "iscsi_hdr.h"
#include "iscsid.h"
#include "socketlib.h"
#include "cmd.h"

static char *next_key(char **data, int *datasize, int *value)
{
	char *key, *p;
	int size = *datasize;

	key = p = *data;
        printf("key:%s\n", key);
	for (; size > 0 && *p != '='; p++, size--)
		;
	if (!size)
		return NULL;
        p++;
	size--;
	*value = size;

	for (; size > 0 && *p != 0; p++, size--) {
                printf("0x%02x ", *p);
        }
	if (!size)
		return NULL;
	p++;
	size--;

	*data = p;
	*datasize = size;

	return key;
}

static void text_scan_text(char *buf, int len, int *newlen)
{
	char *key, *data;
        int value = 0, totallen = len;
        char tmpbuf[] = {0x31, 0x30, 0x2e, 0x31, 0x30, 0x2e,
                0x31, 0x31, 0x38, 0x2e, 0x31, 0x31, 0x36, 0x3a,
                0x33, 0x32, 0x36, 0x30, 0x2c, 0x31, 0x00
        };
        struct iscsi_text_rsp_hdr *req;

	data = buf;

	while ((key = next_key(&data, &len, &value))) {
		if (!strncmp(key, "TargetAddress", 13)) {
                        printf("This key is TargetAddress value:%d\n", value);
                        memset(&buf[totallen-value], 0, value);
                        memcpy(&buf[totallen-value], tmpbuf, ARRAY_SIZE(tmpbuf));
                        req = (struct iscsi_text_rsp_hdr *)buf;
                        req->datalength[0] = (totallen-value+
                                        ARRAY_SIZE(tmpbuf)-BHS_SIZE) >> 16;
                        req->datalength[1] = (totallen-value +
                                        ARRAY_SIZE(tmpbuf)-BHS_SIZE) >> 8;
                        req->datalength[2] = (totallen-value+
                                        ARRAY_SIZE(tmpbuf)-BHS_SIZE);
                        *newlen = totallen - value + ARRAY_SIZE(tmpbuf);
                        return;
                }
        }
}

void cmd_exec_text_rsp(struct iscsi_hdr *bhs,
                void *name, char *buf, int *len)
{
	struct iscsi_text_rsp_hdr *req = (struct iscsi_text_rsp_hdr *)bhs;
        int datalength = 0;
        int ret = 0, newlen = *len;

        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (!datalength)
                return;
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);
        ret = skt_recv(name, &buf[BHS_SIZE], datalength);
        if (ret < 0) {
                printf("skt_recv ret = %d\n", ret);
                return;
        }
        *len += datalength;
        text_scan_text(buf, *len, &newlen);
        *len = newlen;
        printf("dump text rsp data:(%d)\n", *len);
}

void cmd_exec_login_rsp(struct iscsi_hdr *bhs,
                void *name, char *buf, int *len)
{
	struct iscsi_login_rsp_hdr *req = (struct iscsi_login_rsp_hdr *)bhs;
        int datalength = 0;
        int ret = 0;

        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (!datalength)
                return;
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);
        ret = skt_recv(name, &buf[BHS_SIZE], datalength);
        if (ret < 0) {
                printf("skt_recv ret = %d\n", ret);
                return;
        }
        *len += datalength;
        printf("dump login rsp data:(%d)\n", ret + BHS_SIZE);
}

void cmd_exec_scsi_data_in(struct iscsi_hdr *bhs,
                void *name, char *buf, int *len, char *dir)
{
	struct iscsi_data_in_hdr *req = (struct iscsi_data_in_hdr *)bhs;
        int datalength = 0;
        int ret = 0;

        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (!datalength)
                return;
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);
        if (datalength) {
                ret = skt_recv(name, &buf[BHS_SIZE], datalength);
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
        }
        *len += datalength;
        printf("dump scsi data in:(%d)\n", ret + BHS_SIZE);
}

void cmd_exec_data_out(struct iscsi_hdr *bhs, void *name,
                char *buf, int *len)
{
	struct iscsi_data_out_hdr *req = (struct iscsi_data_out_hdr *)bhs;
        int datalength;
        int ret;

        printf("ahslength:%d\n", req->ahslength);
        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);

        if (datalength) {
                ret = skt_recv(name, &buf[BHS_SIZE], datalength);
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
                *len += datalength;
        }
        printf("dump scsi data out:(%d)\n", ret + BHS_SIZE);
}

void cmd_exec_scsi_rsp(struct iscsi_hdr *bhs,
                void *name, char *buf, int *len, char *dir)
{
	struct iscsi_scsi_rsp_hdr *req = (struct iscsi_scsi_rsp_hdr *)bhs;
        struct iscsi_sense_data *sense_data = NULL;
        int data_length;
        int ret;

        data_length = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (!data_length)
                return;

        printf("dir:%s, data_length:%d, bi_residual:%d residual:%d\n",
                        SCST_DATA_DIR[(unsigned char)*dir],
                        data_length, req->bi_residual_count,
                        req->residual_count);
        
        if (data_length > 16) {
                ret = skt_recv(name, &buf[BHS_SIZE],
                                data_length - 16);
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
                *len += ret;
                printf("data_length > 16, dump new data:\n");
                dumpdata(buf, *len);
        }
        if (req->cmd_status != SAM_STAT_GOOD) {
                printf("cmd_status=%d\n", req->cmd_status);
                if (req->cmd_status == SAM_STAT_CHECK_CONDITION) {
                        sense_data = 
                                (struct iscsi_sense_data *)&buf[BHS_SIZE];
                        printf("sense_data length:(%d)\n",
                                        BE_16(sense_data->length));
                        ret = skt_recv(name, &buf[*len],
                               BE_16(sense_data->length) -
                               (ret - sizeof(sense_data->length)/sizeof(char)));
                        if (ret < 0) {
                                printf("skt_recv ret = %d\n", ret);
                                return;
                        }
                        *len += ret;
                }
        }
        printf("dump scsi rsp data:(%d)\n", *len);
}

void cmd_exec_scsi(struct iscsi_hdr *bhs,
                void *name, char *buf, int *len, char *dir)
{
	struct iscsi_scsi_cmd_hdr *req = (struct iscsi_scsi_cmd_hdr *)bhs;
        int datalength = 0;
        int ret = 0;

        if (req->flags & ISCSI_CMD_READ)
                *dir = SCST_DATA_READ;
        else if (req->flags & ISCSI_CMD_WRITE)
                *dir = SCST_DATA_WRITE;
        else if ((req->flags & ISCSI_CMD_READ) &&
                        (req->flags & ISCSI_CMD_WRITE))
                *dir = SCST_DATA_BIDI;
        else
                *dir = SCST_DATA_NONE;

        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];

        printf("dir:%s, datasegmentlen:%d, expected data_length:%d\n",
                        SCST_DATA_DIR[(unsigned char)*dir],
                        datalength, BE_32(req->data_length));
        
        if (datalength && (req->data_length > 16)) {
                if (*dir == SCST_DATA_READ) {
                        ret = skt_recv(name, &buf[BHS_SIZE],
                                        BE_32(req->data_length) - 16);
                } else if (*dir == SCST_DATA_WRITE) {
                        ret = skt_recv(name, &buf[BHS_SIZE],
                                        datalength);
                }
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
                printf("dump initor %s data:(%d)\n",
                                SCST_DATA_DIR[(unsigned char)*dir], ret);
                dumpdata(&buf[BHS_SIZE], ret);
                *len += ret;
        }
}

void cmd_exec_text(struct iscsi_hdr *bhs, void *name,
                char *buf, int *len)
{
	struct iscsi_text_req_hdr *req = (struct iscsi_text_req_hdr *)bhs;
        int datalength;
        int ret;

        printf("ahslength:%d\n", req->ahslength);
        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);

        if (datalength) {
                ret = skt_recv(name, &buf[BHS_SIZE], datalength);
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
                dumpdata(&buf[BHS_SIZE], ret);
                *len += datalength;
        }
        printf("dump test data:(%d)\n", ret + BHS_SIZE);
}

void cmd_exec_login(struct iscsi_hdr *bhs, void *name,
                char *buf, int *len)
{
	struct iscsi_login_req_hdr *req = (struct iscsi_login_req_hdr *)bhs;
        int datalength;
        int ret;

        printf("ahslength:%d\n", req->ahslength);
        datalength = (req->datalength[0]<<16) +
                (req->datalength[1]<<8) + req->datalength[2];
        if (datalength%4)
                datalength += 4 - datalength%4;
        printf("datalength:%d\n", datalength);

        if (datalength) {
                ret = skt_recv(name, &buf[BHS_SIZE], datalength);
                if (ret < 0) {
                        printf("skt_recv ret = %d\n", ret);
                        return;
                }
                dumpdata(&buf[BHS_SIZE], ret);
                *len += datalength;
        }
        printf("dump login data:(%d)\n", ret + BHS_SIZE);
}

int cmd_send(char *nn, char *data, int len)
{
	int ret;
	
	ret = skt_send(nn, data, len);
	if (ret < 0) {
                return -1;
        }
	return ret;
}

struct iscsi_hdr *cmd_recv(char *nn)
{
	struct iscsi_hdr *cmdp = NULL;
	int ret;

	cmdp = malloc(BHS_SIZE);
	if (!cmdp) {
		perror("MALLOC");
		return NULL;
	}
	memset(cmdp, 0, BHS_SIZE);
	ret = skt_recv(nn, cmdp, BHS_SIZE);
        if (ret < 0) {
                printf("ERR In cmd_recv skt_recv err(%d)\n", ret);
                free(cmdp);
                return NULL;
        } else if (ret != BHS_SIZE)
                printf("ERR In cmd_recv skt_recv not enougth(%d)\n", ret);

        return cmdp;
}
