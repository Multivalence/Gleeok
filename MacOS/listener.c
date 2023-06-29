/* server process */

/* include the necessary header files */
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SIZE sizeof(struct sockaddr_in)
#define MAX_CLIENTS 10

typedef struct {
    int sockfd;
} Client;

int newsockfd;

int set_clipboard_text(char *text) {
    FILE* file = fopen("tmp2.txt", "w"); // Open a temporary file to write
    if (!file) return 0; // If file could not be opened, return FALSE

    fprintf(file, "%s", text); // Write the text to file
    fclose(file); // Close the file

    system("cat tmp2.txt | pbcopy"); // pbcopy command takes the file content into clipboard
    system("rm tmp2.txt"); // Remove the temporary file

    return 1;
}


void handleClientMessage(Client *clients, int index, fd_set *active_sockets) {

    char buffer[1024];

    int len = recv(clients[index].sockfd, buffer, 1024, 0);

    if (len == -1) {
        perror("recv");
    }

    // Socket closed
    else if (len == 0) {
        close(clients[index].sockfd);
        FD_CLR(clients[index].sockfd, active_sockets);
        clients[index].sockfd = -1;
    }

    else {
        buffer[len] = '\0';
        printf("Incoming Copy: %s", buffer);

        if (!set_clipboard_text(buffer)){
            fprintf(stderr, "Failed to set clipboard text.\n");
        }
    }

    

}

int main (int argc, char ** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd;
    char c;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket call failed");
        exit (1);
    }

    /* initialize the internet socket with a port number of 8301
       and the local address,specified as INADDR_ANY 
	See: /usr/include/netinet/in.h
       */
    struct sockaddr_in server;
    server.sin_family=AF_INET;          // IPv4 address
    server.sin_addr.s_addr=INADDR_ANY;  // Allow use of any interface 
    server.sin_port = htons(port);      // specify port

    /* "bind server address to the end point */
    if (bind (sockfd, (struct sockaddr *) &server, SIZE) == -1) {
        perror ("bind call failed");
        exit (1);
    }

    /* start listening for incoming connections */
    if (listen (sockfd, 5) == -1) {
        perror ("listen call failed");
        exit (1);
    }

    Client clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = -1;
    }

    fd_set active_sockets, read_fds;
    FD_ZERO(&active_sockets);
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &active_sockets);

    int max_fd = sockfd;

    for (;;) {

        read_fds = active_sockets;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                
                if (i == sockfd) {
                    
                    // New Connection

                    /*accept a connection */
                    if ((newsockfd = accept (sockfd, NULL, NULL)) == -1) {
                        perror ("accept call failed");
                        continue;
                    }

                    int index = -1;

                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].sockfd == -1) {
                            index = j;
                            break;
                        }
                    }

                    if (index == -1) {
                        close(newsockfd);
                        continue;
                    }

                    clients[index].sockfd = newsockfd;

                    FD_SET(newsockfd, &active_sockets);

                    if (newsockfd > max_fd) {
                        max_fd = newsockfd;
                    }
                    
                }

                else {
                    // Existing Connection

                    for (int j = 0; j < MAX_CLIENTS; j++) {

                        if (clients[j].sockfd == i) {
                            handleClientMessage(clients, j, &active_sockets);
                            break;
                        }

                    }
                }

            }
        }
    }
}
