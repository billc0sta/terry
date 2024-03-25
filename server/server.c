#include "../shared/utils/utils.h"
#include "../shared/metadata/metadata.h"
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 20121
#define STORAGE_DIR "../storage/"

const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                     "abcdefghijklmnopqrstuvwxyz" "123456789";
struct Server {
  int sockfd;
  struct sockaddr_in addr;
};

struct Server start_server();
void run_server(struct Server *server, struct Buffer *buff);
int send_reqaccept(int csockfd);
int send_reqdeny(int csockfd, const char *reason);
void upload_request(int csockfd, struct Buffer *buff);
void download_request(int csockfd, struct Buffer *buff);
int make_folder(const char *hash);
int update_usage_date(const char *metadata_dir);
char *get_base64();

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

int update_usage_date(const char *metadata_dir) {
  FILE *file = fopen(metadata_dir, "r");
  char raw[FD_SERIALIZE_LEN];
  int i = 0;
  for (int byte = 0; i < FD_SERIALIZE_LEN; ++i) {
    if ((byte = fgetc(file)) == EOF)
      break;
    raw[i] = byte;
  }
  raw[i + 1] = '\0';
  struct FileData fd = deserialize(raw);
  fd.last_used = get_current_time();
  save_metadata(metadata_dir, &fd);  
}

int make_folder(const char *hash) {
  int dirlen = strlen(hash) + strlen(STORAGE_DIR) + 4;
  char *dir = malloc(sizeof(char) * dirlen);
  snprintf(dir, dirlen, "%s%s/", STORAGE_DIR, hash);
  int ires = mkdir(dir, 0);
  free(dir);
  return ires;
}

char *gen_base64() {
  srand(time(0));
  char *rs = malloc(sizeof(char) * 12);
  for (int i = 0; i < 11; ++i) {
    rs[i] = base64[rand() % 61]; // yes. 61
  }
  return rs;
}

void download_request(int csockfd, struct Buffer *buff) {
  int ires = 0;
  // recieve file hash
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(ires != -1, "recieved file hash",
                   "recieving file hash failed")) {
    return;
  }

  // check if hash is an existing file
  int path_len = 350;
  char path[path_len];
  char hash[HASH_LEN];
  strncpy(hash, buff->text_buff, HASH_LEN);
  snprintf(path, path_len, "%s%s/", STORAGE_DIR, hash);
  DIR *dir = opendir(path);
  if (!write_state(errno != ENOENT, "file found", "no such file")) {
    send_message(csockfd, "no such file");
    return;
  } else {
    send_message(csockfd, "file found");
  }

  // get and send metadata
  strcat(path, "metadata.txt");
  struct FileData fd;
  open_metadata(path, &fd);
  update_usage_date(path);
  ires = send_metadata(csockfd, &fd);
  if (ires != 1) {
    return;
  }

  // open and send file
  snprintf(path, path_len, "%s%s/%s", STORAGE_DIR, hash, fd.name);
  printf("path: %s\n", path);
  FILE* file = fopen(path, "r");
  ires = send_file(csockfd, file, buff);
  fclose(file);
  if (ires == 0) {
    write_state(0, "", "sending file failed");
  }
  printf("bytes send: %d\n", ires);
}


// this function is fucking gigantic, idk what to do
void upload_request(int csockfd, struct Buffer *buff) {
  int ires = 0;
  // recieve file metadata
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(ires != -1, "recieved metadata", "recv metadata failed")) {
    return;
  }
  struct FileData fd = deserialize(buff->text_buff);
  char *hash = gen_base64();
  strcpy(fd.hash, hash);
  free(hash);
  if (!write_state(make_folder(fd.hash) == 0, "created directory",
                   "creating directory failed")) {
    return;
  }
  int path_len = 350;
  char path[path_len];
  snprintf(path, path_len, "%s%s/%s", STORAGE_DIR, fd.hash, fd.name);
  FILE *write_file = fopen(path, "w");
  if (!write_state(write_file != 0, "writing file opened",
                   "opening writing file failed")) {
    return;
  }
  ires = receive_file(csockfd, write_file, fd.size, buff);
  fclose(write_file);
  if (ires == 0) {
    send_message(csockfd, "upload failed");
    return;
  }
  fd.size = ires;
  snprintf(path, path_len, "%s%s/metadata.txt", STORAGE_DIR, fd.hash);
  send_metadata(csockfd, &fd);
  save_metadata(path, &fd);  
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

int send_reqdeny(int csockfd, const char *reason) {
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
    } else if (strcmp(buff->text_buff, RQ_DOWNLOAD) == 0) {
      send_reqaccept(csockfd);
      download_request(csockfd, buff);
    } else {
      send_reqdeny(csockfd, "no_such_request");
    }

    shutdown(csockfd, SHUT_RDWR);
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
