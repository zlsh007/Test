/*************************************************************************
	> File Name: service.c
	> Author: suhua
	> Mail:  
	> Created Time: 2018年9月25日 星期一 09时49分21秒
 ************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "types.h"
#include "iscsi_hdr.h"
#include "iscsid.h"
#include "socketlib.h"
#include "sktlist.h"
#include "cmd.h"

void accept_create_cleanup(void *args)
{
        printf("pthread clean up:\n");
        printf("pthread name:accept_create_cleanup, ptid=%u\n",
                        (unsigned int)pthread_self());
        return ;
}
/*
 * recv data from target, send data to A board
 */
void *skt_send_fn_cli(void *args)
{
        struct fifo_hdr *f_h = NULL;
        struct iscsi_hdr *bhs = NULL;
        int thread_num = *(int *)args;
	char *buf = NULL;
        char *fifoname = NULL;
        char sktnnp[10];
        int fd_storage;
        int ret = 0, len = 0;
        char dir = 0;

        pthread_cleanup_push(accept_create_cleanup, args);

        fifoname = malloc(FIFO_NAME_LEN);
        memset(fifoname, 0, FIFO_NAME_LEN);
        sprintf(fifoname, "%s_%d", FIFO_SS_NAME, thread_num);

        fd_storage = open(fifoname, O_WRONLY);
        if (fd_storage == -1) {
                perror("open");
                pthread_exit(0);
        }

        buf = malloc(ARRAY_SIZE_MAX);
        if (!buf) {
                perror("malloc\n");
                unlink("storage_data");
        }

        f_h = malloc(sizeof(f_h)+SKT_NAME_LEN);
        if (f_h == NULL) {
                perror("malloc\n");
                free(buf);
                pthread_exit(0);
        }
        memset(sktnnp, 0, 10);
        sprintf(sktnnp, "%s_%d", SKT_NODE_NAME, thread_num);

        while (1) {
                /* recv rsp from storage */
	        ret = skt_recv(sktnnp, buf, BHS_SIZE);
                if (ret != BHS_SIZE) {
                        printf("ERR skt_recv not enougth(%d)\n", ret);
                }
                printf("============recv rsp from storage=============\n");
                show_time();

                bhs = (struct iscsi_hdr *)buf;

                printf("dump BHS data from storage:(%d)\n", ret);
		dumpdata((char *)bhs, ret);
                len = BHS_SIZE;

                bhs = (struct iscsi_hdr *)buf;
                switch (bhs->opcode & ISCSI_OPCODE_MASK) {
                case ISCSI_OP_NOP_IN:
                        printf("ISCSI_OP_NOP_IN\n");
                        break;

                case ISCSI_OP_SCSI_RSP:
                        printf("ISCSI_OP_SCSI_RSP\n");
                        cmd_exec_scsi_rsp(bhs, sktnnp, buf, &len, &dir);
                        if (dir == SCST_DATA_READ)
                                f_h->decrypt = 1;
                        break;

                case ISCSI_OP_SCSI_TASK_MGT_RSP:
                        printf("ISCSI_OP_SCSI_TASK_MGT_RSP\n");
                        break;

                case ISCSI_OP_LOGIN_RSP:
                        printf("ISCSI_OP_LOGIN_RSP\n");
                        cmd_exec_login_rsp(bhs, sktnnp, buf, &len);
                        break;

                case ISCSI_OP_TEXT_RSP:
                        printf("ISCSI_OP_TEXT_RSP\n");
                        cmd_exec_text_rsp(bhs, sktnnp, buf, &len);
                        break;

                case ISCSI_OP_SCSI_DATA_IN:
                        printf("ISCSI_OP_SCSI_DATA_IN\n");
                        cmd_exec_scsi_data_in(bhs, sktnnp, buf, &len, &dir);
                        break;

                case ISCSI_OP_LOGOUT_RSP:
                        printf("ISCSI_OP_LOGOUT_RSP\n");
                        break;

                case ISCSI_OP_R2T_RSP:
                        printf("ISCSI_OP_R2T_RSP\n");
                        break;

                case ISCSI_OP_ASYNC_EVENT:
                        printf("ISCSI_OP_ASYNC_EVENT\n");
                        break;

                case ISCSI_OP_REJECT_MSG:
                        printf("ISCSI_OP_REJECT_MSG\n");
                        break;

                default:
                        printf("opcode is err\n");
                        sleep(60);
                }
                /* send rsp to A board */
		dumpdata(buf, len);
                f_h->hdr_size = len;
                memcpy(f_h->name, sktnnp, strlen(sktnnp)+1);
                printf("============send rsp(%d) to A board(%s)============\n\n",
                                len, f_h->name);
                write(fd_storage, f_h, sizeof(f_h)+SKT_NAME_LEN);
                write(fd_storage, buf, f_h->hdr_size);

                memset(buf, 0, ARRAY_SIZE_MAX);
        }
        free(f_h);
        free(buf);
        pthread_cleanup_pop(0);
        pthread_exit(0);
}
/*
 * recv data from A board, send data to target
 */
