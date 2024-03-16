#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

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
