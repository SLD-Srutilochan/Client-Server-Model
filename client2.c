#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER "127.0.0.1"
#define PORT 8080

// Function to send file to server
void send_file(SOCKET socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File opening failed");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size to server
    send(socket, (char *)&file_size, sizeof(file_size), 0);

    char buffer[1024];
    while (fread(buffer, sizeof(char), sizeof(buffer), file)) {
        send(socket, buffer, sizeof(buffer), 0);
    }

    fclose(file);
    printf("File data sent successfully.\n");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connection failed. Error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to server.\n");

    // User menu
    int choice;
    printf("Enter the file name to send: ");
    char filename[256];
    scanf("%s", filename);

    // Send file to server
    send_file(sock, filename);

    closesocket(sock);
    WSACleanup();
    return 0;
}
