#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <pthread.h>



#define MAX_BUFFER_SIZE 1024



void* receive_messages(void* socket_desc) {

    int socket = *(int*)socket_desc;

    char buffer[MAX_BUFFER_SIZE];

    int read_size;



    while ((read_size = recv(socket, buffer, MAX_BUFFER_SIZE, 0)) > 0) {

        buffer[read_size] = '\0';

        printf("Mesazhi i marre:  %s\n", buffer);

        fflush(stdout);

        memset(buffer, 0, sizeof(buffer));

    }



    if (read_size == 0) {

        printf("Lidhja u shkeput.\n");

      //  fflush(stdout);

    } else if (read_size == -1) {

        perror("Ka ndodhur nje gabim gjate marrjes se mesazheve");

        exit(1);

    }



    close(socket);

  //  free(socket_desc);

    pthread_exit(NULL);

}
int main() {

    int client_socket;

    struct sockaddr_in server_address;

    pthread_t receive_thread;



    // Krijimi i client socket

   

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {

        perror("Ka ndodhur nje gabim gjate krijimit te client socket");

        exit(1);

    }



   

    memset((char*)&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;

    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_address.sin_port = htons(8888);



    // Lidhu me serverin

   

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {

        perror("Ka ndodhur nje gabim gjate lidhjes me server");

        exit(1);

    }



 

    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_socket) != 0) {

        perror("Ka ndodhur nje gabim gjate marrjes se thread - ave ");

        exit(1);

    }



   

    char message[MAX_BUFFER_SIZE];

    while (1) {

        fgets(message, MAX_BUFFER_SIZE, stdin);



       

        size_t length = strlen(message);

        if (length > 0 && message[length - 1] == '\n') {

            message[length - 1] = '\0';

        }



      // Komanda '/quit' nese klienti do te shkycet

     

        if (strcmp(message, "/quit") == 0) {

            if (send(client_socket, message, strlen(message), 0) < 0) {

                perror("Ka ndodhur nje gabim gjate dergimit te komandes ne server");

                exit(1);

            }

            break;

        }



       

        if (send(client_socket, message, strlen(message), 0) < 0) {

            perror("Ka ndodhur nje gabim gjate dergimit te mesazhit ne server");

            exit(1);

        }

    }



   

    pthread_join(receive_thread, NULL);



    return 0;

}

