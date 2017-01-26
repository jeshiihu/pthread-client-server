#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define thread_count 1000
#define STR_LEN 1000

int NUM_STR; 
char** theArray;
unsigned int* seed;
pthread_mutex_t mutex;

void initSeed();
void initArray();
void *ServerEcho(void *args);
void *Operate(void* rank);  /* Thread function */

// command line arguements 
int main(int argc, char* argv[])
{
	if(argc != 3) {
		printf("Error: incorrect number of arguments, please try again\n");
		return 0;
	}

	NUM_STR = atoi(argv[2]); // number of strings in the array
	initSeed();
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
	return 0;
}

void *ServerEcho(void *args)
{
	int clientFileDescriptor=(int)args;
	char str[20];

	read(clientFileDescriptor,str,20);
	printf("\nreading from client: %s",str);
	write(clientFileDescriptor,str,20);
	printf("\nechoing back to client");
	close(clientFileDescriptor);

	return NULL;
}

void initSeed() {
	seed = malloc(thread_count*sizeof(int));

	int i;
	for (i = 0; i < thread_count; i++)
		seed[i] = i;
}

void initArray() {
	// initialize the array with number of strings
	theArray = malloc(NUM_STR*sizeof(char[STR_LEN]));

	int i;
	for(i = 0; i < NUM_STR; i++) {
		theArray[i] = malloc(STR_LEN*sizeof(char));
	}
}

void *Operate(void* rank) {
	long my_rank = (long) rank;
	
	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % NUM_STR;
	int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability
	
	pthread_mutex_lock(&mutex); 

	if (randNum >= 95) // 95% are write operations, others are reads
		sprintf(theArray[pos], "theArray[%d] modified by thread %ld", pos, my_rank);

	printf("Thread %ld: randNum = %d\n", my_rank, randNum);
	printf("%s\n\n", theArray[pos]); // return the value read or written

	pthread_mutex_unlock(&mutex);
		
	return NULL;
}
