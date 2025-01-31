#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>  // For CreateThread and mutex operations

#define SIZE 1024

HANDLE mutex;  // Global mutex for synchronizing file access

void write_file(SOCKET sockfd) {
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[SIZE];

    WaitForSingleObject(mutex, INFINITE);  // Lock mutex before writing to the file

    // Open file to write
    fp = fopen(filename, "a");  // Use "a" mode to append to the file if it already exists
    if (fp == NULL) {
        perror("[-]Error in opening file for writing.");
        ReleaseMutex(mutex);  // Release the mutex before returning
        return;
    }

    // Receive and write data to the file
    while (1) {
        n = recv(sockfd, buffer, SIZE, 0);
        if (n <= 0) {
            break;  // Exit if no data is received or if the connection is closed
        }
        fprintf(fp, "%s", buffer);  // Write received data to the file
        printf("[+]Data received: %s\n", buffer);  // Debugging: Print received data
        memset(buffer, 0, SIZE);  // Clear the buffer
    }

    fclose(fp);  // Close the file after writing
    ReleaseMutex(mutex);  // Unlock the mutex after writing
    printf("[+]Data written to the file successfully.\n");
}

DWORD WINAPI handle_client(LPVOID client_socket) {
    SOCKET sockfd = *(SOCKET*)client_socket;
    free(client_socket);  // Free the dynamically allocated socket

    write_file(sockfd);  // Handle file writing for this client
    closesocket(sockfd);  // Close the client socket
    return 0;
}

int main() {
    char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    WSADATA wsa;
    SOCKET sockfd, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    int addr_size;

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
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("[+]Binding successful.\n");

    // Start listening for incoming connections
    if (listen(sockfd, 10) == SOCKET_ERROR) {
        printf("Error in listening: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("[+]Listening....\n");

    // Create a mutex for file access synchronization
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        printf("[-]Error creating mutex.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Accept incoming connections
    while (1) {
        addr_size = sizeof(client_addr);
        SOCKET new_sock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size);
        if (new_sock == INVALID_SOCKET) {
            printf("Error in accept: %d\n", WSAGetLastError());
            continue;
        }
        printf("[+]Connection accepted.\n");

        // Allocate memory for the socket to pass to the thread
        SOCKET *client_socket = malloc(sizeof(SOCKET));
        *client_socket = new_sock;

        // Create a thread to handle the client
        HANDLE thread = CreateThread(NULL, 0, handle_client, client_socket, 0, NULL);
        if (thread == NULL) {
            printf("[-]Error creating thread.\n");
            closesocket(new_sock);
            free(client_socket);
        } else {
            CloseHandle(thread);  // Close the handle as we don't need it
        }
    }

    // Cleanup
    CloseHandle(mutex);  // Close the mutex handle
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
