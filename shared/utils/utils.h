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
