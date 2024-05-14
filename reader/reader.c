#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> 
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

struct str_plot collected_data[10];
int collected_data_counter=0;
void *funcUDP2(void *ptr)
{
}

void *funcUDP(void* ptr) {

	int sockfd;		/* socket */
	int portno = 3000; 	/* port to listen on */
	int clientlen;		/* byte size of client's address */
	struct sockaddr_in serveraddr;	/* server's addr */
	struct sockaddr_in clientaddr;	/* client addr */
	struct hostent *hostp;	/* client host info */

	char *hostaddrp;	/* dotted decimal host addr string */
	int optval;		/* flag value for setsockopt */
	int n;			/* message byte size */
	printf("udp\n");

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		perror("ERROR opening socket");

	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
		   (const void *)&optval, sizeof(int));

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *)&serveraddr,
		 sizeof(serveraddr)) < 0)
		perror("ERROR on binding");

	clientlen = sizeof(clientaddr);
	printf("connected\n");

	while (1) {
		char buf[1];
		n = recvfrom(sockfd, buf, 1, 0,
			     (struct sockaddr *)&clientaddr, &clientlen);
		if (n < 0)
			perror("ERROR in recvfrom");

		printf("data_received\n");

		int tempCounter=collected_data_counter;
		for (int i=0;i<10;i++)
		{
			int ap=tempCounter-i-1;
			char string_data[1024];
			if (ap>0) {
				char buf_ts[80];
	    			time_t plot_time;
				plot_time = collected_data[ap%10].timestamp/1000000000;  
		    		long int decimal_time = (collected_data[ap%10].timestamp%1000000000);
				struct tm *ts;
				ts = localtime(&plot_time);
		    		strftime(buf_ts, sizeof(buf_ts), "%a %Y-%m-%d %H:%M:%S %Z", ts);
				sprintf(string_data,"%d (%s.%d) - %lu\n", ap, buf_ts, decimal_time, collected_data[ap%10].data);
				n = sendto(sockfd, string_data, strlen(string_data), 0, (struct sockaddr *)&clientaddr, clientlen);
			} 
		}
	}

}


int main()
{
    int uiofd,cpuinfo;
    int err;
    int i;
    struct tm *ts;
    char       buf[80];
    long int   decimal_time;	      
    unsigned int command_high = 10; 
    time_t plot_time;
    struct str_label 	label0;
    struct str_data 	data0;
    
    uiofd = open("/dev/char_dev", O_RDWR);
    if (uiofd < 0) {
        perror("uio open:");
        return errno;
    }
    
    char temp_label[10];
    struct str_plot data;
    
    err = pread(uiofd, temp_label, 10, 10);
    
    pthread_t ptid; 
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int iret2 = pthread_create( &ptid, &attr, funcUDP, NULL);

    printf("label: %s\n", temp_label);  
  
    while (1) {
    	
    	err = pread(uiofd, &data, sizeof(str_plot), 0);
    	if (err != sizeof(str_plot)) {
    		perror("read error0:");
    	} else {
    		plot_time = data.timestamp/1000000000;  
    		decimal_time = (data.timestamp%1000000000);
    		ts = localtime(&plot_time);
    		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		collected_data[collected_data_counter%10].timestamp = data.timestamp;
		collected_data[collected_data_counter%10].data = data.data;
		collected_data_counter++;
    	}
    }
     
}
