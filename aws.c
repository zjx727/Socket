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

#define TCPCLIENTPORT "24765"
#define TCPMONITORPORT "25765"
#define UDPPORT "23765"
#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	int AWSC_sockfd,client_sockfd;
	int AWSM_sockfd,monitor_sockfd;
	int AWSA_sockfd;
	int AWSB_sockfd; 
	//UDP socket address structure
	struct sockaddr_in AWSA_addr;
	struct sockaddr_in AWSB_addr;
	struct sockaddr_in serverA_addr;
	struct sockaddr_in serverB_addr;
	//parent socket address structure
	struct sockaddr_in AWSC_addr; 
	struct sockaddr_in AWSM_addr;
	//child socket address structure
	struct sockaddr_in client_addr;
	struct sockaddr_in monitor_addr;
	char buf[MAXBUFLEN];
	int backlog=10;
	int len,sin_size;
	double write[4],data[4];
	double compute[3],result[3];
	int i,j,m;
	
	
////////TCP socket for client///////////////////////////////////////////////////////////////////////
	if ((AWSC_sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		return 1;
	}
	
	memset(&AWSC_addr,0,sizeof(AWSC_addr));   //make sure the struct is empty
	AWSC_addr.sin_family=AF_INET;		//ip address format
	AWSC_addr.sin_port=htons(24765);		//port number of AWS
	AWSC_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	
	printf("The AWS is up and running.\n");
	
	if (bind(AWSC_sockfd,(struct sockaddr *)&AWSC_addr, sizeof(struct sockaddr))==-1)
	{
		perror(" client bind");
		return 1;
	}	
	listen(AWSC_sockfd, backlog);
	sin_size=sizeof(struct sockaddr_in);
	
////////UDP for server A/////////////////////////////////////////////////////////////////////////////
		if ((AWSA_sockfd=socket(AF_INET,SOCK_DGRAM,0)) == -1)
		{
			perror("socket");
			return 1;
		}
	
		if (bind(AWSA_sockfd,(struct sockaddr *)&AWSC_addr, sizeof(struct sockaddr))==-1)
		{
			perror(" A bind");
			return 1;
		}
	
		memset(&serverA_addr,0,sizeof(serverA_addr));
		serverA_addr.sin_family=AF_INET;		//ip address format
		serverA_addr.sin_port=htons(21765);		//port number of serverA
		serverA_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

////////UDP for server B//////////////////////////////////////////////////////////////////////////////
		if ((AWSB_sockfd=socket(AF_INET,SOCK_DGRAM,0)) == -1)
		{
			perror("socket");
			return 1;
		}
	
		memset(&serverB_addr,0,sizeof(serverB_addr));
		serverB_addr.sin_family=AF_INET;		
		serverB_addr.sin_port=htons(22765);	
		serverB_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

	
////////TCP child socket for monitor
	if ((AWSM_sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		return 1;
	}
	
	memset(&AWSM_addr,0,sizeof(AWSM_addr));   //make sure the struct is empty
	AWSM_addr.sin_family=AF_INET;		//ip address format
	AWSM_addr.sin_port=htons(25765);		//port number of AWS
	AWSM_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	if (bind(AWSM_sockfd,(struct sockaddr *)&AWSM_addr, sizeof(struct sockaddr))==-1)
	{
		perror("monitor bind");
		return 1;
	}
	
	listen(AWSM_sockfd, backlog);
	
	sin_size=sizeof(struct sockaddr_in);
	if ((monitor_sockfd=accept(AWSM_sockfd,(struct sockaddr *)&monitor_addr,&sin_size)) == -1)
	{
		perror("accept");
		return 1;
	}
	
	while(1)
	{
		//printf("start while\n");
		if ((client_sockfd=accept(AWSC_sockfd,(struct sockaddr *)&client_addr,&sin_size)) == -1)
		{
			perror("accept");
			return 1;
		}
	
		recv(client_sockfd,buf,sizeof(buf),0);
		argc=atoi(buf);
	
		if (argc == 6)
		{
			for (i=0;i<4;i++)
			{
				len=recv(client_sockfd,buf,sizeof(buf),0);
				write[i]=strtod(buf,NULL);
			}
			printf("The AWS received write operation from the client using TCP over the port <%s>.\n", TCPCLIENTPORT);
		}

		if (argc == 5)
		{
			for (i=0;i<3;i++)
			{
				len=recv(client_sockfd,buf,sizeof(buf),0);
				compute[i]=strtod(buf,NULL);
			}
			printf("The AWS received compute operation from the client using TCP over the port <%s>.\n",TCPCLIENTPORT);
		}


		//AWS sent write function to monitor
		if (argc == 6)
		{
			strcpy(buf,"6");
			send(monitor_sockfd,buf,sizeof(buf),0);
			for (j=0;j<4;j++)
			{
				sprintf(buf,"%lf\n",write[j]);
				len=send(monitor_sockfd,buf,sizeof(buf),0);
			}	
			printf("The AWS sent operation write and arguments to the monitor using TCP over port number <%s>.\n",TCPMONITORPORT);
		}
		if (argc == 5)
		{
			strcpy(buf,"5");
			send(monitor_sockfd,buf,sizeof(buf),0);
			printf("test:no error\n");
		}
	
		//*******AWS sent write to server A*******//
		int a=sizeof(serverA_addr);
		if (argc == 6)
		{
			strcpy(buf,"6");
			sendto(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr,sizeof(struct sockaddr));
			for (j=0;j<4;j++)
			{
				sprintf(buf,"%lf\n",write[j]);
				len=sendto(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr,sizeof(struct sockaddr));
				//printf("sent write to serverA: %f.buf is %s.\n",write[j],buf);
			}
			printf("The AWS sent operation write Backened-serverA using UDP over port <%s>.\n",UDPPORT);	
		
			//printf("prepare to receive write ACK.\n");
			if ((len=recvfrom(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr, &a)) == -1)
			{
				perror("recvfrom");
				return 1;
			}
			printf("The AWS received response from Backened-Server A for writing using UDP over port <%s>\n",UDPPORT);
			//printf("%s\n",buf);
			
			//*******AWS sent the ACK message to the client and monitor*******//
			strcpy(buf,"Success ACK");
			len=send(client_sockfd,buf,sizeof(buf),0);
			printf("The AWS sent result to client for operation <write> using TCP over port <%s>.\n",TCPCLIENTPORT);
			len=send(monitor_sockfd,buf,sizeof(buf),0);
			printf("The AWS sent write response to the monitor using TCP over port <%s>\n",TCPMONITORPORT);
		}
	
		if (argc == 5)
		{
			//*******send the compute data to server A*******//
			strcpy(buf,"5");
			sendto(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr,sizeof(struct sockaddr));
			for (j=0;j<3;j++)
			{
				//printf("sent compute to serverA: %f.\n",compute[j]);
				sprintf(buf,"%lf\n",compute[j]);
				len=sendto(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr,sizeof(struct sockaddr));
				//printf("sent compute to server A: %f.\n",compute[j]);
			}	
			printf("The AWS sent operation compute to Backened-serverA using UDP over port <%s>.\n",UDPPORT);	
		
			//*******AWS received the data from the database of server A*******//
			len=recvfrom(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr, &a);
			len=send(client_sockfd,buf,sizeof(buf),0);
			len=send(monitor_sockfd,buf,sizeof(buf),0);
			//printf("test3:m=<%s>, no error\n",buf);
			m=atoi(buf);
			if (m == 1)
			{
				//*******AWS sent compute function to monitor*******//
				for (j=0;j<3;j++)
				{
					sprintf(buf,"%lf\n",compute[j]);
					len=send(monitor_sockfd,buf,sizeof(buf),0);
					//printf("sent compute to monitor: %f.buf is: %s.\n",compute[j],buf);

				printf("The AWS sent operation compute and arguments to the monitor using TCP over port number <%s>.\n",TCPMONITORPORT);
				}
				
				//*******AWS received data from server A*******//
				memset(data,0,sizeof(data));
				memset(buf,0,MAXBUFLEN);
				for(j=0;j<4;j++)
				{
					//printf("buf is %s\n",buf);
					len=recvfrom(AWSA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverA_addr, &a);
					data[j]=strtod(buf,NULL);
					//printf("test: receive data =%lf\n",data[j]);
				}

				printf("The AWS received link information from Backened-Server A using UDP over port <%s>\n",UDPPORT);
				
				//*******AWS sent compute data from client to server B*******//
				for (j=0;j<3;j++)
				{
					//printf("sent compute to serverB: %f.\n",compute[j]);
					sprintf(buf,"%lf\n",compute[j]);
					len=sendto(AWSB_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverB_addr,sizeof(struct sockaddr));
				}
				printf("AWS sent link ID = <%f>, size <%f>, power <%f>, and link information to Backen-ServerB using UDP over port <%s>.\n",compute[0],compute[1],compute[2],UDPPORT);	
	
				for (j=0;j<4;j++)
				{
					//printf("sent data to serverb: %f\n",data[j]);
					sprintf(buf,"%lf\n",data[j]);
					len=sendto(AWSB_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverB_addr,sizeof(struct sockaddr));
				}
	
				//*******AWS received the results from server B*******//
				int b=sizeof(serverB_addr);
				for(j=0;j<3;j++)
				{
					len=recvfrom(AWSB_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &serverB_addr, &b);
					result[j]=strtod(buf,NULL);
					//printf("test: result =%lf\n",result[j]);
				}
				printf("AWS received outputs from Backened-ServerB using UDP over port <%s>.\n",UDPPORT);
	
				//*******AWS sent the results to client and monitor*******//
				//sprintf(buf,"%f",compute[0]);
				//len=send(monitor_sockfd,buf,sizeof(buf),0);
				for (j=0;j<3;j++)
				{
					sprintf(buf,"%f",result[j]);
					len=send(client_sockfd,buf,sizeof(buf),0);				
					len=send(monitor_sockfd,buf,sizeof(buf),0);
					//printf("send: %s\n",buf);	
				}
				printf("The AWS sent result to client for operation compute using TCP over port <%s>\n",TCPCLIENTPORT);
				printf("The AWS sent compute results to the monitor using TCP over port <%s>\n",TCPMONITORPORT);		
			}
			
			else 
			{
				printf("Link ID not found\n");
			}
		}			
	}     //while(1) end
}
