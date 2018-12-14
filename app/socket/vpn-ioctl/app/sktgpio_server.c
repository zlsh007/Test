/*************************************************************************
	> File Name: cli.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Sat 01 Mar 2014 10:21:39 AM CST
 ************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "socketlib.h"
#include "cmd.h"
#include "getenv.h"
#include "file_io.h"
#include "sktlist.h"
#include "log.h"
#include "bdversion.h"
#include "dev_process.h"
#include "hear.h"

#define SKT_ATTR  "SKTATTRSERVER"
#define SKT_NODE_SERVER  "SKTNODESERVER"
#define SKT_NODE_CLIENT  "SKTNODECLIENT"
#define SKT_CLIENT_MAX 10

struct pthread_sktgpio{
	pthread_attr_t pattr;
	pthread_t stid;
	pthread_t ctid[SKT_CLIENT_MAX];
	struct sktlist *slp;
}; 

static struct pthread_sktgpio  psktgpio;

void accept_create_cleanup(void *args)
{
	printf("pthread clean up:\n");
	printf("pthread name:accept_create_cleanup, ptid=%u\n",(unsigned int)pthread_self());
	return ;
}

int crc_check(struct gpiocmd *cmdp)
{
	int val;
	struct gpiocmd *pcrc = cmdp;
	val = (pcrc->cmd)^(pcrc->start)^(pcrc->subcmd);
	if(val==pcrc->crc){

	}else{

	}
	return 0;
}

static void led_process(char num, char data)
{
	(data>0)?led_on(num&0xff):led_off(num&0xff);
	return ;
}

#if defined(WHITE)  || defined(SEC)
void beep_process(char mod, char tms, char tdelay, char bt)
{
	printf("mod=%d, tms= %d, tdelay:%d, bt:%d\n", mod, tms, tdelay, bt);
	if (1==mod) {
		auto_beep(tms, tdelay, bt);
	} else if(2 == mod) {
		manu_beep(tms);
	}
	return ;	
}

#ifdef WHITE
char *random_process(char channel, int len)
{
	char *p = NULL;
	if (channel<0 ||channel >4) return NULL;	
	p = malloc(len+1);
	if (!p) {
		perror("random process");
		return NULL;
	}
	memset(p, 0, len);
	if( get_random_from_wng8(channel, p, len)) {
		printf("Get random error\n");
		free(p);
		return NULL;
	}
	return p;
}
#endif
#endif

void *skt_procces_fn(void *args)
{
	struct gpiocmd *cmdp = NULL;
	//struct sktlist *slistp = args;
	char *sktnnp = NULL;
#ifdef	WHITE
	char *dp = NULL;
#endif
	int ret;
	char data[1] = {0};
		
	pthread_cleanup_push(accept_create_cleanup, args);

	while (1) {
		if (!sktnnp) {
			sktnnp = get_from_sktlist();
		}
		HEAR("sktnnp is %s\n", sktnnp);
		cmdp = cmd_recv(sktnnp);	
		if (!cmdp) {
			//printf("NO DATA RECV\n");
			skt_close_free_node(sktnnp);	
			free(sktnnp);
			sktnnp=NULL;
			continue;
		}
		ret = crc_check(cmdp);
		if(-1==ret){
			printf("SEND TO CLIENT RELOAD.......\n");
			return NULL;
		}
		switch(cmdp->cmd){
			case CMD_LED:
				led_process(cmdp->data[0], cmdp->data[1]);
				printf("CMD LED.....,\n");
				break;
#if  defined(WHITE)  || defined(SEC)
			case CMD_BEEP:
				beep_process(cmdp->data[0], cmdp->data[1], cmdp->data[2], cmdp->data[3]);
				printf("CMD BEEP.....,\n");
				break;
			case CMD_MTD3_REBUILD:
				ret = myrebuild();
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_MTD3_REBUILD,data, 1);	
					break;
				}
				data[0] = 0;
				cmd_send(sktnnp, CMD_MTD3_REBUILD,data, 1);				
				printf("CMD MTD3_REBUILD.....,\n");
				break;
			case CMD_MTD3_ERASE:
				ret = myflash_erase();
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_MTD3_ERASE,data, 1);	
					break;
				}
				data[0] = 0;
				cmd_send(sktnnp, CMD_MTD3_ERASE,data, 1);				
				printf("CMD MTD3_ERASE.....,\n");
				break;
			case CMD_MTD3_WRITE:
				ret = mywrite_block(cmdp->len, cmdp->data);
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_MTD3_WRITE,data, 1);	
					break;
				}
				data[0] = 0;
				HEAR("mywrite_block 123 de sktnnp is %s\n",sktnnp);
				cmd_send(sktnnp, CMD_MTD3_WRITE,data, 1);				
				printf("CMD MTD3_WRITE.....,\n");
				break;
			case CMD_MTD3_READ:
				ret = myread_block();
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_MTD3_READ,data, 1);	
					break;
				}
				data[0] = 0;
				printf("mywrite_block 258 de sktnnp is %s\n",sktnnp);
				ret = cmd_send(sktnnp, CMD_MTD3_READ,data, 1);
				if(ret<0){
					HEAR("cmd_send err\n");
					perror("cmd_send");
				}
				printf("CMD MTD3_READ.....,\n");
				break;				
#endif				
#ifdef WHITE
			case CMD_RANDOM:
				dp = random_process(cmdp->data[0], (*(int *)(&cmdp->data[1])));
				if (!dp) {
					cmd_send(sktnnp, cmdp->cmd & 0x80,NULL, 0);	
					break;
				}
				cmd_send(sktnnp, cmdp->cmd, dp,*((int *) (&cmdp->data[1])));	
				printf("CMD RANDOM.....,\n");
				break;
#endif
#ifdef SEC
			case CMD_UARTMODE:
				if (1==cmdp->data[0]) {
					set_uart();
				} else if(2== cmdp->data[0]) {
					HEAR("set_ac4\n");
					set_ac4();
				} else {
					printf("error uart mode\n");	
				}
				break;
			case CMD_OPENLOCK:
				lock_open();		
				break;
			case CMD_AC4RESET:
				ac4_reset();		
				break;
			case CMD_A5_A11_SET:
				ret = my_a5_a11_set(cmdp->data[0]);
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_A5_A11_SET,data, 1);	
					break;
				}
				data[0] = 0;
				cmd_send(sktnnp, CMD_A5_A11_SET, data, 1);					
				printf("CMD CMD_A5_A11_SET.....,\n");
				break;	
			case CMD_A5_A11_CLE:
				ret = my_a5_a11_clear(cmdp->data[0]);
				if (ret) {
					data[0] = -1;
					cmd_send(sktnnp, CMD_A5_A11_CLE,data, 1);	
					break;
				}
				data[0] = 0;
				cmd_send(sktnnp, CMD_A5_A11_CLE, data, 1);					
				printf("CMD CMD_A5_A11_CLE.....,\n");
				break;
			case CMD_A5_A11_GET:
				ret = my_a5_a11_getstatus();
				if (ret<0) {
					data[0] = ret;
					cmd_send(sktnnp, CMD_A5_A11_GET,data, 1);	
					break;
				}
				data[0] = ret;
				HEAR("mywrite_block 666777 de sktnnp is %s\n",sktnnp);
				cmd_send(sktnnp, CMD_A5_A11_GET, data,1);					
				printf("CMD CMD_A5_A11_GET.....,\n");
				break;
#endif
			default:
				printf("ERROR CMD\n");
				break;
		}
	}
	pthread_cleanup_pop(0);
	pthread_exit(0);
}

extern struct sktlist slist;
int sktgpio_pthread_create(void)
{
	int ret;
	int i;
	struct pthread_sktgpio *siop = &psktgpio;
	
	memset(siop, 0, sizeof(struct pthread_sktgpio));
	siop->slp = &slist;
	memset(siop->slp, 0, sizeof(struct sktlist));
	siop->slp->list.prev = &siop->slp->list;
	siop->slp->list.next = &siop->slp->list;	
	pthread_cond_init(&siop->slp->cond, NULL);
	pthread_mutex_init(&siop->slp->mutex, NULL);
	
	ret = pthread_attr_init(&siop->pattr);
	if (ret!=0) {
		return ret;
	}

	ret = pthread_attr_setdetachstate(&siop->pattr, PTHREAD_CREATE_DETACHED);
	if(ret==0) {
		for (i = 0; i < SKT_CLIENT_MAX; i++) {	
			ret = pthread_create(&siop->ctid[i], &siop->pattr, skt_procces_fn, (void*)siop->slp);
		}
	}	
	pthread_attr_destroy(&siop->pattr);
	return 0;
}


int main(int argc, char **argv)
{
	void *sktattrp = NULL;
	char *sktlistnodep = NULL;
	int ret;
	int num = 0;
	char  sktnode[100];

	memset(sktnode, 0, 100);
	sprintf(sktnode, "SKT GPIO VERSION INFO FOR %s", DEVARCH);
	sys_version_log_mode_init(argv[0], sktnode);
	dp_env_init();	
	skt_sys_init("SKTGPIOSERVER");
	logsys_init();
	sktgpio_pthread_create();	
	dev_init("/dev/gpioa", "/dev/gpiob");
	printf("===================================start service=====================================\n");
#define SKT_SERVER_PORT  8888
	sktattrp = skt_alloc_attr(SOCK_STREAM, SKT_SERVER_PORT);
	if (!sktattrp) {
		printf("SKT GPIO SKT ALLOC ATTR ERROR, PORT:%d\n", SKT_SERVER_PORT);
		return -1;
	}
	ret = skt_attr_regester(sktattrp, SKT_ATTR);
	if (ret) {
		printf("SKT GPIO SKT ATTR REGISTER ERROR\n");
		return -1;
	}
	printf("SKT GPIO SKT ATTR REGISTER OK!\n");

	while (1) {
		memset(sktnode, 0, 100);
		sprintf(sktnode, "%s%d", SKT_NODE_CLIENT, num++);
		ret = skt_accept(SKT_ATTR, sktnode);
		//ret = skt_accept(SKT_ATTR, SKT_NODE_CLIENT);
		if (ret) {
			continue;
		}	
		//printf("skt accept ok...\n");
		sktlistnodep = alloc_sktnode(sktnode);
		send_to_sktlist(sktlistnodep);
		print_attr();
		print_node();
	}

	return 0;
}
