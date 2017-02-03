#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define THREAD_COUNT 3
#define STR_LEN 1000

int num_str, port;
unsigned int* seed;

void initSeed();
void *Operate(void* rank);

int main(int argc, char* argv[]) {

	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles; 

	if(argc != 3) {
		printf("Error: incorrect number of arguments, please try again\n");
		return 0;
	}

	port = atoi(argv[1]);
	num_str = atoi(argv[2]); // number of string in array
	
	initSeed();

	thread_handles = malloc(THREAD_COUNT*sizeof(pthread_t)); 

	for (thread = 0; thread < THREAD_COUNT; thread++)  
		pthread_create(&thread_handles[thread], NULL, Operate, (void*) thread);  

	for (thread = 0; thread < THREAD_COUNT; thread++) 
		pthread_join(thread_handles[thread], NULL); 

	free(thread_handles);
	free(seed);

	return 0;
}


void initSeed() {
	seed = malloc(THREAD_COUNT*sizeof(int));

	int i;
	for (i = 0; i < THREAD_COUNT; i++)
		seed[i] = i;
}

void *Operate(void* rank) {
	long my_rank = (long) rank;

	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % num_str;
	int converted_pos = htonl(pos);

	int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability

	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	char str_clnt[STR_LEN], str_ser[STR_LEN];

	struct sockaddr_in sock_var;
	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=port;
	sock_var.sin_family=AF_INET;

	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Client %d has connected to the server!\n",clientFileDescriptor);

		if (randNum >= 95) { // 5% are write operations, others are reads
			printf("Client %d is writing...\n", clientFileDescriptor);

			// send write msg to the server - acts like a flag
			snprintf(str_clnt, sizeof(str_clnt), "write");
			write(clientFileDescriptor,str_clnt,STR_LEN);

			// send position num to server (as a network byte order)
			printf("Client %d wants to write to pos: %d\n", clientFileDescriptor, pos);
			write(clientFileDescriptor, &converted_pos, sizeof(converted_pos));

			// read from server - array should be modified
			read(clientFileDescriptor,str_ser,STR_LEN);
			printf("Client %d received: %s\n", clientFileDescriptor, str_ser);
		} 
		else {
			printf("Client %d is reading...\n", clientFileDescriptor);

			// sending read message to let server know if read or write
			snprintf(str_clnt, sizeof(str_clnt), "read");
			write(clientFileDescriptor, str_clnt, STR_LEN);

			// send position num to server (as a network byte order)
			printf("Client %d wants to read to pos: %d\n", clientFileDescriptor, pos);
			write(clientFileDescriptor, &converted_pos, sizeof(converted_pos));

			// read from server
			read(clientFileDescriptor,str_ser,STR_LEN);
			printf("String from Server: %s\n",str_ser);
		}

		close(clientFileDescriptor);
	}
	else{
		printf("\nsocket creation failed\n");
	}
	
	return 0; 
}


