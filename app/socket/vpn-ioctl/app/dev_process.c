#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "dev_process.h"
#include "cmd.h"

#ifdef WHITE
#undef SEC
#undef USEC
enum seirmode{
UARTMODE=0,
AC4MODE,
};
//typedef void (*sighandler_t)(int);
//sighandler_t signal(int signum, sighandler_t handler);
struct sysio{
	int fda;
	int fdb;
	int reduptype;
	int fd_null;
	int fdsys[3];
};

static struct sysio  iohandle;
int dev_init(char *ioa, char *iob)
{
	int data;
	iohandle.fda= open(ioa, O_RDWR);
	if (!iohandle.fda) {
		perror("OPEN IOA");
		return -1;
	}

	iohandle.fdb= open(iob, O_RDWR);
	if (!iohandle.fdb) {
		perror("OPEN IOB");
		return -1;
	}
#ifdef WHITE
	//setup  led
	data = PORTLED2;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTLED3;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);

	data = PORTLED4;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);

	data = PORTLED5;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);
#endif



#ifdef SEC || NSEC
	data = PORTLED1;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);
	
	data = PORTLED2;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTLED3;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTBEEP;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);
#endif

	return 0;
}



#ifdef WHITE	
void led_option(int num, int cmd)
{
	//int data;
	int led[] = {PORTLED1,PORTLED2, PORTLED3, PORTLED4, PORTLED5,PORTLED6};
	switch(cmd) {
	case 1:
		if (num==0 || num==1 || num==4){
				ioctl(iohandle.fda, GPIO_IOC_SET, &led[num]);
				printf("WHITE led set num = %d\n",num);
			}
		else {
				ioctl(iohandle.fdb, GPIO_IOC_SET, &led[num]);
				printf("WHITE led set num = %d\n",num);
			}
		break;
	case 0:
		if (num==0 || num==1 || num==4){
			ioctl(iohandle.fda, GPIO_IOC_CLEAR, &led[num]);
			printf("WHITE led clear num = %d\n",num);}
		else {
			ioctl(iohandle.fdb, GPIO_IOC_CLEAR, &led[num]);
			printf("WHITE led clear num = %d\n",num);}
		break;
	default:break;
	}
	return ;
}
#endif
#ifdef SEC
void led_option(int num, int cmd)
{
	int led_sec[] = {PORTLED1,PORTLED2, PORTLED3};
	switch(cmd) {
	case 1:
		ioctl(iohandle.fda, GPIO_IOC_SET, &led_sec[num]);
		printf("sec led set\n");
		break;
	case 0:
		ioctl(iohandle.fda, GPIO_IOC_CLEAR, &led_sec[num]);
		printf("sec led clear\n");
		break;
	default:break;
	}
	return; 
}
#endif
#ifdef NSEC 
void led_option(int num, int cmd)
{
	int led_nsec[] = {PORTLED1,PORTLED2, PORTLED3};
	switch(cmd) {
	case 1:
		ioctl(iohandle.fda, GPIO_IOC_SET, &led_nsec[num]);
		printf("nosec led set\n");
		break;
	case 0:
		ioctl(iohandle.fda, GPIO_IOC_CLEAR, &led_nsec[num]);
		printf("nosec led clear\n");
		break;
	default:break;
	}
	return ;
}
#endif

void led_on(void *cmdp)
{
	struct gpiocmd *pledpro =  (struct gpiocmd *)cmdp;	
	char tmp_data = pledpro->data[0];
#ifdef	WHITE	
		led_option(((int)tmp_data-1), 0);
#endif
#ifdef	SEC 
		led_option(((int)tmp_data-1), 0);
#endif
#ifdef	NSEC 
		led_option(((int)tmp_data-1), 0);
#endif
}
void led_off(void *cmdp)
{
	struct gpiocmd *pledpro =  (struct gpiocmd *)cmdp;
	char tmp_data = pledpro->data[0];
#ifdef	WHITE	
		led_option(((int)tmp_data-1), 1);
#endif
#ifdef	SEC 
		led_option(((int)tmp_data-1), 1);
#endif
#ifdef	NSEC 
		led_option(((int)tmp_data-1), 1);
#endif	
}
void led_process(void *cmdp)
{
	struct gpiocmd *pledpro = (struct gpiocmd *)cmdp;
	char tmp_data = pledpro->data[1];
	if(tmp_data){
		led_on(pledpro);
	}else{
		led_off(pledpro);
	}
}

