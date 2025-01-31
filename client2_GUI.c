#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>

#define MAX_PATH 260

// Function to download a file from the server
void download_file(SOCKET socket, const char *filename) {
    // Request download by sending the "DOWNLOAD" command
    send(socket, "DOWNLOAD", strlen("DOWNLOAD") + 1, 0);  // Send request type

    // Send the filename of the file to download
    send(socket, filename, strlen(filename) + 1, 0);

    // Receive the file size from the server
    long file_size;
    recv(socket, (char *)&file_size, sizeof(file_size), 0);  // Receive file size

    // Handle error if file not found (received -1)
    if (file_size == -1) {
        MessageBox(NULL, "File not found on the server.", "Error", MB_ICONERROR);
        return;
    }

    // Open the file for saving
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        MessageBox(NULL, "Failed to open file for saving", "Error", MB_ICONERROR);
        return;
    }

    // Receive the file content in chunks
    char buffer[1024];
    long bytes_received = 0;
    while (bytes_received < file_size) {
        int bytes = recv(socket, buffer, sizeof(buffer), 0);  // Receive file data
        if (bytes <= 0) {
            break;
        }
        fwrite(buffer, sizeof(char), bytes, file);
        bytes_received += bytes;
    }

    fclose(file);
}

// Client button handler for downloading files
void on_download_button_click(HWND hwnd, SOCKET client_sock) {
    // Open file dialog to select a file to download
    OPENFILENAME ofn;
    char filename[MAX_PATH] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';

    if (GetOpenFileName(&ofn) == TRUE) {
        // Call the download function
        download_file(client_sock, filename);
    }
}

// Window procedure to handle button clicks
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static SOCKET client_sock;

    switch (uMsg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 2:  // Download button ID
                    on_download_button_click(hwnd, client_sock);
                    break;
                // Handle other cases if necessary (e.g., upload)
                default:
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
        } break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Main function to initialize the client window and socket
int main() {
    const char *class_name = "FileTransferClient";
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = class_name;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, class_name, "File Transfer Client", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, wc.hInstance, NULL);

    CreateWindow("BUTTON", "Download File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 50, 50, 100, 30, hwnd, (HMENU)2, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Initialize socket
    WSADATA wsaData;
    SOCKET client_sock;
    struct sockaddr_in server_addr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Server port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Server address

    connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(client_sock);
    WSACleanup();
    return 0;
}
