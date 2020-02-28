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
	int client_sockfd;
	struct sockaddr_in AWSC_addr;
	char buf[MAXBUFLEN];
	int len;
	double result[3];
	int i,j,ID,m;
	
	if ((client_sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		return 1;
	}
	
	memset(&AWSC_addr,0,sizeof(AWSC_addr));
	AWSC_addr.sin_family=AF_INET;		//ip address format
	AWSC_addr.sin_port=htons(24765);		//port number of AWS
	AWSC_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

	printf("The client is up and running.\n");
	
	if (connect(client_sockfd,(struct sockaddr *) &AWSC_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		return 1;
	}
	
		
	//find the function <write> <compute>
	//write function
	if (argc==6)
	{
		strcpy(buf,"6");
		send(client_sockfd,buf,sizeof(buf),0);
		for (i=2;i<6;i++)
		{
			strcpy(buf,argv[i]);
			//printf("%s.\n",buf);
			len=send(client_sockfd,buf,sizeof(buf),0);
		}	
		printf("The client sent write operation to AWS. \n");
		len=recv(client_sockfd,buf,sizeof(buf),0);
		printf("The write operation has been completed successfully.\n");
		
	}
	//len=recv(client_sockfd,buf,sizeof(buf),0);
	//printf("The write operation has been completed successfully.\n");
		
	//compute function
	if (argc==5)
	{
		strcpy(buf,"5");
		send(client_sockfd,buf,sizeof(buf),0);
		for (j=2;j<5;j++)
		{
			strcpy(buf,argv[j]);
			//printf("%s.\n",buf);
			len=send(client_sockfd,buf,sizeof(buf),0);
		}
		ID=atoi(argv[2]);
		printf("The client sent ID=%s, size=%s,and power=%s to AWS.\n",argv[2],argv[3],argv[4]);
	

	
		//receive the results from AWS
		len=recv(client_sockfd,buf,sizeof(buf),0);
		m=atoi(buf);
		if (m == 1)
		{
			for (j=0;j<3;j++)
			{
				len=recv(client_sockfd,buf,sizeof(buf),0);
				result[j]=strtod(buf,NULL);
				//printf("The AWS sent result to client for operation compute using TCP over port <%s>\n",TCPCLIENTPORT);
			}
			printf("The delay for link <%d> is <%.2f>:\n",ID,result[2]);
			}
		else 
		{
			printf("Link ID not found.\n");
		}
	}

	close(client_sockfd);
	return 0;
	
}
