#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>
#include <asm/types.h>
#include <mtd/mtd-user.h>
#include "dev_ioctrol.h"
#include "cmd.h"
#include "client.h"

typedef void (*sighandler_t) (int);
#define NANDFLASH_MTD3_ADDR  0
#define NANDFLASH_MTD3_MAXLEN  0X1320000

struct nandflash_7825{
    int fd;
    int dlen;
}nandhandle = {-1,0};

struct mtd_info_user meminfo;
struct mtd_ecc_stats oldstats, newstats;
unsigned char *wbuf = NULL,*rbuf = NULL;

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
	
	data = PORTLED2;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);

	data = PORTLED3;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);

	data = PORTLED4;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);

	data = PORTLED5;
	ioctl(iohandle.fdb, GPIO_ATTR_OUT, &data);
	
	data = PORTBEEP;
	ioctl(iohandle.fda, GPIO_ATTR_OUT, &data);
	data = PORTBEEP;
	ioctl(iohandle.fda,GPIO_IOC_SET , &data);

	return 0;
}


int led_on(int num)
{
	int fd = -1;
	int led[] = {0,0, PORTLED2, PORTLED3, PORTLED4, PORTLED5,0};

	if (num<2) return -1;
	if (num>6) return -2;
	if (num==3) {
		fd = iohandle.fda;
	} else {
		fd = iohandle.fdb;
	}

	ioctl(fd, GPIO_IOC_CLEAR, &led[num]);
	return 0;
}
int led_off(int num)
{
	int fd = -1;
	int led[] = {0,0, PORTLED2, PORTLED3, PORTLED4, PORTLED5,0};

	if (num<2) return -1;
	if (num>6) return -2;
	if (num==3) {
		fd = iohandle.fda;
	} else {
		fd = iohandle.fdb;
	}

	ioctl(fd, GPIO_IOC_SET, &led[num]);
	return 0;	
}


int auto_beep(char ttimes, char tdelay, char bt)
{
	unsigned char data;
	unsigned int i,j;
	
	for(i = 0; i < ttimes; i++) {
		for(j = 0;j < 8; j++){
			data = PORTBEEP;
			if((bt>>j) & 0x1){
				ioctl(iohandle.fda,GPIO_IOC_CLEAR, &data);
			}else{
				ioctl(iohandle.fda,GPIO_IOC_SET , &data);
			}
			usleep((tdelay*1000));
		}
	}
	return 0;
}
void manu_beep(int bp)
{
	int data = PORTBEEP;
	if(bp==1){//为1响
		ioctl(iohandle.fda,GPIO_IOC_CLEAR, &data);
	}else{
		ioctl(iohandle.fda,GPIO_IOC_SET , &data);
	}
	return ;	
}
int get_random_from_wng8(char ch, char *buf, int len)
{
	int ret = -1;
	buf[0] = ch;
	ret = read(iohandle.fda,buf, len);
	if (ret < 0) {
		perror("Read random");
		return ret;
	}	
	return 0;
}


int mynand_init()
{
    nandhandle.fd = open("/dev/mtd3", O_RDWR);
    if (nandhandle.fd < 0) {
        perror("can not open");
        return -1;
    }

    if (ioctl(nandhandle.fd, MEMGETINFO, &meminfo)) {//获取meminfo信息
        perror("MEMGETINFO");
        close(nandhandle.fd);
        return -1;
    }

    nandhandle.dlen = meminfo.erasesize;//定义一个与一个块长度相同的变量用来与存入的数据长度进行对比

    wbuf = malloc(meminfo.erasesize);
    if (!wbuf) {
        fprintf(stderr, "Could not allocate %d bytes for writebuffer\n",\
            meminfo.erasesize);
        return -1;
    }
    memset(wbuf,0,nandhandle.dlen);

    rbuf = malloc(meminfo.erasesize);
    if (!rbuf) {
        fprintf(stderr, "Could not allocate %d bytes for readbuffer\n",\
            meminfo.erasesize);
        return -1;
    }
    memset(rbuf,0,nandhandle.dlen);

    if (ioctl(nandhandle.fd, ECCGETSTATS, &oldstats)) {
        perror("ECCGETSTATS");
        close(nandhandle.fd);
        return -1;
    }

    printf("ECC corrections: %d\n", oldstats.corrected);
    printf("ECC failures   : %d\n", oldstats.failed);
    printf("Bad blocks     : %d\n", oldstats.badblocks);
    printf("BBT blocks     : %d\n", oldstats.bbtblocks);

    return 0;
}

int mynand_erase()
{
    struct erase_info_user er;
    loff_t i;
    er.start = NANDFLASH_MTD3_ADDR;
    er.length = nandhandle.dlen;

    printf("start erase----------\n");
    for (i = er.start; i < NANDFLASH_MTD3_MAXLEN; i += nandhandle.dlen){
        if (ioctl(nandhandle.fd, MEMGETBADBLOCK, &i)) {
            printf("\rBad block at 0x%08x\n", (unsigned)i);
            continue;
        }
        er.start = i;
        er.length = nandhandle.dlen;

        if (ioctl(nandhandle.fd, MEMERASE, &er)) {
            perror("MEMERASE ERROR");
            return -1;
        }
		
    }
    printf("erase all ok!\n");
    return 0;
}

