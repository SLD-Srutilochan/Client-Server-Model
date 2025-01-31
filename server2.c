#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#define MAX_PATH 260

// Function to handle both uploading and downloading files based on the command
void handle_file_request(SOCKET client_sock) {
    char request_type[10];
    int received = recv(client_sock, request_type, sizeof(request_type), 0);
    if (received <= 0) {
        printf("Failed to receive request type\n");
        return;
    }

    if (strcmp(request_type, "DOWNLOAD") == 0) {
        // Receive the filename from the client
        char file_name[MAX_PATH];
        recv(client_sock, file_name, sizeof(file_name), 0);

        // Open the file for reading (binary mode)
        FILE *file = fopen(file_name, "rb");

        if (file == NULL) {
            // Send error message if the file does not exist
            printf("File not found\n");
            send(client_sock, "ERROR", 6, 0);  // Notify client of error
            return;
        }

        // Send file size to the client
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        send(client_sock, (char *)&file_size, sizeof(file_size), 0);

        // Send the file contents in chunks
        char buffer[1024];
        int bytes_read;
        while ((bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
            send(client_sock, buffer, bytes_read, 0);
        }

        fclose(file);
    }

    // Handle other request types (e.g., upload) if necessary
}

// Server function to listen for incoming connections
void start_server() {
    WSADATA wsaData;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_size = sizeof(client_addr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port number
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 3);

    printf("Server is listening on port 8080...\n");

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_size);
        printf("Client connected...\n");

        handle_file_request(client_sock);
        closesocket(client_sock);
    }

    closesocket(server_sock);
    WSACleanup();
}

int main() {
    start_server();
    return 0;
}
