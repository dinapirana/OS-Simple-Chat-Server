#include <stdio.h>

#include <stdlib.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <pthread.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include <semaphore.h>

#include <sys/time.h>

#include <sys/msg.h>

#define MAX_CLIENTS 10

#define BUFFER_SIZE 1024



typedef struct {



    long mtype;



    char mtext[BUFFER_SIZE];



} Message;



typedef struct {



    int client_socket;



    int client_id;



    struct SharedMemory* shared_memory;



} ThreadArgs;



typedef struct {



    int socket;



    struct sockaddr_in address;







} ClientInfo;



typedef struct SharedMemory {



    int client_count;



    ClientInfo client_info[MAX_CLIENTS];



    pthread_t thread_ids[MAX_CLIENTS];



    sem_t client_count_sem;



   char message_buffer[BUFFER_SIZE];



   int message_queue;



    int server_socket;



} SharedMemory;



int client_id = 0;



void handle_client(void* arguments) {



    ThreadArgs* args = (ThreadArgs*)arguments;



    int client_socket = args->client_socket;



   int client_id = args->client_id;



       SharedMemory* shared_memory = args->shared_memory;



     char buffer[BUFFER_SIZE];



     int read_size;



      while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {



         buffer[read_size] = '\0';



         struct timeval tv;



        gettimeofday(&tv, NULL);



        char timestamp[30];



        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));



        if (strncmp(buffer, "/list", 5) == 0) {



          // Komanda l'/ist'



          printf("[%s] Klienti %d - Kerkesa per komanden '/list'\n", timestamp, client_id+1);



           fflush(stdout);



           sem_wait(&shared_memory->client_count_sem);



           printf("Klientet e kycur: %d\n", shared_memory->client_count);



           for (int i = 0; i < shared_memory->client_count; i++) {





           printf("Klienti %d: %s\n", i+1, inet_ntoa(shared_memory->client_info[i].address.sin_addr));

}

         fflush(stdout);



            sem_post(&shared_memory->client_count_sem);



     }
