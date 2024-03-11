#define NAME_MAXLEN 255
#define PORT 56821
#define BUFFER_SIZE 26400

int write_state(int success, const char* on_success, const char* on_failure);
char* get_file_name(const char* file_src);
void int_in_charr(char* arr, int put, int index); 