void auto_beep(void *cmdp)
{
	unsigned char beep_data,data;
	unsigned int i,j;
	data = PORTBEEP;
	char tmp_data1;
	char tmp_data2;
	char tmp_data3;
	int num = 1;
	int bignum = 1;
	int timedata;
	struct gpiocmd *pbeepctl = (struct gpiocmd *)cmdp;
	tmp_data1 = pbeepctl->data[1];
	tmp_data2 = pbeepctl->data[2];
	tmp_data3 = pbeepctl->data[3];

	timedata = tmp_data2*100;
	
	printf("the vale of data[1] is %d\n",tmp_data1);
	printf("the vale of data[2] is %d\n",tmp_data2);
	printf("the vale of data[3] is %d\n",tmp_data3);
	
	for(i=0;i<(tmp_data1);i++){
		for(j=8;j>0;--j){
			beep_data = (tmp_data3>>(j-1))&&0x01;
			if(beep_data){
				ioctl(iohandle.fdb,GPIO_IOC_CLEAR, &data);
				usleep((timedata*1000));		
				printf("beep on %d\n",num);
			}else{
				ioctl(iohandle.fdb,GPIO_IOC_SET , &data);
				usleep((timedata*1000));
				printf("beep off %d\n",num);
			}
			num++;
		}
		bignum++;
		printf("xun huan ci shu %d\n",bignum);
	}
}
void nauto_beep(void *cmdp)
{
	char tmp_data;
	struct gpiocmd *pbeepctl = (struct gpiocmd *)cmdp;
	tmp_data = pbeepctl->data[1];
	//关蜂鸣器
	int data = PORTBEEP;
	if(1==tmp_data){//为1响
		ioctl(iohandle.fdb,GPIO_IOC_CLEAR, &data);
		printf("on beep\n");	
	}else if(tmp_data==2){
		ioctl(iohandle.fdb,GPIO_IOC_SET , &data);
		printf("close beep\n");
	}
	
}
void beep_process(void *cmdp)
{
	int i;
	struct gpiocmd *pbeepctl = (struct gpiocmd *)cmdp;
	char tmp_data = pbeepctl->data[0];
	if(tmp_data==1){//自动控制
		auto_beep(pbeepctl);
		printf("run auto beep\n");
		printf("run auto beep\n");
	}else if(tmp_data==2){//被动控制
		nauto_beep(pbeepctl);
		printf("run nauto beep\n");
		printf("run nauto beep\n");
	}
}

void uart_process(void *cmdp)
{
	int i;
	struct gpiocmd *puartctl = (struct gpiocmd *)cmdp;
	char tmp_data = puartctl->data[0];
	if(1==tmp_data){//console
		printf("set console\n");
		int data = PORTUARTMODE;
		ioctl(iohandle.fda, GPIO_IOC_SET, &data);
		if (iohandle.reduptype == AC4MODE) {
			for (i = 0; i < 3;  i++) {
				dup2(iohandle.fdsys[i], i);
				close(iohandle.fdsys[i]);
			}
			iohandle.reduptype = UARTMODE;	
		}
	}else if(2==tmp_data){//ac4
		printf("set ac4\n");
		int data = PORTUARTMODE;
		ioctl(iohandle.fda, GPIO_IOC_CLEAR, &data);
		if (iohandle.reduptype == UARTMODE) {
			for (i = 0; i < 3; i++) {
				iohandle.fdsys[i] = dup(i);
				dup2(iohandle.fd_null, i);
			}
			iohandle.reduptype=AC4MODE;	
		}
	}
}
void random_process(void *cmdp)
{
	int getdata;
	int ret;
	struct gpiocmd *prandompro = (struct gpiocmd *)cmdp;
	char ch_num = prandompro->data[0];
	char len = prandompro->data[1];
	
	getdata = ch_num;
	printf("getdata is %d\n",getdata);
	ret = read(iohandle.fda,&getdata,len);
	if (ret < len) {
		if(ret){
			fprintf(stderr, "Short read (%d bytes)\n", ret);
		}else
			perror("read");
	}
	printf("getdata is %d\n",getdata);	
}
