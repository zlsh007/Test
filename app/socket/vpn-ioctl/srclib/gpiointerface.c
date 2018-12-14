/*************************************************************************
	> File Name: srclib/gpiointerface.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Thu 29 May 2014 03:25:20 PM CST
 ************************************************************************/
#include <stdio.h>
#include "socketlib.h"
#include "cmd.h"
#include "getenv.h"
#include "file_io.h"
#include "sktlist.h"
#include "log.h"
#include "bdversion.h"
#include "hear.h"

#define SKTGPIOSERVERIP  "127.0.0.1"
#define SKTGPIOSERVERPORT  8888

static int gpio_send_cmd(char cmd, char *data, int len)
{
	void *nodp = NULL;
	char *name = "123456";
	
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, cmd, data, len);	
	skt_close_free_node(name);
	return 0;
}

int led_ctr_on(int num)
{
	int i;
	char data[2] = {0};
	data[0] = num&0xff;
	data[1] = SUBCMD_LED_ON;
	return gpio_send_cmd(CMD_LED, data, 2);
}
int ac4_ctr_reset(int num)
{
	return gpio_send_cmd(CMD_AC4RESET, NULL, 0);
}

int led_ctr_off(int num)
{
	char data[2] = {0};
	data[0] = num&0xff;
	data[1] = SUBCMD_LED_OFF;
	return gpio_send_cmd(CMD_LED, data, 2);
}

int beep_ctr_on(void)
{
	char data[2] = {0};
	data[0] = SUBCMD_BEEP_MOD_MANU;
	data[1] = SUBCMD_BEEP_ON;
	return gpio_send_cmd(CMD_BEEP, data, 2);
}

int beep_ctr_off(void)
{
	char data[2] = {0};
	data[0] = SUBCMD_BEEP_MOD_MANU;
	data[1] = SUBCMD_BEEP_OFF;
	return gpio_send_cmd(CMD_BEEP, data, 2);
}

int beep_ctr_auto(char ttimes, char tdelay, char bt)
{
	char data[4] = {0};
	data[0] = SUBCMD_BEEP_MOD_AUTO;
	data[1] = ttimes;
	data[2] = tdelay;
	data[3] = bt;
	return gpio_send_cmd(CMD_BEEP, data, 4);
}


int uart_mode_console(void)
{
	char data[1] = {0};
	data[0] = SUBCMD_UART_MOD_CON;
	return gpio_send_cmd(CMD_UARTMODE, data, 1);
}

int uart_mode_ac4(void)
{
	char data[1] = {0};
	data[0] = SUBCMD_UART_MOD_AC4;
	return gpio_send_cmd(CMD_UARTMODE, data, 1);
}

int open_elock(void)
{
	return gpio_send_cmd(CMD_OPENLOCK, NULL, 0);
}


int get_random(int ch, char *buf, int len)
{

	char data[5] = {0};
	int *p = NULL;
	struct gpiocmd *cmdp;
	void *nodp = NULL;
	char *name = "123456";
	
	data[0] = ch;
	p = (int *)&data[1];
	*p = len;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_RANDOM, data, 5);	
	
	cmdp = cmd_recv(name);	
	
	skt_close_free_node(name);
	memcpy(buf, cmdp->data, cmdp->len);	
	return cmdp->len;	
}


int rebuild_mtd3(void)
{
	struct gpiocmd *cmdp;
	char *name = "789123";
	void *nodp = NULL;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_MTD3_REBUILD, NULL, 0);		
	
	cmdp = cmd_recv(name);	
	
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
		
	skt_close_free_node(name);
	return 0;
}

int erase_mtd3(void)
{
	struct gpiocmd *cmdp;
	char *name = "456123";
	void *nodp = NULL;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_MTD3_ERASE, NULL, 0);		
	
	cmdp = cmd_recv(name);	
	
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
		
	skt_close_free_node(name);
	return 0;
	
}

int write_mtd3(int len,char *data)
{
	struct gpiocmd *cmdp;
	char *name = "123123";
	void *nodp = NULL;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_MTD3_WRITE,data,len);	
	cmdp = cmd_recv(name);	
	HEAR("write_mtd3 wait respond\n");
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
	skt_close_free_node(name);
	return 0;
}

int read_mtd3(void)
{
	struct gpiocmd *cmdp;
	char *name = "258123";
	void *nodp = NULL;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_MTD3_READ,NULL,0);		
	HEAR("read_mtd3 before cmd_recv\n");
	cmdp = cmd_recv(name);	
	HEAR("\n");
	HEAR("cmdp->data[0] = %d\n",cmdp->data[0]);
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
	skt_close_free_node(name);
	return 0;
}

int a5_a11_set(char data)
{
	struct gpiocmd *cmdp;
	char *name = "147123";
	void *nodp = NULL;
	char num[1] = {0};
	num[0] = data;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_A5_A11_SET,num,1);		
	
	cmdp = cmd_recv(name);	
	
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
	skt_close_free_node(name);	
	return 0;
}

int a5_a11_clear(char data)
{
	struct gpiocmd *cmdp;
	char *name = "368123";
	void *nodp = NULL;
	char num[1] = {0};
	num[0] = data;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_A5_A11_CLE,num,1);		
	
	cmdp = cmd_recv(name);	
	
	if((cmdp->data[0])<0){
		skt_close_free_node(name);
		return -1;
	}
	skt_close_free_node(name);	
	return 0;
}

int a5_a11_getstatus(void)
{
	struct gpiocmd *cmdp;
	char *name = "666777";
	void *nodp = NULL;
	char ret;
	nodp = skt_alloc_node(SOCK_STREAM, SKTGPIOSERVERIP, SKTGPIOSERVERPORT);
	if (!nodp) {
		printf("skt_alloc_node ret error\n");
		return -1;
	}
	skt_node_regester(nodp, name);
	cmd_send(name, CMD_A5_A11_GET,NULL,0);		
	HEAR("before cmd_recv\n");
	cmdp = cmd_recv(name);	
	ret = cmdp->data[0];
	HEAR("data is %d\n",ret);
	if(ret<0){
		skt_close_free_node(name);
		return -1;
	}else{
		skt_close_free_node(name);
		return ret;
	}
}