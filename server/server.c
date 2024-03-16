#include "../shared/utils/utils.h"
#include "../shared/metadata/metadata.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 20024

struct Server {
  int sockfd;
  struct sockaddr_in addr;
};

struct Server start_server();
void run_server(struct Server *server, struct Buffer *buff);
int send_reqaccept(int csockfd);
int send_reqfail(int csockfd, const char *reason);
void upload_request(int csockfd, struct Buffer *buff);

int main() {
  printf("server side\n");
  struct Server server = start_server();
  struct Buffer buff = make_buffer();
  run_server(&server, &buff);
  
  // cleanup
  close(server.sockfd);
  free(buff.text_buff);
  free(buff.file_buff);
  
  return 0;
}

int recieve_file(int csockfd, FILE *write_file, struct Buffer *buff) {
  int ires = 0;
  int EOF_reached = 0;
  while (!EOF_reached) {
    ires = recv(csockfd, buff->file_buff, sizeof(int) * FILE_BUFFLEN, 0);
    if (!write_state(ires != -1, "recieved file chunk",
		     "recieving file chunk failed")) {
      return 0;
    }
    for (int i = 0; i < ires; ++i) {
      if (buff->file_buff[i] == EOF) {
        EOF_reached = 1;
        break;
      }
      fputc(buff->file_buff[i], write_file);
    }
  }
  write_state(1, "file recieved", "");
  return 1;
}

void upload_request(int csockfd, struct Buffer *buff) {
  int ires = 0;
  // recieve file metadata
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(ires != -1, "recieved metadata", "recv metadata failed")) {
    return;
  }
  struct FileData fd = deserialize(buff->text_buff);
  FILE *write_file = fopen("try.txt", "w");
  if (!write_state(write_file != 0, "writing file opened",
                   "opening writing file failed ")) {
    return;
  }
  recieve_file(csockfd, write_file, buff);
  fclose(write_file);
}

int send_reqaccept(int csockfd) {
  int ires =
      send(csockfd, "request_accepted", strlen("request_accepted") + 1, 0);
  if (!write_state(ires != -1, "sent request_accepted",
                   "sending req_accept failed")) {
    return -1;
  }
  return 0;
}

int send_reqfail(int csockfd, const char *reason) {
  int message_len = strlen("request_denied: ") + strlen(reason) + 2;
  char *message = malloc(sizeof(char) * message_len);
  snprintf(message, message_len, "request_denied: %s", reason);
  
  int ires = send(csockfd, message, message_len, 0);
  free(message);
  if (!write_state(ires != -1, "sent request response",
                   "sending request response failed")) {
    return -1;
  }
  return 0;
}

void run_server(struct Server *server, struct Buffer *buff) {
  int csockfd = -1;
  int ires = 0;
  while (1) {
    csockfd = accept(server->sockfd, 0, 0);
    if (!write_state(csockfd != -1, "accepted socket", "accept failed")) {
      exit(1);
    }
    // recieve request
    ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
    if (!write_state(csockfd != 1, "recieved request", "recv request failed")) {
      exit(1);
    }
    if (strcmp(buff->text_buff, RQ_UPLOAD) == 0) {
      send_reqaccept(csockfd);
      upload_request(csockfd, buff);
    } else if (strcmp(buff->text_buff, RQ_DOWNLOAD)) {
      send_reqaccept(csockfd);
      // TODO: MAKE download_req();
    } else {
      send_reqfail(csockfd, "no_such_request");
    }
  }
}

struct Server start_server() {
  int ires = 0;
  struct Server server;
  server.addr.sin_family = AF_INET;
  server.addr.sin_port = htons(PORT);
  server.addr.sin_addr.s_addr = INADDR_ANY;
  server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (!write_state(server.sockfd != -1, "socket created", "socket creation failed")) {
    exit(1);
  }
  ires = bind(server.sockfd, (struct sockaddr *)&server.addr, sizeof(server.addr));
  if (!write_state(ires == 0, "binding done", "binding failed")) {
    exit(1);
  }
  ires = listen(server.sockfd, 10);
  if (!write_state(ires != -1, "listen backlogged", "listen failed")) {
    exit(1);
  }
  return server;
}
