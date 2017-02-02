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
#define string_length 100

int NUM_STR; 
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

	NUM_STR = atoi(argv[2]); // number of strings in the array
	initArray();

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	pthread_t t[20];

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=atoi(argv[1]);
	sock_var.sin_family=AF_INET;
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("\nsocket has been created");
		listen(serverFileDescriptor,2000);
		pthread_mutex_init(&mutex, NULL); 
		while(1)        //loop infinity
		{
			for(i=0;i<20;i++)      //can support 20 clients at a time
			{
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("\nConnected to client %d\n",clientFileDescriptor);
				pthread_create(&t,NULL,ServerEcho,(void *)(uintptr_t)clientFileDescriptor);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("\nsocket creation failed\n");
	}
	pthread_mutex_destroy(&mutex);
	//free(t);
	return 0;
}

void *ServerEcho(void *args)
{
	int clientFileDescriptor=(int)args;
	int requested_string;
	int received_pos;
	char str[string_length];
	char read_buf[] = "read";
	char write_buf[] = "write";


	read(clientFileDescriptor,str,string_length);
	printf("\nreading from client: %s",str);

	pthread_mutex_lock(&mutex);

	if(strcmp(str, read_buf) == 0) {
		read(clientFileDescriptor, &requested_string, sizeof(requested_string));
		received_pos = ntohl(requested_string);
		printf("received_pos is: %d\n", received_pos);
		write(clientFileDescriptor,theArray[received_pos],string_length);	// array stuff
	}
	else if(strcmp(str, write_buf) == 0) {
		printf("\nGot a write");
		read(clientFileDescriptor, &requested_string, sizeof(requested_string));
		received_pos = ntohl(requested_string);
		sprintf(theArray[received_pos], "String %d has been modified by a write request", received_pos);
		write(clientFileDescriptor,theArray[received_pos],string_length);
		printf("\nGot through the write");
		printf("\nechoing back to client: %s", str);
	}

	pthread_mutex_unlock(&mutex);
	close(clientFileDescriptor);
	

	return NULL;
}

void initArray() {
	// initialize the array with number of strings
	theArray = malloc(NUM_STR*sizeof(char[STR_LEN]));

	int i;
	for(i = 0; i < NUM_STR; i++) {
		theArray[i] = malloc(STR_LEN*sizeof(char));
		sprintf(theArray[i], "theArray[%d]: initial value", i);
	}

}
