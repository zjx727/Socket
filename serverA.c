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

#define LINE 100
#define APORT "21765"
#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	int serverA_sockfd;
	int AWSA_sockfd;
	struct sockaddr_in serverA_addr;
	struct sockaddr_in AWSA_addr;  
	char buf[MAXBUFLEN];
	int backlog=10;
	int len,sin_size;
	double write[4];
	double compute[3];
	int i,j,k,m;
	int d[LINE][0];
	double in[LINE][4];
	double data[LINE][4];
	double out[4];
	int ID;
	FILE *fp=NULL;
	FILE *fp1 = NULL;
	FILE *fp2=NULL;
	
	
	if ((serverA_sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("socket");
		exit(1);
	}
	printf("The server A is up and running using UDP on port <%s>\n", APORT);	
	
	memset(&serverA_addr,0,sizeof(serverA_addr));   //make sure the struct is empty
	serverA_addr.sin_family=AF_INET;
	serverA_addr.sin_port=htons(21765);
	serverA_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	if (bind(serverA_sockfd,(struct sockaddr *) &serverA_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}
	
	memset(&AWSA_addr,0,sizeof(AWSA_addr));   //make sure the struct is empty
	AWSA_addr.sin_family=AF_INET;		//ip address format
	AWSA_addr.sin_port=htons(23765);		//port number of AWS
	AWSA_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	if ((AWSA_sockfd=socket(AF_INET, SOCK_DGRAM, 0)) ==-1) 
	{
		perror("aws socket");
		exit(1);
	}
	
	while(1)
	{
		//printf("start while\n");
		recvfrom(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &AWSA_addr, &sin_size);
		argc=atoi(buf);
		//printf("test: argc=%d.\n",argc);

		//*******write function received*******//
		if (argc == 6)
		{
			for (i=0;i<4;i++)
			{
				len=recvfrom(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &AWSA_addr, &sin_size);	
				write[i]=strtod(buf,NULL);
				//printf("test: reveive write is: %f.\n",write[i]);

			}
			printf("The server A received input for writing.\n");
		
			////write the data into database
			fp=fopen("database.txt","a+");
			fclose(fp);
	
			fp1=fopen("database.txt","r");
			fscanf(fp1,"%d",&d[0][0]);
			//printf("scand: %d\n",d[0][0]);		
			if (d[0][0] == 0)
			{
				d[0][0]=1;
				fp=fopen("database.txt","a+");
				fprintf(fp,"%d",d[0][0]);
				for (j=0;j<4;j++)
				{
					in[i][j]=write[j];
					//printf("write in %f\n", in[i][j]);
					fprintf(fp,"	%f",in[i][j]);
					//printf("test\n");
				}
				fputs("\r\n",fp);
				printf("The server A wrote link <%d> to database.\n",d[0][0]);			
				fclose(fp);
				for(j=1;j<5;j++)
				{
					fscanf(fp1,"%lf",&data[0][j]);
					//printf(" %f",data[0][j]);
				}
			}
			else 
			{
			
				for(j=1;j<5;j++)
					{
						fscanf(fp1,"%lf",&data[0][j]);
						//printf(" %f\n",data[0][j]);
					}
			
				for (i=1;i<LINE;i++)
				{
					fscanf(fp1,"%d",&d[i][0]);
					//printf("line is %d\n",i);
					//printf("d[%d][0] = %d\n",i,d[i][0]);		
					if (d[i][0] == i+1)    
					{
						//printf("link id is not empty\n");
						for(j=1;j<5;j++)
						{
							fscanf(fp1,"%lf",&data[i][j]);
							//printf(" %f",data[i][j]);
						}
						//printf("\n");
					}
					else 
					{
						//printf("write from %d\n",i+1);
						//printf("i=%d\n",i);
						fclose(fp);
						fp=fopen("database.txt","a+");
						d[i][0]=i+1;
						fprintf(fp,"%d",d[i][0]);
						for (j=0;j<4;j++)
						{
							in[i][j]=write[j];
							//printf("write  %f\n", in[i][j]);
							fprintf(fp,"	%f",in[i][j]);
							//printf("write finished\n");
						}
						fputs("\r\n",fp);
						printf("The server A wrote link <%d> to database.\n",i+1);
						fp1=fopen("database.txt","r");
						fscanf(fp1,"%d",&d[i+1][0]);
						for(j=1;j<5;j++)
						{
							fscanf(fp1,"%lf",&data[i][j]);
							//printf("new write %d.\n",j);
							//printf(" %f\n",data[i][j]);
						}
						//printf("new write finished. \n");
						printf("\n");
						break;
					}
			
				}
				//printf("write finished\n");
				fclose(fp);
				fclose(fp1);
			}	
					
			//*******server A send write success ack to aws	*******//	
			//printf("prepare to send. \n");
			len=sendto(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSA_addr,sizeof(struct sockaddr));
			//printf("sent. \n");
			strcpy(buf,"Success ACK");
			//printf("test ack:buf is:%s.\n",buf);
		}	

		//*******compute function received*******//
		if (argc == 5)
		{

			for (i=0;i<3;i++)
			{
				len=recvfrom(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *) &AWSA_addr, &sin_size);	
				compute[i]=strtod(buf,NULL);
			}
			ID=(int)compute[0];
			printf("The server A received input <%d> for computing.\n",ID);
		

			fp2=fopen("database.txt","r");
			for (i=0;i<LINE;i++)
			{
				//printf("i=%d\n",i);
				fscanf(fp2,"%d",&d[i][0]);
				//printf("now the line is %d\n",d[i][0]);		
				for(j=1;j<5;j++)
				{
					fscanf(fp2,"%lf",&data[i][j]);
				}
				//printf("id=%d,i=%d\n",ID,i);				
				
			//*******look for the link ID in the database*******//
				if (ID == d[i][0])
				{
					m=1;
					sprintf(buf,"%d",m);
					len=sendto(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSA_addr,sizeof(struct sockaddr));
					//printf("id is %d\n",d[i][0]);
					for(k=0;k<4;k++)
					{
						out[k]=data[i][k+1];
						//printf("test: out is %f\n",out[k]);
					}
					for (j=0;j<4;j++)
					{
						sprintf(buf,"%lf",out[j]);
						//printf("test: buf is %s\n",buf);
						len=sendto(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSA_addr,sizeof(struct sockaddr));
					}
					printf("The server A finished sending the search result to AWS.\n");
					break;																																																																																																																				
				}
				else if ((i != 0) && (d[i][0] != (i+1)))
				{
					//printf("di-1=%d\n",d[i-1][0]);
					m=0;
					sprintf(buf,"%d",m);
					len=sendto(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSA_addr,sizeof(struct sockaddr));
					printf("Link ID not found.\n");
					//sprintf(buf,"%s","Link ID not found.");
					//len=sendto(serverA_sockfd,buf,sizeof(buf),0,(struct sockaddr *)&AWSA_addr,sizeof(struct sockaddr));
					break;
				}
			}
		fclose(fp2);
		}
	}	
}
