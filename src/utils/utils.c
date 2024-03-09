#include "utils.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void write_error(const char* message, int close){
  printf("[error]: %s, errno: %s\n", message, strerror(errno));
  if (close) {
    exit(1);
  }
}
