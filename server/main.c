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

void start(struct sockaddr_in* server_addr, int* server_socket) {
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(port);
  server_addr->sin_addr.s_addr = inet_addr(IP);
  *server_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  if (*server_socket == -1) {
    write_error("couldn't create socket", true);
  }
  printf("socket created\n");
  if (bind(*server_socket, (struct sockaddr*)server_addr, sizeof(*server_addr)) != 0) {
    write_error("binding failed", true);
  }
  printf("binded\n");
  if (listen(*server_socket, 10) != 0) {
    write_error("listen failed", true);
  }
  printf("listening\n");
}

int main() {
  struct sockaddr_in server_addr;
  int server_socket = 0;
  start(&server_addr, &server_socket);
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
