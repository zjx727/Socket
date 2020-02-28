#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	int monitor_sockfd;
	struct sockaddr_in AWSM_addr;
	char buf[MAXBUFLEN];
	int len;
	double write[4];
	double compute[3],result[3];
	int i,j,ID,m;
	
	if ((monitor_sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		return 1;
	}
	
	memset(&AWSM_addr,0,sizeof(AWSM_addr));
	AWSM_addr.sin_family=AF_INET;		//ip address format
	AWSM_addr.sin_port=htons(25765);		//port number of AWS
	AWSM_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	printf("The monitor is up and running.\n");
	
	if(connect(monitor_sockfd,(struct sockaddr *) &AWSM_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		return 1;
	}
	
	while(1)
	{
		//printf("start while\n");
		recv(monitor_sockfd,buf,sizeof(buf),0);
		argc=atoi(buf);
		//printf("argc==%d, no error\n",argc);
		if (argc == 6)
		{
			for (i=0;i<4;i++)
			{
				//printf("%s\n",buf);
				len=recv(monitor_sockfd,buf,sizeof(buf),0);
				write[i]=strtod(buf,NULL);
				//printf("test: receive write=%f.\n",write[i]);
			}
			printf("The monitor received BW=%f, L=%f, V=%f and P=%f from the AWS.\n",write[0],write[1],write[2],write[3]);

			len=recv(monitor_sockfd,buf,sizeof(buf),0);
			//printf("test: write ack from aws is %s.\n",buf);
			printf("The write operation has been completed successfully.\n");

		}
	
		if (argc == 5)
		{

	
			//*******receive the results from AWS*******//
			len=recv(monitor_sockfd,buf,sizeof(buf),0);
			m=atoi(buf);
			//printf("m=%d, no error\n",m);
			if (m == 1)
			{
				//*******monitor received the data from AWS*******//
				for (i=0;i<3;i++)
				{
					len=recv(monitor_sockfd,buf,sizeof(buf),0);
					compute[i]=strtod(buf,NULL);
					//printf("test: receive compute=%f.\n",compute[i]);
				}
				ID=(int)compute[0];
				printf("The monitor received link ID=%d, size=%.2lf and power=%.2lf from the AWS.\n",ID,compute[1],compute[3]);
				//*******monitor received the results from AWS*******//
				for (i=0;i<3;i++)
					{
						len=recv(monitor_sockfd,buf,sizeof(buf),0);
						result[i]=strtod(buf,NULL);
						//printf("test: receive compute=%f.\n",result[i]);
					}
				printf("The result for link <%d>:\nTt = <%.2f> ms\nTp = <%.2f> ms\nDelay = <%.2f> ms\n",ID,result[0],result[1],result[2]);
			}
			else 
			{
				printf("Link ID not found.\n");
			}
		}
	}
}
