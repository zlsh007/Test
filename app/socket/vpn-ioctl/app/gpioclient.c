/*************************************************************************
	> File Name: app/gpioclient.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Thu 29 May 2014 02:44:00 PM CST
 ************************************************************************/
#include <stdio.h>
#include "cmd.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "gpiointerface.h"
#include "dev_ioctrol.h"
#include "socketlib.h"
#include "hear.h"

#define GPIO_IOC_MAGIC  'i'
#define GPIO_IOC_GET					_IOWR(GPIO_IOC_MAGIC,3, int)
static int killfd = -1;

#define VPN1000SECKILLKEY 12
#define WHITEBOXKILLKEY 17
void kill_handler(int io)
{
//#define PORTKILLKEY  WHITEBOXKILLKEY
#define PORTKILLKEY  VPN1000SECKILLKEY
	int data[2], i, j;		
	for (i = 0; i < 10000;i++) {
		for (j=0;j<20000;j++);;
		data[0] = PORTKILLKEY;
		data[1] = 0;
		ioctl(killfd, GPIO_IOC_GET, data);
		HEAR("DATA : %d\n",data[0]);
		if (data[0]) return ;
	}
	printf("start kill all data...,%d\n",PORTKILLKEY);
	
	printf("kill all data end!\n");
	return ;
}

void kill_key_init(void)
{
//#define PORTKILLKEY  WHITEBOXKILLKEY
#define PORTKILLKEY  VPN1000SECKILLKEY
	int data,ret;
	killfd = open("/dev/gpiob", O_RDWR);
	if (!killfd) {
		perror("Open gpiob:");
		return ;
	}
	HEAR("WHITE\n");
	data = PORTKILLKEY;
	ret = ioctl(killfd,GPIO_ATTR_INTERRUPT,data);
	HEAR("ret = %d\n",ret);
	//setup kill handler for vpn
	signal(SIGIO, &kill_handler);
	fcntl(killfd, F_SETOWN, getpid());
	fcntl(killfd, F_SETFL,fcntl(killfd, F_GETFL)|FASYNC);
	return ;
}	

int main(int argc,  char **argv)
{
	int i,j,ret,cnt;
	char buf[10];
	char *data=NULL;
	const char *hehe = {
			"Usage:\n"
			"XXX  [0~13]\n"
			"0:open_elock		1:open_led[led_num]	2:close_led[led_num]\n"
			"3:close_beep		4:open_beep		5:beep_auto[loop_times][delaytimes][rythem]\n"
			"6:uart	7:ac4	8:get_random[chennal_num]	9:kill_key\n"
			"10:rebuild	11:erase		12:write[0/1]		13:read\n"
			"14:a5_a11_set[0-a5/1-a11]	15:a5_a11_clear[0-a5/1-a11]\n"
			"16:a5_a11_getstatus\n"
	};
	if(argc<2){
		goto USAGE;
	}
	
	if(atoi(argv[1])==12){
		data = malloc(256);
		//srand((int)time(0));		
		if(atoi(argv[2])==0){
			memset(data,0,256);
		}else{
			memset(data,1,256);
		}
	}
	
	skt_sys_init("gpioclient");
	if(argc<2){
		while(1){
			for(j=0;j<50;j++){
				for(i=0;i<3;i++){
					led_ctr_on(i);
					usleep(100000);
					usleep(100000);
					led_ctr_off(i);
					usleep(100000);
					usleep(100000);
				}
			}

			for(i=0;i<2;i++){
				beep_ctr_on();
				sleep(1);
				beep_ctr_off();
				sleep(1);
			}	
			beep_ctr_auto(1,100,84);
		}

	}

	switch(atoi(argv[1])){
		case 0:{
			open_elock();
			break;
		}
		case 1:{
			led_ctr_on(atoi(argv[2]));
			break;
		}
                case 2:{
			led_ctr_off(atoi(argv[2]));
                        break;
                }
                case 3:{
			beep_ctr_off();
                        break;
                }
                case 4:{
                        beep_ctr_on();
                        break;
                }
                case 5:{
			  beep_ctr_auto(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
                        break;
                }
                case 6:{
			printf("set uart mode\n");
			uart_mode_console();	
			break;
                }
                case 7:{
			printf("set ac4 mode\n");
			uart_mode_ac4();
                        break;
                }
                case 8:{
			get_random(atoi(argv[2]), buf, 10);
/*
			for (i = 0; i < 10; i ++) {
				printf("%x  ", buf[i]);
			}
*/
                        break;
                }
		case 9:{
			kill_key_init();
			while(1);
			break;
		}
		case 10:{
			if(rebuild_mtd3()){
				HEAR("rebuild_mtd3 err\n");
			}else
			HEAR("rebuild_mtd3 ok\n");
			break;
		}
		case 11:{
			if(erase_mtd3()){
				HEAR("erase_mtd3 err\n");
			}else
			HEAR("erase_mtd3 ok\n");
			break;
		}
		case 12:{
			if(write_mtd3(256,data)){
				HEAR("write_mtd3 err\n");
			}else
			HEAR("write_mtd3 ok\n");
			free(data);
			break;
		}
		case 13:{
			while(1){
				cnt++;
				if(read_mtd3()){
					HEAR("read_mtd3 err\n");
				}else
				HEAR("read_mtd3 ok\n");
				HEAR("******cnt = %d*******\n",cnt);
			}
			break;
		}		
		case 14:{
			if(a5_a11_set(atoi(argv[2]))){
				HEAR("a5_a11_set err\n");
			}else
			HEAR("a5_a11_set is ok\n");
			break;
		}
		case 15:{
			if(a5_a11_clear(atoi(argv[2]))){
				HEAR("a5_a11_clear err\n");
			}else
			HEAR("a5_a11_clear is ok\n");
			break;
		}
		case 16:{
			ret = a5_a11_getstatus();
			if(ret<0){
				HEAR("a5_a11_getstatus err\n");
			}else
			HEAR("a5_a11_getstatus is %d\n",ret);
			break;
		}		
		default:
			printf("cmd error\n");
			break;
		
	}
	return 0;
USAGE:
	HEAR("\n");
	puts(hehe);
	return 0;
}




