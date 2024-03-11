#include "utils.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void write_error(const char* message, int close){
  fprintf(stderr, "[error]: %s, errno: %s\n", message, strerror(errno));
  if (close) {
    exit(1);
  }
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


void int_in_charr(char* arr, int put, int index) {
  for (int byte = 0; byte < 4; ++byte) {
    arr[index++] = *((char*)&put + byte);
  }
}
