#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>  // For socket operations on Windows
#define SIZE 1024

void write_file(int sockfd) {
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[SIZE];

    // Open file to write
    fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("[-]Error in opening file for writing.");
        return;
    }

    // Receive and write data to the file
    while (1) {
        n = recv(sockfd, buffer, SIZE, 0);
        if (n <= 0) {
            break;  // Exit if no data is received or if connection is closed
        }
        fprintf(fp, "%s", buffer);  // Write received data to the file
        printf("[+]Data received: %s\n", buffer);  // Debugging: Print received data
        memset(buffer, 0, SIZE);  // Clear the buffer
    }

    fclose(fp);  // Close the file after writing
    printf("[+]Data written to the file successfully.\n");
}

int main() {
    char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    WSADATA wsa;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    int addr_size;  // Changed socklen_t to int
    char buffer[SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+]Winsock initialized.\n");

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Error in socket: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+]Server socket created successfully.\n");

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Bind the socket to the address and port
    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e == SOCKET_ERROR) {
        printf("Error in bind: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+]Binding successful.\n");

    // Start listening for incoming connections
    if (listen(sockfd, 10) == SOCKET_ERROR) {
        printf("Error in listening: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+]Listening....\n");

    // Accept the incoming connection
    addr_size = sizeof(new_addr);
    new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
    if (new_sock == INVALID_SOCKET) {
        printf("Error in accept: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+]Connection accepted.\n");

    // Write data received from client to file
    write_file(new_sock);

    // Close the connection and cleanup Winsock
    closesocket(new_sock);
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
