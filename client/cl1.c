et.h>
#include <unistd.h>
#define PORT 20024
#define IP "192.168.1.6"

int make_connected_socket(struct sockaddr_in *server);
int send_request(int csockfd, const char* request, struct Buffer *buff);
void upload_file(int csockfd, const char *file_name, struct Buffer *buff);
void download_file(int csockfd, const char *file_hash, struct Buffer *buff);
int get_file_size(const char *file);

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
  upload_file(sockfd, "client.c", &buff);
  
  // cleanup
  close(sockfd);
  free(buff.text_buff);
  free(buff.file_buff);  
  return 0;
}

int get_file_size(const char *file) {
  FILE *read_file = fopen(file, "r");
  int i = 0;
  while (fgetc(read_file) != EOF)
    ++i;
  return i;
}

void download_file(int csockfd, const char *file_hash, struct Buffer *buff) {
  // send file hash
  int ires = 0;
  ires = send_message(csockfd, file_hash);
  if (!write_state(ires != -1, "sent file hash", "sending file hash failed")) {
    return;
  }

  // recieve response
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (ires == -1)
    write_state(0, "", "recieving response failed");
  if (!write_state(strcmp(buff->text_buff, "file found") == 0, "file found",
                   "no such file")) {
    return;
  }

  // recieve metadata
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(ires != -1, "recieved metadata",
                   "recieving metadata failed")) {
    return;
  }
  printf("raw: %s\n, bytes recieved: %d\n", buff->text_buff, ires);
  struct FileData fd = deserialize(buff->text_buff);
  printf("write_file name: %s\n", fd.name);

  // recieve file
  FILE *write_file = fopen(fd.name, "w");
  if (write_file == 0) {
    write_state(0, "", "opening write file failed");
    return;
  }
  ires = receive_file(csockfd, write_file, fd.size, buff);
  printf("bytes recieved: %d\n", ires);
  fclose(write_file);
}

void upload_file(int csockfd, const char *file_name, struct Buffer *buff) {
  struct Date date = get_current_time();
  int ires = 0;
  FILE *file = fopen(file_name, "r");
  struct FileData fd =
    make_filedata(file_name, 0, date.month, date.day, date.year, date.month,
		  date.day, date.year, get_file_size(file_name));
  if (send_metadata(csockfd, &fd) != 1) {
    return;
  }
  if (!write_state(file != 0, "file opened", "file opening failed")) {
    return;
  }
  send_file(csockfd, file, buff);
  fclose(file);
  ires = recv(csockfd, buff->text_buff, TEXT_BUFFLEN, 0);
  if (!write_state(strcmp(buff->text_buff, "upload failed") != 0,
                   "file uploaded", "upload_failed")) {
    return;
  }
  
  fd = deserialize(buff->text_buff);
  printf("HASH: %s | use it to download the file\n", fd.hash);
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
ÿcket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 20024
#define IP "192.168.1.6"

int make_connected_socket(struct sockaddr_in *server);
int send_request(int csockfd, const char* request, struct Buffer *b