#include "../utils/utils.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

struct server {
  int socket;
  struct sockaddr_in address;
};


void start_server(struct server* server_info) {
  server_info->socket = socket(AF_INET, SOCK_STREAM, 0);
  server_info->address.sin_family = AF_INET;
  server_info->address.sin_port = htons(PORT);
  server_info->address.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (server_info->socket == -1) {
    write_error("couldn't create socket", 1);
  }
  printf("socket created\n");
  if (bind(server_info->socket, (struct sockaddr*)&server_info->address, sizeof((server_info->address))) != 0) {
    write_error("binding failed", 1);
  }
  printf("binded\n");
  if (listen(server_info->socket, 10) != 0) {
    write_error("listen failed", 1);
  }
  printf("listening\n");
}


void recieve_file(int csocket, FILE* write_file, char* buffer) {
  int bytes_recieved = 0;
  while((bytes_recieved = recv(csocket, buffer, BUFFER_SIZE, 0)) != 0) {
    if (bytes_recieved == -1) {
      write_error("recv failed", 0);
      return;
    } printf("just recieved bytes: %d\n", bytes_recieved / 4);

    for (int i = 0; i < bytes_recieved; i += 4) {
      int byte = 0;
      for (int b = 0; b < 4; b++) {
	*((unsigned char*)&byte + b) = buffer[b+i];
      }
      fputc(byte, write_file);
    }
  }
  printf("file recieved\n");
}


void upload_request(int csocket, char* buffer) {
  int ires = 0;
  char* file_name = malloc(NAME_MAXLEN);
  ires = recv(csocket, buffer, BUFFER_SIZE, 0);
  if (ires == -1) {
    write_error("recv failed", 0);
    goto cleanup;
  }
  if (ires >= NAME_MAXLEN) {
    write_error("name longer than maximum length", 0);
    goto cleanup;
  }
  
  strcpy(file_name, buffer);
  FILE* write_file = fopen("try.txt", "w");
  printf("file name recieved: %s\n", file_name);

  recieve_file(csocket, write_file, buffer);
  
 cleanup:
  shutdown(csocket, SHUT_RDWR);
  free(file_name);
  fclose(write_file);
}


void download_request(int client_socket, char* buffer) {
  
}


void handle_connections(struct server* server_info, char* buffer) {
  while(1) {
    int client_socket = accept(server_info->socket, NULL, NULL);
    if (client_socket == -1) {
      write_error("accept failed", 0); 
    }
    else {
      printf("accepted client socket\n");
      int recieved = recv(client_socket, buffer, BUFFER_SIZE, 0);
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
}


int main() {
  struct server server_info;
  start_server(&server_info);
  char* buffer = malloc(sizeof(int) * BUFFER_SIZE);

  handle_connections(&server_info, buffer);
  return 0;
}
