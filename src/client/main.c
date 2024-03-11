#include "../utils/utils.h"
#include "../transfer/transfer.h"
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
  int ires = 0;
  if (!write_state(csocket != -1, "socket created", "socket creation failed")) {
    return -1;
  }
  ires = connect(csocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0;
  if (!write_state(csocket != -1, "socket connected", "connection to server failed")) {
    csocket = -1;
  }
  return csocket;
}


int send_request(int rq, int client_socket) {
  int ires = 0;
  char request_str[50] = {'\0'};
  switch(rq) {
  case REQUEST_UPLOAD: strcpy(request_str, "upload_file_request"); break;
  case REQUEST_DOWNLOAD: strcpy(request_str, "download_file_request"); break;
  default: printf("request token not implemented\n"); return 0;
  }

  ires = send(client_socket, request_str, strlen(request_str) + 1, 0);
  return write_state(ires != -1, "request sent", "request sending failed");
}


void upload(char* file_src, unsigned char* buffer) {
  int csocket = make_connected_socket();
  FILE* file = fopen(file_src, "r"); 
  char* file_name = get_file_name(file_src);
  int ires = 0;
  if (csocket == -1) {
    goto cleanup; // sorry.
  }
  if (!write_state(file != NULL, "file opened", "file open failed")) {
    goto cleanup;
  }
  if (send_request(REQUEST_UPLOAD, csocket) == 0) {
    goto cleanup;
  }
  // send name first
  ires = send(csocket, file_name, strlen(file_name) + 1, 0);
  if (!write_state(ires != -1, "name sent", "name sending failed")) {
    goto cleanup;
  }

  // now send file
  send_file(csocket, file, buffer);
 cleanup:
  close(csocket);
  fclose(file);
  free(file_name);
}


int file_exists(const char* file_name) {
  return fopen(file_name, "r") != NULL;
}

int ask_overwrite() {
  char answer[3];
  printf("this file exists, do you want to overwrite it? [yes] or [no]:");
  while(1) {
    scanf("%s", answer);
    if (strcmp(answer, "yes") == 0) {
      return 1;
    }
    else if (strcmp(answer, "no") == 0) {
      return 0;
      break;
    }
    else {
      printf("(please answer with [yes] or [no])\n");
    }
  }
}

void download(const char* file_hash, unsigned char* buffer) {
  int ires = 0;
  int csocket = make_connected_socket();
  char file_name[NAME_MAXLEN] = {'\0'};
  if(csocket == -1) {
    return;
  }
  ires = send(csocket, file_hash, strlen(file_hash) + 1, 0);
  if(!write_state(ires != -1, "hash sent", "send hash failed")) {
    goto cleanup;
  }
  ires = recv(csocket, buffer, BUFFER_SIZE, 0);
  if(!write_state(ires != -1, "name recieved", "recv name failed")) {
    goto cleanup;
  }
  strcpy(file_name, buffer);
  // shouldn't happen, but just in case
  if(strlen(file_name) >= NAME_MAXLEN) {
    printf("name length higher than max\n");
    goto cleanup;
  }
  printf("file name: %s\n", file_name);
  if(file_exists(file_name) && !ask_overwrite()) {
    goto cleanup;
  }
  else {
    FILE* write_file = fopen(file_name, "w");
    recieve_file(csocket, write_file, buffer);
    fclose(write_file);
  }
 cleanup:
  close(csocket);
}


int main() {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(IP);
  unsigned char* buffer = malloc(sizeof(int) * BUFFER_SIZE);

  
  free(buffer);
  return 0;
}
