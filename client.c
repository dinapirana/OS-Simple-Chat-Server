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
