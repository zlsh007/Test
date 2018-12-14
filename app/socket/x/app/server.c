/*************************************************************************
  > File Name:
  > Author: suhua
  > Mail: suhua.tanke@gmail.com
  > Created Time: Sat 01 Mar 2018 10:21:39 AM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "iscsid.h"
#include "sktlist.h"
#include "socketlib.h"
#include "iscsi_hdr.h"
#include "cmd.h"

void accept_create_cleanup(void *args)
{
        printf("pthread clean up:\n");
        printf("pthread name:accept_create_cleanup, ptid=%u\n",
                        (unsigned int)pthread_self());
        return ;
}

void *skt_send_fn_ser(void *args)
{
        int thread_num = *(int *)args;
        struct fifo_hdr *f_h = NULL;
        char *fifoname = NULL;
	char *buf = NULL;
        int fd_storage = 0;
        int ret = 0;

        pthread_cleanup_push(accept_create_cleanup, args);

        fifoname = malloc(FIFO_NAME_LEN);
        memset(fifoname, 0, FIFO_NAME_LEN);
        sprintf(fifoname, "%s_%d", FIFO_SS_NAME, thread_num);

        buf = malloc(ARRAY_SIZE_MAX);
        if (!buf) {
                perror("malloc\n");
        }
        /* fifo send data to A board */
        unlink(fifoname);
        ret = mkfifo(fifoname, 0666);
        if (ret == -1) {
                perror("mkfifo\n");
                pthread_exit(0);
        }

        fd_storage = open(fifoname, O_RDONLY);
        if (fd_storage == -1) {
                perror("open");
                unlink(fifoname);
                pthread_exit(0);
        }

        f_h = malloc(sizeof(f_h)+SKT_NAME_LEN);
        if (f_h == NULL) {
                perror("alloc f_h failed\n");
                unlink(fifoname);
                pthread_exit(0);
        }

        while (1) {
                /* recv rsp from B board, if have */
                ret = read(fd_storage, f_h, sizeof(f_h)+SKT_NAME_LEN);
                if (ret != (sizeof(f_h)+SKT_NAME_LEN)) {
                        perror("ERR read f_h\n");
                        break;
                }
                printf("\n\n-----------begin read data from B board(%d)-----------\n",
                                thread_num);
                show_time();

                printf("f_h->name:%s, size:%d\n", f_h->name,
                                f_h->hdr_size);
                ret = read(fd_storage, buf, f_h->hdr_size);
                if (ret != f_h->hdr_size) {
                        perror("ERR read buf\n");
                        break;
                }
                
                printf("dump rsp data from B board:(%d)\n", f_h->hdr_size);
                dumpdata(buf, f_h->hdr_size);

                printf("send data to initor:(%d)\n", f_h->hdr_size);
                ret = skt_send(f_h->name, buf, f_h->hdr_size);
                if (ret < 0) {
                        continue;
                }

        }
        free(f_h);
        free(buf);
        pthread_cleanup_pop(0);
        pthread_exit(0);
}

