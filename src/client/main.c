#include "../utils/utils.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const int name_max_length = 255;
const int port = 12223;
const int buffer_size = 26400;
const char* IP = "192.168.1.13";
struct sockaddr_in server_addr;

int make_connected_socket() {
  int csocket = socket(AF_INET, SOCK_STREAM, 0);
  if (csocket == -1) {
    write_error("couldn't create_socket", 1);
  }
  if (connect(csocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
    write_error("connection to server failed", 0);
  }
  return csocket;
}

void init_server() {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(IP);
}

int upload_file(FILE* file, const char* file_name, char* buffer) {
  if (strlen(file_name) >= name_max_length) {
    write_error("name length too long", 0);
  }
  int client_socket = make_connected_socket();
  if (send(client_socket, "upload_file_request", strlen("upload_file_request") + 1, 0) == -1) {
    write_error("send failed", 0);
    return -1;
  }
  if (send(client_socket, file_name, strlen(file_name) + 1, 0) == -1) {
    write_error("send failed", 0);
    return -1;
  }
  
  int end_reached = 0;
  while(!end_reached) {
      char byte;
      for (int i = 0; i < buffer_size; ++i) {
	if ((byte = fgetc(file)) == EOF) {
	  end_reached = 1;
	  break;
	}
	buffer[i] = byte;
      }
      if (send(client_socket, buffer, buffer_size, 0) == -1) {
	write_error("send failed", 0);
	return -1;
      }
  }
  close(client_socket);
}

int main() {
  init_server();
  unsigned char* buffer = malloc(sizeof(char) * buffer_size);
  return 0;
}
