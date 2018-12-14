#ifndef __DEV_PROCESS_H__
#define __DEV_PROCESS_H__

int dev_init(const char *ioa, const char *iob);
int led_on(int num);
int led_off(int num);
#ifdef WHITE
int auto_beep(char ttimes, char tdelay, char bt);
void manu_beep(int bp);
int get_random_from_wng8(char ch, char *buf, int len);
int myrebuild(void);
int myflash_erase(void);
int mywrite_block(int len, char *buf);
#endif

#ifdef SEC	
int auto_beep(char ttimes, char tdelay, char bt);
void manu_beep(int bp);
int get_random_from_wng8(char ch, char *buf, int len);
void set_uart(void);
void set_ac4(void);
void lock_open(void);
int myrebuild(void);
int myflash_erase(void);
int mywrite_block(int len, char *buf);
int my_a5_a11_set(char data);
int my_a5_a11_clear(char data);
int my_a5_a11_getstatus(void);
#endif

#endif