void *skt_recv_fn_ser(void *args)
{
        int thread_num = *(int *)args;
        struct iscsi_hdr *bhs = NULL;
        struct fifo_hdr *f_h = NULL;
        char *sktnnp = NULL;
        char *fifoname = NULL;
	char *buf = NULL;
        int ret, len;
        int fd_socket;
        char dir = 0;

        pthread_cleanup_push(accept_create_cleanup, args);

        fifoname = malloc(FIFO_NAME_LEN);
        memset(fifoname, 0, FIFO_NAME_LEN);
        sprintf(fifoname, "%s_%d", FIFO_SR_NAME, thread_num);

        /* fifo send data to B board */
        unlink(fifoname);
        ret = mkfifo(fifoname, 0666);
        if (ret == -1) {
                perror("mkfifo failed\n");
                pthread_exit(0);
        }

        fd_socket = open(fifoname, O_WRONLY);
        if (fd_socket == -1) {
                perror("open");
                unlink(fifoname);
                pthread_exit(0);
        }

        while (1) {
again:
                if (!sktnnp) {
                        sktnnp = get_from_sktlist(thread_num);
                        f_h = malloc(sizeof(f_h)+strlen(sktnnp)+1);
                        if (f_h == NULL) {
                                unlink(fifoname);
                                perror("alloc f_h failed\n");
                                break;
                        }
                        memcpy(f_h->name, sktnnp, strlen(sktnnp)+1);

                        buf = malloc(ARRAY_SIZE_MAX);
                        if (!buf) {
                                perror("malloc\n");
                                unlink(sktnnp);
                                free(f_h);
                                break;
                        }
                }

	        ret = skt_recv(sktnnp, buf, BHS_SIZE);
                if (ret != BHS_SIZE) {
                        printf("ERR skt_recv not enougth(%d)\n", ret);
                        if (ret == 0) {
                                /* rebuild socket */
                                sktnnp = NULL;
                                goto again;
                                printf("Disconnect the socket\n");
                        }
                }
                printf("================recv data from initor(%s)================\n",
                                sktnnp);
                show_time();

                bhs = (struct iscsi_hdr *)buf;

                printf("Dump BHS data:\n");
                dumpdata((char *)bhs, BHS_SIZE);
                len = BHS_SIZE;

                switch (bhs->opcode & ISCSI_OPCODE_MASK) {
                case ISCSI_OP_NOP_OUT:
                        printf("ISCSI_OP_NOP_OUT\n");
                        break;

                case ISCSI_OP_SCSI_CMD:
                        printf("ISCSI_OP_SCSI_CMD\n");
                        cmd_exec_scsi(bhs, sktnnp, buf, &len, &dir);
                        /* if dir is WRITE data need encrypt */
                        if (dir == SCST_DATA_WRITE)
                                f_h->encrypt = 1;
                        break;

                case ISCSI_OP_SCSI_TASK_MGT_MSG:
                        printf("ISCSI_OP_SCSI_TASK_MGT_MSG\n");
                        break;

                case ISCSI_OP_LOGIN_CMD:
                        printf("ISCSI_OP_LOGIN_CMD\n");
                        cmd_exec_login(bhs, sktnnp, buf, &len);
                        break;

                case ISCSI_OP_TEXT_CMD:
                        printf("ISCSI_OP_TEXT_CMD\n");
                        cmd_exec_text(bhs, sktnnp, buf, &len);
                        break;

                case ISCSI_OP_SCSI_DATA:
                        printf("ISCSI_OP_SCSI_DATA\n");
                        cmd_exec_data_out(bhs, sktnnp, buf, &len);
                        break;

                case ISCSI_OP_LOGOUT_CMD:
                        printf("ISCSI_OP_LOGOUT_CMD\n");
                        break;

                case ISCSI_OP_SNACK_CMD:
                        printf("ISCSI_OP_SNACK_CMD\n");
                        break;

                default:
                        printf("opcode is err\n");
                }

                dumpdata(buf, len);
                /* send data to B board */
                f_h->hdr_size = len;
                printf("\n================send %d data to B board================\n",
                                len);
                write(fd_socket, f_h, sizeof(f_h)+SKT_NAME_LEN);
                write(fd_socket, buf, f_h->hdr_size);
                memset(buf, 0, ARRAY_SIZE_MAX);
        }

        unlink(fifoname);
        free(f_h);
        free(buf);
        pthread_cleanup_pop(0);
        pthread_exit(0);
}

int recv_thread_create(void)
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
                                        skt_recv_fn_ser, (void*)(&pskt->thread_num[i]));
                }
        }
        pthread_attr_destroy(&pskt->pattr);
        return 0;
}

int send_thread_create(void)
{
        int i, ret;
        struct pthread_skt *pskt = NULL;

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
                                        skt_send_fn_ser, (void*)(&pskt->thread_num[i]));
                }
        }
        pthread_attr_destroy(&pskt->pattr);
        return 0;
}

int main(int argc, char **argv)
{
        void *sktattrp = NULL;
        char *sktlistnodep = NULL;
        int ret;
        int num = 0;
        char sktnnp[10];

        sktlist_init();
        skt_sys_init("ISCSI_SERVER");

        recv_thread_create();
        send_thread_create();

        sktattrp = skt_alloc_attr(SOCK_STREAM, SKT_SERVER_PORT);
        if (!sktattrp) {
                printf("SKT ALLOC ATTR ERROR, PORT:%d\n",
                                SKT_SERVER_PORT);
                return -1;
        }

        ret = skt_attr_register(sktattrp, SKT_ATTR);
        if (ret) {
                printf("SKT ATTR REGISTER ERROR\n");
                return -1;
        }

        while (1) {
                memset(sktnnp, 0, 10);
                sprintf(sktnnp, "%s_%d", SKT_NODE_NAME, num++);
                ret = skt_accept(SKT_ATTR, sktnnp);
                if (ret) {
                        continue;
                }	
                sktlistnodep = alloc_sktnode(sktnnp);
                send_to_sktlist(sktlistnodep);
                print_attr();
                print_node();
        }

        return 0;
}
