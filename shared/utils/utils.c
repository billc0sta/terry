#include "utils.h"
#include "../metadata/metadata.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

int write_state(int success, const char *on_success, const char *on_failure) {
  const char *label = (success) ? "success" : "failure";
  const char *message = (success) ? on_success : on_failure;
  const char *info = (success) ? "" : strerror(errno);
  fprintf(stdout, "[%s] -> %s : %s\n", label, message, info);
  return success;
}

struct Buffer make_buffer() {
  struct Buffer buff;
  buff.text_buff = malloc(sizeof(char) * TEXT_BUFFLEN);
  buff.file_buff = malloc(sizeof(int) * FILE_BUFFLEN);
  return buff;
}

int recieve_file(int csockfd, FILE *write_file, struct Buffer *buff) {
  int ires = 0;
  int EOF_reached = 0;
  int total_size = 0;
  while (!EOF_reached) {
    ires = recv(csockfd, buff->file_buff, sizeof(int) * FILE_BUFFLEN, 0);
    if (!write_state(ires != -1, "recieved file chunk",
		     "recieving file chunk failed")) {
      return 0;
    }
    for (int i = 0; i < ires; ++i, ++total_size) {
      if (buff->file_buff[i] == EOF) {
        EOF_reached = 1;
        break;
      }
      fputc(buff->file_buff[i], write_file);
    }
  }
  write_state(1, "file recieved", "");
  return total_size;
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

int send_message(int csockfd, const char* message) {
  return send(csockfd, message, strlen(message), 0);
}