void *skt_recv_fn_cli(void *args)
{
        struct skt_node *sktnodep = NULL;
        struct fifo_hdr *f_h = NULL;
        int thread_num = *(int *)args;
        char *fifoname = NULL;
	char *buf = NULL;
        char sktnnp[10];
        int fd_socket = 0;
        int ret = 0;

        pthread_cleanup_push(accept_create_cleanup, args);

        fifoname = malloc(FIFO_NAME_LEN);
        memset(fifoname, 0, FIFO_NAME_LEN);
        sprintf(fifoname, "%s_%d", FIFO_SR_NAME, thread_num);

        fd_socket = open(fifoname, O_RDONLY);
        if (fd_socket == -1) {
                perror("open");
                pthread_exit(0);
        }

        buf = malloc(ARRAY_SIZE_MAX);
        if (!buf) {
                perror("malloc\n");
                close(fd_socket);
                pthread_exit(0);
        }

        sktnodep = skt_alloc_node(SOCK_STREAM, STORAGE_SERVER_IP,
                        STORAGE_SERVER_PORT);
        if (!sktnodep) {
                printf("SKT ALLOC NODE ERROR, PORT:%d\n",
                                STORAGE_SERVER_PORT);
                free(buf);
                pthread_exit(0);
        }
        memset(sktnnp, 0, 10);
        sprintf(sktnnp, "%s_%d", SKT_NODE_NAME, thread_num);

        f_h = malloc(sizeof(f_h)+SKT_NAME_LEN);
        if (f_h == NULL) {
                perror("alloc f_h failed\n");
                free(buf);
                pthread_exit(0);
        }

        ret = skt_node_register(sktnodep, sktnnp);
        if (ret) {
                printf("SKT REGISTER ERROR\n");
                free(buf);
                free(f_h);
                skt_free_node(sktnodep);
                pthread_exit(0);
        }

        while (1) {
                /* recv data from A board */
                read(fd_socket, f_h, sizeof(f_h)+SKT_NAME_LEN);
                if (f_h->hdr_size == 0) {
                        printf("ERR recv_thread size is 0(%u)\n",
                                        (unsigned int)pthread_self());
                        break;
                }

                printf("\n------------read fifo data from A board------------\n");
                show_time();
                printf("need read size:%d, name:%s\n", f_h->hdr_size,
                                f_h->name);
                ret = read(fd_socket, buf, f_h->hdr_size);
                if (f_h->hdr_size != f_h->hdr_size) {
                        printf("ERR recv_thread not enougth(%d-%d)\n",
                                        ret, (int)pthread_self());
                        break;
                }
                printf("dump A board data:(%d)\n", f_h->hdr_size);
                dumpdata(buf, f_h->hdr_size);

                /* send data to storage */
                printf("\nsend (%d) data to storage[%s]\n", f_h->hdr_size,
                                sktnnp);
                ret = skt_send(sktnnp, buf, f_h->hdr_size);
                if (ret < 0) {
                        printf("skt send ret = %d\n", ret);
                        break;
                }
                memset(f_h->name, 0, strlen(sktnnp)+1);
                memset(buf, 0, ARRAY_SIZE_MAX);
        }
        free(f_h);
        free(buf);
        pthread_cleanup_pop(0);
        pthread_exit(0);
}

int recv_thread_create_cli(void)
{
        int i, ret;
        struct pthread_skt *pskt = NULL;

        pskt = malloc(sizeof(*pskt));
        memset(pskt, 0, sizeof(struct pthread_skt));

        ret = pthread_attr_init(&pskt->pattr);
        if (ret != 0) {
                perror("pthread_attr_init\n");
                return ret;
        }

        ret = pthread_attr_setdetachstate(&pskt->pattr, PTHREAD_CREATE_DETACHED);
        if (ret == 0) {
                for (i = 0; i < SKT_CLIENT_MAX; i++) {	
                        pskt->thread_num[i] = i;
                        ret = pthread_create(&pskt->ctid[i], &pskt->pattr,
                                        skt_recv_fn_cli, (void*)(&pskt->thread_num[i]));
                }
        }
        pthread_attr_destroy(&pskt->pattr);
        return 0;
}

int send_thread_create_cli(void)
{
        struct pthread_skt *pskt = NULL;
        int i, ret;

        pskt = malloc(sizeof(*pskt));
        memset(pskt, 0, sizeof(struct pthread_skt));

        ret = pthread_attr_init(&pskt->pattr);
        if (ret != 0) {
                printf("pthread_attr_init failed\n");
                return ret;
        }

        ret = pthread_attr_setdetachstate(&pskt->pattr, PTHREAD_CREATE_DETACHED);
        if (ret == 0) {
                for (i = 0; i < SKT_CLIENT_MAX; i++) {	
                        pskt->thread_num[i] = i;
                        ret = pthread_create(&pskt->ctid[i], &pskt->pattr,
                                        skt_send_fn_cli, (void*)(&pskt->thread_num[i]));
                }
        }
        pthread_attr_destroy(&pskt->pattr);
        return 0;
}

int main(int argc, char **argv)
{
        int i,j;

        for (j=0; j<5; j++) {
                for (i=0; i<180; i++)
                        printf("+");
                printf("\n");
        }


        sktlist_init();
        skt_sys_init("ISCSI_CLIENT");

        recv_thread_create_cli();
        sleep(3);
        send_thread_create_cli();

        while (1)
                sleep(10);

        return 0;
}
