#include <stdio.h>
#include "../metadata/metadata.h"
#define TEXT_BUFFLEN 1000
#define FILE_BUFFLEN 4000
#define RQ_UPLOAD "upload_request"
#define RQ_DOWNLOAD "download_request"

struct Buffer {
  char *text_buff;
  int *file_buff;
};

struct Buffer make_buffer();
int write_state(int success, const char *on_success, const char *on_failure);
int recieve_file(int csockfd, FILE *write_file, struct Buffer *buff);
int send_file(int csockfd, FILE *file, struct Buffer *buff);
int send_metadata(int csockfd, struct FileData *fd);
int send_message(int csockfd, const char *message);
