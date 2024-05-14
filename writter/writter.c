#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>

struct str_plot{
	unsigned long timestamp;
	unsigned long data; 
} str_plot;

struct str_data{
	int type;
	unsigned int data;
} str_data;

struct str_label{
	int type;
	unsigned char label[10];
} str_label;

unsigned int get_cpu_usage(){
    int cpuinfo = open("/proc/stat", O_RDWR);
    char cpuLine[100];
    char cpuChar=0;
    int cpuCount=0; 
    const char d[2] = " ";
    char* token;
    int sum=0;
	
    if (cpuinfo < 0) {
        perror("uio open proc:");
        return errno;
    }
	
    while (cpuChar != '\n'){
	read(cpuinfo, &cpuChar,1); 
	cpuLine[cpuCount] = cpuChar;
	cpuCount++;   
    } 
    cpuLine[cpuCount] = '\0';
    token = strtok(cpuLine,d);
    
    sum = 0;
    int i=0;
    int idle=0;
    float idleFraction=0;
    while(token!=NULL){
	token = strtok(NULL,d);
	if(token!=NULL){
		sum += atoi(token);
		if(i==3)
			idle = atoi(token);
		i++;
	}
    }	
    idleFraction = 100 - (idle)*100.0/(sum);
    printf("Busy for : %lf %% of the time.\n", idleFraction);
    close(cpuinfo);
    return (int) (idleFraction * 1000000);
}

unsigned int get_free_mem(){

    int cpuinfo = open("/proc/meminfo", O_RDWR);
    if (cpuinfo < 0) {
        perror("uio open meminfo:");
        return errno;
    }

    char meminfoStr[56];
    char MemTotalStr[26];
    read(cpuinfo, &meminfoStr,56);
    meminfoStr[55] = '\0';
    for (int i=0;i<17;i++) {
    	MemTotalStr[i] = meminfoStr[i+36];	
    }
    MemTotalStr[17] = '\0';
    int resMem = atoi(MemTotalStr);
    close(cpuinfo);
    return resMem;
}

int main()
{
    int uiofd,cpuinfo;
    int err;
    int i;
    struct tm *ts;
    char       buf[80];
    double 	   decimal_time;	      
    unsigned int command_high = 10; 
    time_t plot_time;
    struct str_label 	label0;
    struct str_data 	data0;

    uiofd = open("/dev/char_dev", O_RDWR);
    if (uiofd < 0) {
        perror("uio open:");
        return errno;
    }
    
    label0.type=0;
    strcpy(label0.label,"LBL0"); 
    pwrite(uiofd, &label0, sizeof(str_label), 0);
    
    while (1) {
    	int cpuUsage = get_cpu_usage();
      	data0.type=1;
    	data0.data=cpuUsage;
    	err = pwrite(uiofd, &data0, sizeof(str_data), 0);
    	if (err != 1) {
    		perror("write error:");
    	}
	command_high++;
	usleep(10000);
    }

    return errno;
}
