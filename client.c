#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#define SIZE 1024

void send_file(FILE *fp, SOCKET sockfd){
  int n;
  char data[SIZE] = {0};

  while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == SOCKET_ERROR) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    memset(data, 0, SIZE);
  }
}

int main(){
  char *ip = "127.0.0.1";
  int port = 8080;
  int e;

  SOCKET sockfd;
  struct sockaddr_in server_addr;
  FILE *fp;
  char *filename = "send.txt";

  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa); // Initialize Winsock

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == INVALID_SOCKET) {
    perror("[-]Error in socket");
    WSACleanup();
    exit(1);
  }
  printf("[+]Server socket created successfully.\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);

  e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(e == SOCKET_ERROR) {
    perror("[-]Error in connecting to server");
    closesocket(sockfd);
    WSACleanup();
    exit(1);
  }
  printf("[+]Connected to Server.\n");

  fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    closesocket(sockfd);
    WSACleanup();
    exit(1);
  }

  send_file(fp, sockfd);
  printf("[+]File data sent successfully.\n");

  printf("[+]Closing the connection.\n");
  fclose(fp);
  closesocket(sockfd);
  WSACleanup();

  return 0;
}
