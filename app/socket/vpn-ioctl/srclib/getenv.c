/*************************************************************************
	> File Name: getenv.c
	> Author: guotao
	> Mail: guozengqiang163@163.com 
	> Created Time: Tue 04 Mar 2014 04:53:15 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//char *getenv(const char *name);

#define DEFAULT_DPS_INPUT_IP  "172.16.4.46"  //dp server ip addr
#define DEFAULT_DPS_INPUT_PORT  5000  //dp server ip addr
#define DEFAULT_DPS_OUTPUT_IP  "172.16.4.46"  //dp server ip addr
#define DEFAULT_DPS_OUTPUT_PORT  5050  //dp server ip addr
#define DEFAULT_DPM_INPUT_IP  "172.16.4.46"  //dp server ip addr
#define DEFAULT_DPM_INPUT_PORT  6000  //dp server ip addr
#define DEFAULT_DPM_OUTPUT_IP  "172.16.4.46"  //dp server ip addr
#define DEFAULT_DPM_OUTPUT_PORT  6050  //dp server ip addr
#define DEFAULT_DATA_DIR  "./data"
#define DEFAULT_LOGFILE  "./syslog/logfile"
#define DEFAULT_DPM_CHANNEL_MAX 16
#define DEFAULT_DPS_CHANNEL_MAX 4

enum envdp_ipport{
DPSINIP=0,
DPSOUTIP,
DPSINPORT,
DPSOUTPORT,
DPMINIP,
DPMOUTIP,
DPMINPORT,
DPMOUTPORT,
};
const char *envdpipport[8] = {"DPSINPUTIP", "DPSOUTPUTIP", "DPSINPUTPORT","DPSOUTPUTPORT","DPMINPUTIP", "DPMOUTPUTIP", "DPMINPUTPORT", "DPMOUTPUTPORT"};

const char *envdatadir = "DPDATADIR";
const char *envlogfile = "DPLOGFILE";
const char *dpmchannelmax="DPMMAXCHANNEL";
const char *dpschannelmax="DPSMAXCHANNEL";
struct dp_env{
	int dpsmax;
	char **dpsinip;
	int  *dpsinport;
	char **dpsoutip;
	int *dpsoutport;
	
	int dpmmax;
	char **dpminip;
	int *dpminport;
	char **dpmoutip;
	int *dpmoutport;

	char *datadir;
	char *logfile;
};

static struct dp_env  *envp;
void dump_env(void)
{
	int i;
	printf("DPSMAX=%d\n", envp->dpsmax);
	for (i = 0; i < envp->dpsmax; i++){
		printf("DPSINIP:%s\n", envp->dpsinip[i]);
		printf("DPSOUTIP:%s\n", envp->dpsoutip[i]);
		printf("DPSINPORT:%d\n", envp->dpsinport[i]);
		printf("DPSOUTPORT:%d\n", envp->dpsoutport[i]);
	}
	printf("DPMMAX=%d\n", envp->dpmmax);
	for (i = 0; i < envp->dpmmax; i++){
		printf("DPMINIP:%s\n", envp->dpminip[i]);
		printf("DPMOUTIP:%s\n", envp->dpmoutip[i]);
		printf("DPMINPORT:%d\n", envp->dpminport[i]);
		printf("DPMOUTPORT:%d\n", envp->dpmoutport[i]);
	}
	printf("DATADIR:%s\n",envp->datadir);
	printf("LOGFILE:%s\n",envp->logfile);
	return ;
}

int dp_env_init(void)
{
	char *p = NULL;
	char *envnamep;
	int i;
	
	envp = malloc(sizeof(struct dp_env));
	if (!envp) {
		return -1;
	}
	memset(envp, 0, sizeof(struct dp_env));
	
	p = getenv(dpmchannelmax);
	if (p) { 
		envp->dpmmax = atoi(p);
	} else {
		envp->dpmmax = DEFAULT_DPM_CHANNEL_MAX;
	}

	p = getenv(dpschannelmax);
	if (p) { 
		envp->dpsmax = atoi(p);
	} else {
		envp->dpsmax = DEFAULT_DPS_CHANNEL_MAX;
	}

	envp->dpminip = malloc(envp->dpmmax<<2);
	envp->dpmoutip = malloc(envp->dpmmax<<2);
	envp->dpminport = malloc(envp->dpmmax*sizeof(int));
	envp->dpmoutport = malloc(envp->dpmmax*sizeof(int));
		
	envp->dpsinip = malloc(envp->dpsmax<<2);
	envp->dpsoutip = malloc(envp->dpsmax<<2);
	envp->dpsinport = malloc(envp->dpsmax*sizeof(int));
	envp->dpsoutport = malloc(envp->dpsmax*sizeof(int));
	envnamep = malloc(20);
	for (i = 0; i < envp->dpmmax;i++) {
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPMINIP], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpminip[i] = p;
		}else {
			envp->dpminip[i] = DEFAULT_DPM_INPUT_IP;
		}

		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPMOUTIP], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpmoutip[i] = p;
		}else {
			envp->dpmoutip[i] = DEFAULT_DPM_OUTPUT_IP;
		}
	
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPMINPORT], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpminport[i] = atoi(p);
		} else {
			envp->dpminport[i] = DEFAULT_DPM_INPUT_PORT+i;//bug todo
		}
	
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPMOUTPORT], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpmoutport[i] = atoi(p);
		} else {
			envp->dpmoutport[i] = DEFAULT_DPM_OUTPUT_PORT+i;//bug todo
		}
	}
	
	for (i = 0; i < envp->dpsmax;i++) {
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPSINIP], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpsinip[i] = p;
		}else {
			envp->dpsinip[i] = DEFAULT_DPS_INPUT_IP;
		}

		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPSOUTIP], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpsoutip[i] = p;
		}else {
			envp->dpsoutip[i] = DEFAULT_DPS_OUTPUT_IP;
		}
	
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPSINPORT], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpsinport[i] = atoi(p);
		} else {
			envp->dpsinport[i] = DEFAULT_DPS_INPUT_PORT+i;//bug todo
		}
	
		memset(envnamep, 0, 20);
		sprintf(envnamep, "%s%d", envdpipport[DPSOUTPORT], i);
		p = getenv(envnamep);
		if (p) {
			envp->dpsoutport[i] = atoi(p);
		} else {
			envp->dpsoutport[i] = DEFAULT_DPS_OUTPUT_PORT+i;//bug todo
		}
	}
	
	p = getenv(envdatadir);
	if (p) { 
		envp->datadir = p;
	} else {
		envp->datadir = DEFAULT_DATA_DIR;
	}
	p = getenv(envlogfile);
	if (p) { 
		envp->logfile = p;
	} else {
		envp->logfile = DEFAULT_LOGFILE;
	}
	//dump_env();
	return 0;
}

char *get_dpsout_ip(int i)
{
	return envp->dpsoutip[i];	
}

char *get_dpsin_ip(int i)
{
	return envp->dpsinip[i];	
}

char *get_dpmout_ip(int i)
{
	return envp->dpmoutip[i];	
}

char *get_dpmin_ip(int i)
{
	return envp->dpminip[i];	
}


int get_dpmin_port(int i)
{
	return envp->dpminport[i];	
}
int get_dpsin_port(int i)
{
	return envp->dpsinport[i];	
}


int get_dpmout_port(int i)
{
	return envp->dpmoutport[i];	
}
int get_dpsout_port(int i)
{
	return envp->dpsoutport[i];	
}

char *get_data_dir(void)
{
	return envp->datadir;
}
char *get_logfilename(void)
{
	return envp->logfile;
}


int get_dpmchannel_max(void)
{
	return envp->dpmmax;
}

int get_dpschannel_max(void)
{
	return envp->dpsmax;
}

