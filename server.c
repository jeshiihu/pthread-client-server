#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

#define STR_LEN 1000

int num_str; 
char** theArray;
pthread_mutex_t mutex;

void initArray();
void *ServerEcho(void *args);

// command line arguements 
int main(int argc, char* argv[])
{
	if(argc != 3) {
		printf("Error: incorrect number of arguments, please try again\n");
		return 0;
	}

	num_str = atoi(argv[2]); // number of strings in the array
	
	initArray();

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	pthread_t t[20];

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=atoi(argv[1]); // port from command line
	sock_var.sin_family=AF_INET;
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Socket has been created!\n");
		listen(serverFileDescriptor,2000);
		pthread_mutex_init(&mutex, NULL); 
		while(1)        //loop infinity
		{
			for(i=0;i<20;i++)      //can support 20 clients at a time
			{
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("Connected to client %d\n",clientFileDescriptor);
				pthread_create(&t,NULL,ServerEcho,(void *)(uintptr_t)clientFileDescriptor);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("\nsocket creation failed\n");
	}

	pthread_mutex_destroy(&mutex);
	return 0;
}

void *ServerEcho(void *args)
{
	int clientFileDescriptor=(int)args;
	int converted_pos;
	int pos;

	// used to know whether the client wants to read or write
	char operation[STR_LEN];
	char read_buf[] = "read";
	char write_buf[] = "write";

	// reading the operation from the client
	read(clientFileDescriptor,operation,STR_LEN);
	printf("Operation from client %d: %s\n", clientFileDescriptor, operation);

	// lock critical section
	pthread_mutex_lock(&mutex);

	if(strcmp(operation, read_buf) == 0) {
		printf("Client %d wants to read...\n", clientFileDescriptor);
		
		// read the array postion the client wants to read from
		read(clientFileDescriptor, &converted_pos, sizeof(converted_pos));
		pos = ntohl(converted_pos);
		printf("Client %d sent pos is: %d\n", clientFileDescriptor, pos);

		// send the string at the specified position to the client
		write(clientFileDescriptor,theArray[pos],STR_LEN);
	}
	else if(strcmp(operation, write_buf) == 0) {
		printf("Client %d wants to write...\n", clientFileDescriptor);

		// read the array postion the client wants to write to
		read(clientFileDescriptor, &converted_pos, sizeof(converted_pos));
		pos = ntohl(converted_pos);
		printf("Client %d sent pos is: %d\n", clientFileDescriptor, pos);
		
		// write to the array
		sprintf(theArray[pos], "String %d has been modified by a write request", pos);
		
		// send the string at the modified position to the client
		write(clientFileDescriptor,theArray[pos],STR_LEN);
	}

	pthread_mutex_unlock(&mutex);
	close(clientFileDescriptor);

	return NULL;
}

void initArray() {
	// initialize the array with number of strings
	theArray = malloc(num_str*sizeof(char[STR_LEN]));

	int i;
	for(i = 0; i < num_str; i++) {
		theArray[i] = malloc(STR_LEN*sizeof(char));
		sprintf(theArray[i], "theArray[%d]: initial value", i);
	}

}
