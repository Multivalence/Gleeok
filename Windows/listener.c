/* server process */

/* include the necessary header files */
#include <ctype.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE sizeof(struct sockaddr_in)
#define MAX_CLIENTS 10

typedef struct {
    SOCKET sockfd;
} Client;

int newsockfd;

int set_clipboard_text(char *text) {
    FILE* file = fopen("tmp2.txt", "w");
    if (!file) return 0;

    fprintf(file, "%s", text);
    fclose(file);

    system("type tmp2.txt | clip"); // 'type' command reads the file content and 'clip' command copies it to the clipboard

    return 1;
}

void handleClientMessage(Client *clients, int index, fd_set *active_sockets) {
    char buffer[1024];

    int len = recv(clients[index].sockfd, buffer, 1024, 0);

    if (len == SOCKET_ERROR) {
        perror("recv");
    }
    // Socket closed
    else if (len == 0) {
        closesocket(clients[index].sockfd);
        FD_CLR(clients[index].sockfd, active_sockets);
        clients[index].sockfd = INVALID_SOCKET;
    } else {
        buffer[len] = '\0';
        printf("Incoming Copy: %s", buffer);

        if (!set_clipboard_text(buffer)) {
            fprintf(stderr, "Failed to set clipboard text.\n");
        }
    }
}

int main(int argc, char **argv) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    SOCKET sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket call failed");
        WSACleanup();
        return 1;
    }

    /* initialize the internet socket with a port number of 8301
       and the local address, specified as INADDR_ANY 
       See: /usr/include/netinet/in.h
    */
    struct sockaddr_in server;
    server.sin_family = AF_INET;          // IPv4 address
    server.sin_addr.s_addr = INADDR_ANY;  // Allow use of any interface 
    server.sin_port = htons(port);        // specify port

    /* "bind server address to the end point */
    if (bind(sockfd, (struct sockaddr *)&server, SIZE) == SOCKET_ERROR) {
        perror("bind call failed");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    /* start listening for incoming connections */
    if (listen(sockfd, 5) == SOCKET_ERROR) {
        perror("listen call failed");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    Client clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = INVALID_SOCKET;
    }

    fd_set active_sockets, read_fds;
    FD_ZERO(&active_sockets);
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &active_sockets);

    int max_fd = sockfd;

    for (;;) {
        read_fds = active_sockets;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
            perror("select");
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sockfd) {
                    // New Connection
                    /* accept a connection */
                    if ((newsockfd = accept(sockfd, NULL, NULL)) == INVALID_SOCKET) {
                        perror("accept call failed");
                        continue;
                    }

                    int index = -1;

                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].sockfd == INVALID_SOCKET) {
                            index = j;
                            break;
                        }
                    }

                    if (index == -1) {
                        closesocket(newsockfd);
                        continue;
                    }

                    clients[index].sockfd = newsockfd;

                    FD_SET(newsockfd, &active_sockets);

                    if (newsockfd > max_fd) {
                        max_fd = newsockfd;
                    }
                } else {
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

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
