#ifdef _WIN32
	char full_direct[100] = ".\\db.txt";
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
char full_direct[100] = "./db.txt";
#endif
#pragma warning(disable : 4996)
#define HAVE_STRUCT_TIMESPEC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
int r;
int w;
int r_delay;
int w_delay;
int starter = 0;
int* writer_thread_id;
int* reader_thread_id;
char complete_string[100] = "";
char line[100];
sem_t rw;//between reads and writes
sem_t ww;//between writes and writes
FILE *filedest;
FILE *fileread;
pthread_t main_t;
pthread_attr_t attr;
struct timeb timestruct;
char *Read(){
	if(starter == 0){
	fseek(filedest, 0, SEEK_SET);
	}
	if (fgets(line, sizeof line, fileread) == NULL) {
		#ifdef _WIN32
		Sleep(r_delay * 1000);
		#else
		sleep(r_delay);
		#endif
	}
	if (fgets(line, sizeof line, fileread) != NULL){
		//puts(line);
		//printf("%s", line);
	}
	//printf("%s \n", line);
	starter = 1;
return (line);
}
char *Write(){
	memset(complete_string, 0, sizeof complete_string);
	fseek(filedest, 0, SEEK_END);
	ftime(&timestruct);
	int secs;
	char str[100];
	char str_milli[100];
	int msecs;
	secs=(int)timestruct.time;
	msecs=(int)timestruct.millitm;
	sprintf(str, "%d", secs);
	sprintf(str_milli, "%d", msecs);
	char *newstring = str + 7;
	
	strcat(complete_string, newstring);
	strcat(complete_string, ":");
	strcat(complete_string, str_milli);
	strcat(complete_string, "\n");
	//printf("%s", complete_string);
	fputs(complete_string, filedest);
	return (complete_string);
}
void *read_threads(void *reader_thread_id){
	int j;
	int *my_read_thread_id = (int *)reader_thread_id;
	for(j=0; j < 10; j++){
		Read();
		char *q = line;
		q[strlen(q)-1] = 0;
		printf("DB value read =: %s by reader number: %d \n", q, *my_read_thread_id);
#ifdef _WIN32
		Sleep(r_delay*1000);
#else
		sleep(r_delay);
#endif
	}
}
void *write_threads(void *writer_thread_id){
	int i;
	int *my_write_thread_id = (int *)writer_thread_id;
	for(i=0; i < 10; i++){
		sem_post(&ww);
		Write();
		sem_wait(&ww);
		char *p = complete_string;
		p[strlen(p)-1] = 0;
		printf("DB value set to %s by writer number: %d \n", p, *my_write_thread_id);
#ifdef _WIN32
		Sleep(w_delay*1000);
#else
		sleep(w_delay);
#endif
	}
}
void *master_read(){
	int num_r_threads;
	pthread_t *reader_t = (pthread_t*)malloc(sizeof(pthread_t)*r);
	reader_thread_id = malloc(sizeof(int)*r);
	for(num_r_threads = 0; num_r_threads < r; num_r_threads++){
		//sem_post(&rw);
		reader_thread_id[num_r_threads] = num_r_threads;
		pthread_create(&reader_t[num_r_threads], NULL, read_threads, (void *)&reader_thread_id[num_r_threads]);
		//printf("Read Thread %d was created \n", num_r_threads);
		//sem_wait(&rw);
		pthread_join(reader_t[num_r_threads], NULL);
	}
}
void *master_write(){
	int num_w_threads;
	pthread_t *writer_t = (pthread_t*)malloc(sizeof(pthread_t)*w);
	writer_thread_id = malloc(sizeof(int)*w);
	for(num_w_threads = 0; num_w_threads < w; num_w_threads++){
		writer_thread_id[num_w_threads] = num_w_threads;
		pthread_create(&writer_t[num_w_threads], NULL, write_threads, (void *)&writer_thread_id[num_w_threads]);
		pthread_join(writer_t[num_w_threads], NULL);
	}
}

void *mainthread(){
	printf("Enter the number of readers: ");
	scanf("%d",&r);
	printf("Enter the number of writers: ");
	scanf("%d",&w);
	printf("Enter the delay for a reader to restart: ");
	scanf("%d",&r_delay);
	printf("Enter the delay for a writer to restart: ");
	scanf("%d",&w_delay);
	sem_init(&ww, 0, 0);
	sem_init(&rw, 0, 0);
	filedest = fopen(full_direct, "w+");
	fileread = fopen(full_direct, "r+");
	pthread_t mast_read;
	pthread_t mast_write;
	pthread_create(&mast_write, NULL, master_write, NULL);
	pthread_create(&mast_read, NULL, master_read, NULL);
	pthread_join(mast_write, NULL);
	pthread_join(mast_read, NULL);
	printf("The Complete Database: \n");
	fseek(filedest, 0, SEEK_SET);
	while (fgets(line, sizeof line, filedest) != NULL) {
		printf("%s", line);
	}
	#ifdef _WIN32
	getch();
	#endif	
}
int main(){
	pthread_create(&main_t, NULL, mainthread, NULL);
	pthread_join(main_t, NULL);
return 0;
}