int mynand_read(void)
{
	int i,m,numblock;
	ssize_t read_len;
	unsigned long address;
	struct erase_info_user er;
	loff_t j;
	char tmpb[100];
	char buf[1025];
	char *a="channel_0";
	
	dp_client_sysinit(1);//创建一个数据打桩通道
        memset(tmpb, 0, 100);
        sprintf(tmpb, "%s\n", a);	
	
	er.start = NANDFLASH_MTD3_ADDR;
	numblock = NANDFLASH_MTD3_MAXLEN/(nandhandle.dlen);
	for(i=0;i<numblock;i++){
		j = er.start + i*nandhandle.dlen;
	        if (ioctl(nandhandle.fd, MEMGETBADBLOCK, &j)) {
	            printf("\rBad block at 0x%08x\n", (unsigned)j);
	            continue;
	        }
		read_len = pread(nandhandle.fd,rbuf,nandhandle.dlen,j);
		if (read_len < 0) {
		    address = NANDFLASH_MTD3_ADDR+i*nandhandle.dlen;
		    printf("Mark block bad at %08lx\n", (long)NANDFLASH_MTD3_ADDR+i*nandhandle.dlen);
		    ioctl(nandhandle.fd, MEMSETBADBLOCK, &address);
		    continue;
		}
		if (read_len < nandhandle.dlen) {
		    printf("\n");
		    fprintf(stderr, "Short write (%d bytes)\n", read_len);
		    return -1;
		}	
		dpappdump(0,tmpb,rbuf,nandhandle.dlen);
	}
	printf("mynand_read ok\n");
	dp_client_sysclose();
	return 0;
}

int mynand_write(int len,char *buf)
{
    int i,j,numblock,bufnum,cnt=0;
    ssize_t write_len;
    unsigned long addres;
    numblock = NANDFLASH_MTD3_MAXLEN/(nandhandle.dlen);
    bufnum = (nandhandle.dlen)/len;
    for(i=0;i<numblock;i++){
        for(j=0;j<bufnum;j++){
            memcpy(&wbuf[j*len],(void*)buf,len);
        }
        write_len = pwrite(nandhandle.fd,wbuf,nandhandle.dlen,NANDFLASH_MTD3_ADDR+i*nandhandle.dlen);
        if (write_len < 0) {
            addres = NANDFLASH_MTD3_ADDR+i*nandhandle.dlen;
            printf("Mark block bad at %08lx\n", (long)NANDFLASH_MTD3_ADDR+i*nandhandle.dlen);
            ioctl(nandhandle.fd, MEMSETBADBLOCK, &addres);
            continue;
        }
        if (write_len < nandhandle.dlen) {
            printf("\n");
            fprintf(stderr, "Short write (%d bytes)\n", write_len);
            return -1;
        }
       // printf("write_cnt=%d\t",++cnt);
    }
    printf("nand_write ok\n");
    return 0;
}
int space_free()
{
    if (nandhandle.fd>0) {
        close(nandhandle.fd);
        nandhandle.fd = -1;
    }
    nandhandle.dlen = 0;
    free(rbuf);
    free(wbuf);
    return 0;
}
int static inline pox_system(const char * cmdstring)
{
    pid_t pid;
    int status;
    if(cmdstring == NULL){
        return(-3);//如果cmdstring为空，返回非零值
    }
    if((pid = fork())<0){
        status=-1;//fork失败，返回-1
    }else if(pid == 0){
        execl("/bin/sh","sh","-c",cmdstring,(char*)0);
        _exit(127);//execl失败返回127，注意execl只在失败后才返回现在的进程，成功的话现在的进程就不存在了
    }else{//父进程
        while(waitpid(pid,&status,0)<0){
            if(errno!=EINTR){
                status=-2;//如果waitpid被信号中断，则返回-2；
                break;
            }
        }
    }
    return status;
}

int my_system(const char*cmd_line)
{
    int ret=0;
    sighandler_t old_handler;
    old_handler=signal(SIGCHLD,SIG_DFL);
    ret=pox_system(cmd_line);
    signal(SIGCHLD,old_handler);
    return ret;
}
int str_process(const char *s)
{
    int status;
    status = my_system(s);
    //free(s);
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status))
            return 1;
        else
            return 0;
    }else{
        return status;
    }
}
int myrebuild(void)
{
	int ret;
	const char *ss= "/opt/nas7825/script/rebuild.sh";
	ret = str_process(ss);
	if(ret<0){
		return -1;
	}
	sleep(2);
	return 0;
}
int myflash_erase(void)
{
	int i,ret;
	const char *ss[5]={"umount /mnt/ubidata","ubirmvol /dev/ubi1 -n 0",\
				    "ubidetach /dev/ubi_ctrl -d 1","rm /dev/ubi1*","rm /mnt/ubidata/* -rf"};
	for(i=0;i<5;i++){
		ret = str_process(ss[i]);
		if(ret<0){
			return -1;
		}
		usleep(5000);
		printf("i=%d\n",i);
	}
	return 0;
}
int mywrite_block(int len,char * buf)
{
	int ret;
	ret = mynand_init();
	if(ret<0){
		perror("mynand_init");
		return -1;
	}
	ret = mynand_erase();
	if(ret<0){
		perror("mynand_erase");
		return -1;
	}
	ret = mynand_write(len,buf);
	if(ret<0){
		perror("mynand_write");
		return -1;
	}
	
	space_free();
	return 0;
}
int myread_block(void)
{
	int ret;
	ret = mynand_init();
	if(ret<0){
		perror("mynand_init");
		return -1;
	}
	ret = mynand_read();
	if(ret<0){
		perror("mynand_read");
		return -1;
	}
	
	space_free();	
	return 0;
}

