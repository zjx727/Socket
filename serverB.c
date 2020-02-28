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
#include<netdb.h>
#include<math.h>

#define BPORT "22765"
#define MAXBUFLEN 100

int main()
{
	int serverB_sockfd;
	int AWSB_sockfd;
	struct sockaddr_in serverB_addr;
	struct sockaddr_in AWSB_addr;  
	char buf[MAXBUFLEN];
	int backlog=10;
	int len,sin_size;
	double compute[3],data[4];
	int i,j,ID;
	double Tt,Tp,delay,C,SNR;
	double result[3];
	double a,b,c;
		
	if ((serverB_sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("socket");
		return 1;
	}
	printf("The server B is up and running using UDP on port <%s>\n", BPORT);
	
	memset(&serverB_addr,0,sizeof(serverB_addr));   //make sure the struct is empty
	serverB_addr.sin_family=AF_INET;
	serverB_addr.sin_port=htons(22765);
	serverB_addr.sin_addr.s_addr=inet_addr("127.0.0.1");		
	
	if ((bind(serverB_sockfd,(struct sockaddr *)&serverB_addr, sizeof(serverB_addr)))==-1)
	{
		perror("bind");
		return 1;
	}
		
	memset(&AWSB_addr,0,sizeof(AWSB_addr));
	AWSB_addr.sin_family=AF_INET;		//ip address format
	AWSB_addr.sin_port=htons(23765);		//port number of serverA
	AWSB_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	if ((AWSB_sockfd=socket(AF_INET, SOCK_DGRAM, 0)) ==-1) 
	{
		perror("aws socket");
		return 1;
	}	
	sin_size=sizeof(struct sockaddr_in);	
	
	while(1)
	{
		//printf("start while\n");
		for (i=0;i<3;i++)
		{
			len=recvfrom(serverB_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &AWSB_addr, &sin_size);	
			compute[i]=strtod(buf,NULL);
			//printf("test: reveive compute is: %f.\n",compute[i]);
		}
		printf("The server B received link information: link <%f>, file size <%f>, and signal power <%f>\n",compute[0],compute[1],compute[2]);
	
		for (i=0;i<4;i++)
		{
			len=recvfrom(serverB_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &AWSB_addr, &sin_size);	
			data[i]=strtod(buf,NULL);
			//printf("test: reveive data is: %f.\n",data[i]);
		}
	
		SNR=compute[2]/data[3];
		C=1000000*data[0]*log(1+SNR)/log(2);
		Tt=compute[1]/C*1000;
		Tp=data[1]/data[2]*1000;
		delay=Tt+Tp;

		ID=(int)compute[0];
		//printf("The result for link <%d>:\n Tt=<%.2lf> ms\n Tp=<%.2lf> ms\n Delay=<%.2lf> ms\n",ID,Tt,Tp,delay);
		printf("The Server B finished the calculation for link <%d>\n",ID);
		result[0]=((int)(Tt*100+0.51))/100.0;
		result[1]=((int)(Tp*100+0.51))/100.0;
		result[2]=((int)(delay*100+0.51))/100.0;
		for (i=0;i<3;i++)
		{
			sprintf(buf,"%f",result[i]);
			len=sendto(serverB_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSB_addr,sizeof(struct sockaddr));
		}
		printf("The Server B finished sending the outputs to AWS\n");
	
	}
}
