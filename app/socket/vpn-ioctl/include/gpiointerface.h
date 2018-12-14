/*************************************************************************
	> File Name: include/gpiointerface.h
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Thu 29 May 2014 03:55:26 PM CST
 ************************************************************************/
#ifndef __GPIOINTERFACE__H__
#define __GPIOINTERFACE__H__

#ifdef __cplusplus
extern  "C" {
#endif

int ac4_ctr_reset(int num);
int led_ctr_on(int num);
int led_ctr_off(int num);
int beep_ctr_on(void);
int beep_ctr_off(void);
int beep_ctr_auto(char ttimes, char tdelay, char bt);
int uart_mode_console(void);
int uart_mode_ac4(void);
int get_random(int ch, char *buf, int len);
int open_elock(void);
int rebuild_mtd3(void);
int erase_mtd3(void);
int write_mtd3(int len,char *buf);
int read_mtd3(void);
int a5_a11_set(char data);
int a5_a11_clear(char data);
int a5_a11_getstatus(void);

#ifdef __cplusplus
}
#endif

#endif
