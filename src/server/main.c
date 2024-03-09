#include "../utils/utils.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

const int max_name_length = 255;
const int port = 56821;
const int buffer_size = 26400;
const char* IP = "192.168.1.13";

struct server {
  int socket;
  struct sockaddr_in address;
};

void start_server(struct server* server_info) {
  server_info->address.sin_family = AF_INET;
  server_info->address.sin_port = htons(port);
  server_info->address.sin_addr.s_addr = inet_addr(IP);
  server_info->socket = socket(AF_INET, SOCK_STREAM, 0);
  
  if (server_info->socket == -1) {
    write_error("couldn't create socket", 1);
  }
  printf("socket created\n");
  if (bind(server_info->socket, (struct sockaddr*)&server_info->address, sizeof(server_info->address)) != 0) {
    write_error("binding failed", 1);
  }
  printf("binded\n");
  if (listen(server_info->socket, 10) != 0) {
    write_error("listen failed", 1);
  }
  printf("listening\n");
}

int upload_request(int client_socket, char* buffer) {
  if (recv(client_socket, buffer, buffer_size, 0) == -1) {
    write_error("recv failed", 0);
  }
  if (strlen(buffer) >= max_name_length) {
    shutdown(client_socket, SHUT_RDWR);
    return -1;
  }
  const char* file_name = buffer;
  FILE* write_file = fopen(buffer, "w");

  int recieved = 0;
  while((recieved = recv(client_socket, buffer, buffer_size, 0)) != 0) {
    if (recieved == -1) {
      write_error("recv failed", 0);
    }
    for(int i = 0; i < recieved; fputc(buffer[i], write_file), ++i)
      ;
  }
}

int download_request(int client_socket, char* buffer) {
  
}

int main() {
  struct server server_info;
  start_server(&server_info);
  unsigned char* buffer = malloc(sizeof(char) * buffer_size);
    
  while(1) {
    int client_socket = accept(server_info.socket, (struct sockaddr*)&server_info.address, NULL);
    if (client_socket == -1) {
      write_error("accept failed", 0); 
    }
    else {
      printf("accepted client socket\n");
      int recieved = recv(client_socket, buffer, buffer_size, 0);
      if (recieved == -1) {
	write_error("recv failed", 0);
      }
      else if(strcmp(buffer, "upload_file_request") == 0) {
	printf("upload requested\n");
	upload_request(client_socket, buffer);
      }
      else if (strcmp(buffer, "download_file_request") == 0) {
	printf("download requested\n");
	download_request(client_socket, buffer);
      }
    }
  }
  return 0;
}
