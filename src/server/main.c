#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#define true 1
#define false 0 
const int port = 12223;
const int buffer_size = 26400;
const char* IP = "192.168.1.13";

void write_error(char* message, char do_exit) {
  printf("[error]: %s, errno: %s", message, strerror(errno));
  if (do_exit) {
    exit(1);
  }
}


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
    write_error("couldn't create socket", true);
  }
  printf("socket created\n");
  if (bind(server_info->socket, (struct sockaddr*)&(server_info->address), sizeof(server_info-address)) != 0) {
    write_error("binding failed", true);
  }
  printf("binded\n");
  if (listen(server_info->socket, 10) != 0) {
    write_error("listen failed", true);
  }
  printf("listening\n");
}

int main() {
  struct server server_info;
  start_server(&server_info);
  char* recv_buffer = malloc(sizeof(char) * buffer_size);
    
  while(true) {
    int client_socket = accept(server_socket, (struct sockaddr*)&server_addr, NULL);
    if (client_socket == -1) {
      write_error("accept failed", true); 
    }
    else {
      int recieved = recv(client_socket, recv_buffer, buffer_size, 0);
      if (recieved == -1) {
	write_error("recv failed", false);
      }
    }
  }
  return 0;
}
