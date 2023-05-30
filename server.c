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



     } else if (strncmp(buffer, "/msg", 4) == 0) {





           // Komanda '/msg'



           printf("[%s] Klienti %d - Mesazhi privat\n", timestamp, client_id+1);



            fflush(stdout);

            char* recipient = strtok(buffer, " ");

            char* message = strtok(NULL, "");





            sem_wait(&shared_memory->client_count_sem);





            int recipient_id = atoi(recipient + 1);





            if (recipient_id >= 0 && recipient_id < shared_memory->client_count) {

               



                Message msg;



                msg.mtype = recipient_id + 1; // Set message type to recipient's client ID

                strncpy(msg.mtext, message, BUFFER_SIZE);

                if (msgsnd(shared_memory->message_queue, &msg, sizeof(msg.mtext), 0) < 0) {



                    perror("Ka ndodhur nje gabim gjate dergimit te mesazhit privat");



                    exit(1);

                }



            } else {



                printf("ID jo valide: %d\n", recipient_id);



                fflush(stdout);



            }



            sem_post(&shared_memory->client_count_sem);



        } else if (strncmp(buffer, "/quit", 5) == 0) {





            //Komanda '/quit'





            printf("[%s] Klienti %d - u shkyc\n", timestamp, client_id+1);



            fflush(stdout);









           sem_wait(&shared_memory->client_count_sem);



            close(client_socket);





            if (client_id < shared_memory->client_count - 1) {



                shared_memory->client_info[client_id] = shared_memory->client_info[shared_memory->client_count - 1];



                shared_memory->thread_ids[client_id] = shared_memory->thread_ids[shared_memory->client_count - 1];

            }



            shared_memory->client_count--;

            sem_post(&shared_memory->client_count_sem);





            break;



        } else {





                printf("[%s] Klienti %d - Mesazhi: %s\n", timestamp, client_id+1, buffer);



                fflush(stdout);



               sem_wait(&shared_memory->client_count_sem);



               for (int i = 0; i < shared_memory->client_count; i++) {

 

                   if (shared_memory->client_info[i].socket != client_socket) {



                    char message_with_sender[2 * BUFFER_SIZE];



                   snprintf(message_with_sender, sizeof(message_with_sender), "Client %d: %s", client_id+1, buffer);







                  if (send(shared_memory->client_info[i].socket, message_with_sender, strlen(message_with_sender), 0) < 0) {

   

                   perror("Ka ndodhur nje gabim gjate dergimit te mesazheve te klienti");



                    exit(1);

 

        }



    }



}

            sem_post(&shared_memory->client_count_sem);

      }



        memset(buffer, 0, sizeof(buffer));

    }

    free(arguments);

    pthread_exit(NULL);




}
int main() {



    int server_socket;



    int client_socket;



    int client_count = 0;



    struct sockaddr_in server_address, client_address;







    pthread_t thread_ids[MAX_CLIENTS];







    sem_t client_count_sem;



    SharedMemory* shared_memory;







    int shared_memory_id;



    shared_memory_id = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);



    if (shared_memory_id < 0) {



    perror("Ka ndodhur nje gabim gjate krijimit te shared memory segment");



    exit(1);



 }



    shared_memory = (SharedMemory*)shmat(shared_memory_id, NULL, 0);







    if (shared_memory == (void*)-1) {



        perror("Ka ndodhur nje gabim gjate bashkangjitjes se shared memory segment");



       exit(1);



    }

    shared_memory->message_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0666);



    if (shared_memory->message_queue < 0) {



        perror("Ka ndodhur nje gabim ne krijimin e message queue");



        exit(1);



    }

    shared_memory->client_count = 0;



    sem_init(&shared_memory->client_count_sem, 1, 1);



    server_socket = socket(AF_INET, SOCK_STREAM, 0);



    if (server_socket < 0) {



        perror("Ka ndodhur gabim ne krijimin e server socket");



       exit(1);



    }



    int opt = 1;



    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {



           perror("Ka ndodhur nje gabim gjate vendosjes se opsioneve te server socket");



        exit(1);



    }



    memset((char*)&server_address, 0, sizeof(server_address));



    server_address.sin_family = AF_INET;



    server_address.sin_addr.s_addr = INADDR_ANY;



    server_address.sin_port = htons(8888);



    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {



        perror("Ka ndodhur nje gabim ne lidhjen e server socket");

  exit(1);



    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {



        perror("Ka ndodhur nje gabim gjate lidhjes se klienteve");



        exit(1);

}



    printf("Serveri ka filluar, duke pritur per lidhje te klienteve...\n");





    int address_length = sizeof(struct sockaddr_in);



    while (1) {



        client_socket = accept(server_socket, (struct sockaddr*)&client_address, (socklen_t*)&address_length);



        if (client_socket < 0) {

  perror("Ka ndodhur nje gabim gjate pranimit te lidhjes se klienteve");



            exit(1);



        }



        sem_wait(&shared_memory->client_count_sem);



        shared_memory->client_info[shared_memory->client_count].socket = client_socket;



        shared_memory->client_info[shared_memory->client_count].address = client_address;



        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));



        args->client_socket = client_socket;



       args->client_id = client_id++;



        args->shared_memory = shared_memory;



        if (pthread_create(&shared_memory->thread_ids[shared_memory->client_count], NULL, (void*)handle_client, (void*)args) != 0) {



            perror("Ka ndodhur nje gabim gjate krijimit te thread - it per klient");



            exit(1);

        }

        printf("Thread eshte krijuar per klientin %d: %lu\n", client_id, shared_memory->thread_ids[shared_memory->client_count]);

        shared_memory->client_count++;

        sem_post(&shared_memory->client_count_sem);

    }

    shmdt(shared_memory);

    shmctl(shared_memory_id, IPC_RMID, NULL);

    close(server_socket);



    return 0;

}



