#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define thread_count 1000

int NUM_STR;
unsigned int* seed;
// pthread_mutex_t mutex;

void initSeed();
void *Operate(void* rank);

int main(int argc, char* argv[]) {

	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles; 

	if(argc != 3) {
		printf("Error: incorrect number of arguments, please try again\n");
		return 0;
	}

	NUM_STR = atoi(argv[2]); // number of string in array
	initSeed();

	thread_handles = malloc (thread_count*sizeof(pthread_t)); 
	// pthread_mutex_init(&mutex, NULL); // very unsure of this... where client? server?

	for (thread = 0; thread < thread_count; thread++)  
		pthread_create(&thread_handles[thread], NULL, Operate, (void*) thread);  

	for (thread = 0; thread < thread_count; thread++) 
		pthread_join(thread_handles[thread], NULL); 

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


void initSeed() {
	seed = malloc(thread_count*sizeof(int));

	int i;
	for (i = 0; i < thread_count; i++)
		seed[i] = i;
}

void *Operate(void* rank) {
	long my_rank = (long) rank;
	
	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % NUM_STR;
	int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability
	
	// pthread_mutex_lock(&mutex); 
	if (randNum >= 95) // 5% are write operations, others are reads
		printf("jdsja\n");
		// sprintf(theArray[pos], "theArray[%d] modified by thread %d", pos, my_rank);
	printf("Thread %ld: randNum = %d\n", my_rank, randNum);
	// printf("%s\n\n", theArray[pos]); // return the value read or written
	// pthread_mutex_unlock(&mutex);
		
	return NULL;
}


