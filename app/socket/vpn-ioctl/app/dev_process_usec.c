#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "dev_ioctrol.h"
#include "cmd.h"

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

int dev_init(const char *ioa, const char *iob)
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
	
	data = PORTLED1;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTLED2;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTLED3;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	return 0;
}

int led_on(int num)
{
	int led[] = {PORTLED1, PORTLED2, PORTLED3,0};

	if (num<0) return -1;
	if (num>3) return -2;

	ioctl(iohandle.fda, GPIO_IOC_CLEAR, &led[num]);
	return 0;
}
int led_off(int num)
{
	int led[] = {PORTLED1, PORTLED2, PORTLED3,0};

	if (num<0) return -1;
	if (num>4) return -2;

	ioctl(iohandle.fda, GPIO_IOC_SET, &led[num]);
	return 0;	
}


