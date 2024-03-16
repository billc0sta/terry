#include "../shared/metadata/metadata.h"
#include "../shared/utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 20024
#define IP "192.168.1.6"

int make_connected_socket(struct sockaddr_in *server);
int send_request(int csockfd, const char* request, struct Buffer *buff);
void upload_file(int csockfd, const char *file_name, struct Buffer *buff);
int send_metadata(int csockfd, struct FileData *fd);
int send_file(int csockfd, FILE* file, struct Buffer *buff);

int main(int argc, char** argv) {
  printf("client side\n");

  // initalize server
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr(IP);

  int sockfd = make_connected_socket(&server);
  struct Buffer buff = make_buffer();
  if (send_request(sockfd, RQ_UPLOAD, &buff) != 1) {
    exit(1);
  }
  upload_file(sockfd, "try.txt", &buff);
  
  // cleanup
  close(sockfd);
  free(buff.text_buff);
  free(buff.file_buff);  
  return 0;
}

int send_metadata(int csockfd, struct FileData *fd) {
  int ires = 0;
  char *ser = serialize(fd);
  ires = send(csockfd, ser, strlen(ser) + 1, 0);
  free(ser);
  if (!write_state(ires != -1, "sent metadata", "sending metadata failed")) {
    return 0;
  }
  return 1;
}

void upload_file(int csockfd, const char *file_name, struct Buffer *buff) {
  struct Date date = get_current_time();
  struct FileData fd =
      make_filedata(file_name, 0, date.month, date.day, date.year, date.month,
                    date.day, date.year, -1);
  if (send_metadata(csockfd, &fd) != 1) {
    return;
  }
  int ires = 0;
  FILE *file = fopen(file_name, "r");
  if (!write_state(file != 0, "file opened", "file opening failed")) {
    fclose(file);
    return;
  }
  send_file(csockfd, file, buff);  
  fclose(file);
}

int send_file(int csockfd, FILE *file, struct Buffer *buff) {
  int ires = 0;
  int EOF_reached = 0;
  while (!EOF_reached) {
    for (int i = 0; i < FILE_BUFFLEN; ++i) {
      buff->file_buff[i] = fgetc(file);
      if (buff->file_buff[i] == EOF) {
        EOF_reached = 1;
	break;
      }
    }
    ires = send(csockfd, buff->file_buff, FILE_BUFFLEN * sizeof(int), 0);
    if (!write_state(ires != -1, "sent file chunk",
                     "sending file chunk failed")) {
      return 0;
    }
  }
  write_state(1, "file sent", "");
  return 1;
}

int send_request(int csockfd, const char* request, struct Buffer *buff) {
  int ires = send(csockfd, request, strlen(request) + 1, 0);
  if (!write_state(ires != -1, "sent request", "sending request failed")) {
    exit(1);
  }
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(ires != 0, "recieved response", "recv response failed")) {
    exit(1);
  }
  if (write_state(strcmp(buff->text_buff, "request_accepted") == 0,
		  buff->text_buff, buff->text_buff)) {
    return 1;
  }
  return 0;
}


int make_connected_socket(struct sockaddr_in *server) {
  int ires = 0;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (!write_state(sockfd != -1, "socket created", "socket creation failed")) {
    exit(1);
  }
  ires = connect(sockfd, (struct sockaddr *)server, sizeof(*server));
  if (!write_state(ires == 0, "connected to server", "connection failed")) {
    exit(1);
  }
  return sockfd;
}
