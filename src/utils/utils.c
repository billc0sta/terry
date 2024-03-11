#include "utils.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int write_state(int success, const char* on_success, const char* on_failure) {
  const char* state = (success) ? "error" : "success";
  const char* message = (success) ? on_success : on_failure;
  const char* info = (success) ? strerror(errno) : "no info";
  fprintf(stderr, "[%s]: message: %s, info: %s\n", state, message, info);
  return success;
}

char* get_file_name(const char* file_src) {
  char* file_name = malloc(sizeof(char) * (NAME_MAXLEN + 1));
  int last_slash_index = -1;
  for(int i = 0; file_src[i] != EOF; ++i) {
    if (file_src[i] == '\\' || file_src[i] == '/') {
      last_slash_index = i;
    }
  }
  int count = 0;
  for(count = 0; count < NAME_MAXLEN && file_src[count] != '\0'; file_name[count] = file_src[last_slash_index + 1 + count], ++count)
    ;
  file_name[count + 1] = '\0';
  return file_name;
}
