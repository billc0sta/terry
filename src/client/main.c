#include "../utils/utils.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define IP "192.168.1.4"
#define REQUEST_UPLOAD 1
#define REQUEST_DOWNLOAD 2

struct sockaddr_in server_addr;

int make_connected_socket() {
  int csocket = socket(AF_INET, SOCK_STREAM, 0);
  if (csocket == -1) {
    write_error("couldn't create_socket", 1);
  }
  if (connect(csocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
    write_error("connection to server failed", 0);
    csocket = -1;
  }
  return csocket;
}


void init_server() {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(IP);
}


int send_request(int rq, int client_socket) {
  char request_str[50] = {'\0'};
  switch(rq) {
  case REQUEST_UPLOAD: strcpy(request_str, "upload_file_request"); break;
  case REQUEST_DOWNLOAD: strcpy(request_str, "download_file_request"); break;
  default: write_error("unimplemented request", 1);
  }
  if(send(client_socket, request_str, strlen(request_str) + 1, 0) == -1) {
    write_error("send_failed", 0);
    return 0;
  }
  return 1;
}


void send_file(int csocket, FILE* file, unsigned char* buffer) {
  int ires = 0;
  char end_reached = 0;
  int total_sent = 0;
  while(!end_reached) {
    int to_send = BUFFER_SIZE;
    for (int i = 0; i < BUFFER_SIZE; i += 4, total_sent += 1) {
      int byte = fgetc(file);
      if (byte == EOF) {
	end_reached = 1;
	to_send = i;
	break;
      }
      for(int b = 0; b < 4; ++b) {
	buffer[b+i] = *((unsigned char *)&byte + b);
      }
    }

    ires = send(csocket, buffer, to_send, 0);
    if (ires == -1) {
      write_error("send failed", 0);
      return;
    } printf("just sent bytes: %d\n", to_send / 4);
  }

  printf("uploaded, total sent bytes: %d\n", total_sent);
}


void upload_file(char* file_src, unsigned char* buffer) {
  int csocket = make_connected_socket();
  FILE* file = fopen(file_src, "r"); 
  char* file_name = get_file_name(file_src);
  int ires = 0;
  if (csocket == -1) {
    write_error("socket creation failed", 0);
    goto cleanup;
  } printf("socket created\n");
  if (file == NULL) {
    write_error("file open failed", 0);
    goto cleanup;
  } printf("file opened: %s\n", file_name);
  if (send_request(REQUEST_UPLOAD, csocket) == 0) {
    goto cleanup;
  } printf("upload request sent\n");
  // send name first
  ires = send(csocket, file_name, strlen(file_name) + 1, 0);
  if (ires == -1) {
    write_error("send failed", 0);
    goto cleanup;
  }

  send_file(csocket, file, buffer);
 cleanup:
  close(csocket);
  fclose(file);
  free(file_name);
}


int main() {
  init_server();
  unsigned char* buffer = malloc(sizeof(int) * BUFFER_SIZE);
  upload_file("example.txt", buffer);

  free(buffer);
  return 0;
}
