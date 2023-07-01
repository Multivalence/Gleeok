/* client process */

/* include the necessary header files */
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SIZE sizeof(struct sockaddr_in)

char* get_clipboard_text() {
    system("pbpaste > tmp.txt");

    FILE* file = fopen("tmp.txt", "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
    }
    
    fclose(file);
    return buffer;
}

int main (int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP address> <Port>\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int sockfd;

    char *copied_data = NULL;

   /* initialize the internet socket with a port number of `port`
       and the address specified in ip_address */
    struct sockaddr_in server;


    server.sin_family=AF_INET;
    /* convert and store the server's IP Address */
    server.sin_addr.s_addr = inet_addr (ip_address);
    /* convert and store the port */
    server.sin_port = htons(port);

    /* set up the transport end point */
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket call failed");
        exit (1);
    }


    /* connect the socket to the server's address */
    if (connect (sockfd, (struct sockaddr *) &server, SIZE) == -1) {
        perror ("connect call failed");
        exit (1);
    }

    /* send information to the server */
    for (;;) {

        char *new_data = get_clipboard_text();

        if (copied_data == NULL || (new_data != NULL && strcmp(new_data, copied_data) != 0)) {
            printf("\nTransmitting Data to Server: %s\n", new_data);
            ssize_t bytes_sent = send(sockfd, new_data, strlen(new_data), 0);
            if (bytes_sent == -1) {
                perror("send failed");
                exit(1);
            }
            free(copied_data);
            copied_data = strdup(new_data);
        }
        sleep(1);
    }
}
