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
  fprintf(stdout, "[%s] -> %s * %s\n", label, message, info);
  return success;
}

struct Buffer make_buffer() {
  struct Buffer buff;
  buff.text_buff = malloc(sizeof(char) * TEXT_BUFFLEN);
  buff.file_buff = malloc(sizeof(int) * FILE_BUFFLEN);
  return buff;
}

int send_file(int csockfd, FILE *file, struct Buffer *buff) {
    int ires = 0;
    int total_sent = 0;
    while (1) {
        int to_send = fread(buff->file_buff, sizeof(int), FILE_BUFFLEN, file);
        ires = send(csockfd, buff->file_buff, to_send * sizeof(int), 0);
        if (!write_state(ires != -1, "sent file chunk", "sending file chunk failed")) {
            return 0;
        }
        total_sent += to_send;
        if (to_send < FILE_BUFFLEN)
            break;
    }
    write_state(1, "file sent", "");
    return total_sent;
}


int receive_file(int csockfd, FILE *write_file, int max_recv, struct Buffer *buff) {
    int ires = 0;
    int total_recv = 0;
    while (total_recv < max_recv) {
        ires = recv(csockfd, buff->file_buff, FILE_BUFFLEN * sizeof(int), 0);
        if (!write_state(ires != -1, "received file chunk", "receiving file chunk failed")) {
            return 0;
        }
        if (ires == 0) {
            // End of file received
            break;
        }
        total_recv += ires / sizeof(int);
        fwrite(buff->file_buff, sizeof(int), ires / sizeof(int), write_file);
    }
    write_state(1, "file received", "");
    return total_recv;
}

int send_metadata(int csockfd, struct FileData *fd) {
  int ires = 0;
  char *ser = serialize(fd);
  int strl = strlen(ser) + 1;
  ires = send(csockfd, ser, strl, 0);
  free(ser);
  if (!write_state(ires != -1, "sent metadata", "sending metadata failed")) {
    return 0;
  }
  return 1;
}

int send_message(int csockfd, const char* message) {
  return send(csockfd, message, strlen(message) + 1, 0);
}
