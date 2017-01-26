#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

int NUM_STR;

int main(int argc, char* argv[])
{
	if(argc != 3) {
		printf("Error: incorrect number of arguments, please try again\n");
		return 0;
	}

	NUM_STR = atoi(argv[2]); // number of string in array

	struct sockaddr_in sock_var;
	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	char str_clnt[20],str_ser[20];

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=atoi(argv[1]); // pass in port number by command line arg
	sock_var.sin_family=AF_INET;

	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Connected to server %d\n",clientFileDescriptor);
		printf("\nEnter String to send: ");
		scanf("%s",str_clnt);
		write(clientFileDescriptor,str_clnt,20);
		read(clientFileDescriptor,str_ser,20);
		printf("String from Server: %s\n",str_ser);
		close(clientFileDescriptor);
	}
	else{
		printf("\nsocket creation failed\n");
	}
	return 0;
}