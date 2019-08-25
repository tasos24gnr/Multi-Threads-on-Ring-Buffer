#include "p3130141-p3130210-prodcons.h"
int prods;		//Global number of producers
int cons;		//Global number of consumers
int length;		//Global length of queue
int numofrandoms;	//Global sum of random numbers per producer
int seed;		//Global seed
circular_buffer *cb;
pthread_mutex_t lock;
int producerStarted=0;
int consumerStarted=0;
pthread_cond_t condition;
void *producer(void*);
void *consumer(void*);

int main(int argc,int ** argv){

	if (argc != 6) {
		printf("ERROR: The programm should take exactly 5 parameters! Try again...\n");
		exit(-1);
	}else{
		prods=  atoi(argv[1]);
			printf("prods: %i\n",prods);		
		cons=  atoi(argv[2]);
			printf("cons: %i\n",cons);		
		length=  atoi(argv[3]);	
			printf("length: %i\n",length);		
		numofrandoms=  atoi(argv[4]);
			printf("randoms: %i\n",numofrandoms);	
		seed=  atoi(argv[5]);
			printf("seed: %i\n",seed);			
			printf("/////////////////////////\n");
	}
	cb = (circular_buffer*)malloc(sizeof (struct circular_buffer));
	cb_init(cb, length, sizeof(int));

	pthread_t thread_prod[prods], thread_cons[cons];
	int rc = pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&condition,NULL);
	for(int i=1;i<=prods;i++){
		if (rc=pthread_create( &thread_prod[i], NULL, producer,(void *)(i)) ){
			printf("Error creating producers thread.");
			abort();
		}
	}
	for(int i=1;i<=cons;i++){
		if (rc=pthread_create( &thread_cons[i], NULL, consumer,(void *)(i)) ){
			printf("Error creating consumers thread.");
			abort();
		}
	}
	for(int i=1;i<=prods;i++){	
		if (rc=pthread_join(thread_prod[i], NULL)){
			printf("Error joining producers thread.");
			abort();
		}
	}
	for(int i=1;i<=cons;i++){	
		if (rc=pthread_join(thread_cons[i], NULL)){
			printf("Error joining consumers thread.");
			abort();
		}
	}

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&condition);
	exit(0);
}

void *producer(void *threadID){
	producerStarted=1;
	pthread_mutex_lock(&lock);	
	int tid;
	tid = (int)threadID;	
	unsigned int seedp=seed * (tid);
	int k=0;
	
	for(int i=0;i<numofrandoms;i++){
		int random= rand_r(&seedp)%100;			//Random ari8mos mexri to 100
	
		cb_push_back(cb, (void*)&random); 			//PUSH stin queue, ton random ari8mo 
		printf("PRODUCER %i: %i\n",tid,random);

		FILE * fl = fopen("prod_in.txt", "a");
		if(fl==NULL) {exit(1);}
		fprintf(fl,"Producer %i : %i\n",tid,random);
		fclose(fl);
		if(cb->count==length){
			if (k==0) {k++; pthread_cond_signal(&condition);}
			else pthread_cond_broadcast(&condition);
			pthread_cond_wait(&condition,&lock);
	
		}
	}
	pthread_mutex_unlock(&lock);
	pthread_cond_broadcast(&condition);
	producerStarted=0;
	pthread_exit(NULL);

}

void *consumer(void *threadID){

	pthread_mutex_lock(&lock);	
	int tid;
	tid = (int)threadID;
	while(producerStarted==0&&cb->count==0){
		if(pthread_cond_wait(&condition,&lock) != 0){
			printf("ERROR: Wait failed!\n");
			pthread_exit(tid);
		}
	}
	consumerStarted=1;
	int * buffy=(int*)malloc(sizeof(int));
	for(int p=0;p<numofrandoms;p++){			
		if(cb->count==0){
			pthread_cond_signal(&condition);
			if(pthread_cond_wait(&condition,&lock) != 0){
					printf("ERROR: Wait failed!\n");
	
			}	
		}	
			
		if(cb->count!=0){
			cb_pop_front(cb,buffy);
			FILE * fl = fopen("prod_out.txt", "a");	
			if(fl==NULL) exit(1);
			fprintf(fl,"Consumer %i : %i\n",tid,*buffy);
			fclose(fl);
			printf("CONSUMER %i: %i\n",tid,*buffy);

		}	
	}

	pthread_mutex_unlock(&lock);	
	pthread_cond_broadcast(&condition);
	pthread_exit(NULL);
}


