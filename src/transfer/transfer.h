#include <stdio.h>

void recieve_file(int csocket, FILE* write_file, unsigned char* buffer);
void send_file(int csocket, FILE* file, unsigned char* buffer);
