/*************************************************************************
	> File Name: cmd.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Wed 05 Mar 2014 05:01:38 PM CST
 ************************************************************************/
#ifndef __CMD__H
#define __CMD__H


#define CMD_START  0X7e
#define CMD_LED	1
#define CMD_BEEP 2
#define CMD_UARTMODE 3
#define CMD_RANDOM  4
#define CMD_OPENLOCK  5
#define CMD_AC4RESET  6
#define CMD_MTD3_REBUILD  7
#define CMD_MTD3_ERASE  8
#define CMD_MTD3_WRITE  9
#define CMD_MTD3_READ  10
#define CMD_A5_A11_SET  11
#define CMD_A5_A11_CLE  12
#define CMD_A5_A11_GET  13

#define CMD_MAX  6

#define CMD_ACK_SIGN  0x80


#define SUBCMD_LED_ON  1
#define SUBCMD_LED_OFF 0
#define SUBCMD_BEEP_MOD_AUTO  1
#define SUBCMD_BEEP_MOD_MANU  2
#define SUBCMD_UART_MOD_CON  1
#define SUBCMD_UART_MOD_AC4  2
#define SUBCMD_BEEP_OFF  2
#define SUBCMD_BEEP_ON  1

struct gpiocmd{
	char start;
	char cmd;
	char subcmd;
	char crc;
	int len;
	char data[0];
};


int cmd_send(const char *nn, char cmd, char *data, int len);
struct gpiocmd* cmd_recv(const char *nn);



#endif

