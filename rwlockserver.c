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
#define THREAD_COUNT 50

int num_str; 
char** theArray;

typedef struct {
	int readers;
	int writer;
	pthread_cond_t readers_proceed;
	pthread_cond_t writer_proceed;
	int pending_writers;
	pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

pthread_mutex_t mutex;

mylib_rwlock_t rwlock;

void mylib_rwlock_t_init(mylib_rwlock_t *l);
void mylib_rwlock_rlock(mylib_rwlock_t *l);
void mylib_rwlock_wlock(mylib_rwlock_t *l);
void mylib_rwlock_unlock(mylib_rwlock_t *l);

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
	//pthread_t t[20];
	pthread_t* t;
	
	pthread_mutex_init(&mutex, NULL);
	mylib_rwlock_t_init(&rwlock);	

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=atoi(argv[1]); // port from command line
	sock_var.sin_family=AF_INET;
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Socket has been created!\n");
		listen(serverFileDescriptor,2000);
		while(1)        //loop infinity
		{
			t = malloc(THREAD_COUNT*sizeof(pthread_t));
			for(i=0;i<THREAD_COUNT;i++)      //can support 20 clients at a time
			{
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				//printf("Connected to client %d\n",clientFileDescriptor);
				pthread_create(&t[i],NULL,ServerEcho,(void *)(uintptr_t)clientFileDescriptor);
			}
			for(i=0; i<THREAD_COUNT;i++)
			{	
				pthread_join(t[i],NULL);
			}
			free(t);
				
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
	//mylib_rwlock_unlock(&rwlock);
	// lock critical section
	//mylib_rwlock_unlock(&rwlock);	
	
	mylib_rwlock_rlock(&rwlock);
	printf("locked critical section\n");

	if(strcmp(operation, read_buf) == 0) {
		mylib_rwlock_unlock(&rwlock);
		mylib_rwlock_rlock(&rwlock);
		printf("Client %d wants to read...\n", clientFileDescriptor);
		
		// read the array postion the client wants to read from
		//mylib_rwlock_unlock(&rwlock);
		read(clientFileDescriptor, &converted_pos, sizeof(converted_pos));
		pos = ntohl(converted_pos);
		printf("Client %d sent pos is: %d\n", clientFileDescriptor, pos);

		// send the string at the specified position to the client
		write(clientFileDescriptor,theArray[pos],STR_LEN);
		printf("Got past write on read side \n");
		//mylib_rwlock_rlock(&rwlock);
		
		printf("got past unlock on read side \n");
		mylib_rwlock_unlock(&rwlock);	
	}
	else if(strcmp(operation, write_buf) == 0) {
		mylib_rwlock_unlock(&rwlock);
		//mylib_rwlock_wlock(&rwlock);		
		printf("Client %d wants to write...\n", clientFileDescriptor);


		// read the array postion the client wants to write to
		read(clientFileDescriptor, &converted_pos, sizeof(converted_pos));
		mylib_rwlock_wlock(&rwlock);		
		pos = ntohl(converted_pos);
		
		//mylib_rwlock_wlock(&rwlock);
		
		printf("Client %d sent pos is: %d\n", clientFileDescriptor, pos);
		//mylib_rwlock_unlock(&rwlock);
		//mylib_rwlock_wlock(&rwlock);

		// write to the array
		sprintf(theArray[pos], "String %d has been modified by a write request", pos);
		
		// send the string at the modified position to the client
		write(clientFileDescriptor,theArray[pos],STR_LEN);
		mylib_rwlock_unlock(&rwlock);
	}

	close(clientFileDescriptor);

	return NULL;
}

void mylib_rwlock_t_init(mylib_rwlock_t *l){
	l -> readers = l -> writer = l -> pending_writers = 0;
	pthread_mutex_init(&(l -> read_write_lock), NULL);
	pthread_cond_init(&(l -> readers_proceed), NULL);
	pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> pending_writers > 0) || (l -> writer > 0))
		pthread_cond_wait(&(l -> readers_proceed),
			&(l -> read_write_lock));
	l -> readers ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
  /* if there are readers or writers, increment pending
	writers count and wait. On being woken, decrement pending
	writers count and increment writer count */
  pthread_mutex_lock(&(l -> read_write_lock));
  while ((l -> writer > 0) || (l -> readers > 0)) {
  		l -> pending_writers ++;
    		pthread_cond_wait(&(l -> writer_proceed),
    			&(l -> read_write_lock));
  	l -> pending_writers --;
  }
  l -> writer ++;
  pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
  /* if there is a write lock then unlock, else if there
	are read locks, decrement count of read locks. If the count
	is 0 and there is a pending writer, let it through, else if
	there are pending readers, let them all go through */
  pthread_mutex_lock(&(l -> read_write_lock));
  if (l -> writer > 0)
    l -> writer = 0;
  else if (l -> readers > 0)
    l -> readers --;
  pthread_mutex_unlock(&(l -> read_write_lock));
  if ((l -> readers == 0) && (l -> pending_writers > 0))
    pthread_cond_signal(&(l -> writer_proceed));
  else if (l -> readers > 0)
    pthread_cond_broadcast(&(l -> readers_proceed));
 }

void initArray() {
	// initialize the array with number of strings
	theArray = malloc(num_str*sizeof(char[STR_LEN]));

	int i;
	for(i = 0; i < num_str; i++) {
		theArray[i] = malloc(STR_LEN*sizeof(char));
		//sprintf(theArray[i], "theArray[%d]: initial value", i);
	}

}
