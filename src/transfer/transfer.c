#include "transfer.h"
#include "../utils/utils.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

void recieve_file(int csocket, FILE* write_file, unsigned char* buffer) {
  int bytes_recieved = 0;
  while((bytes_recieved = recv(csocket, buffer, BUFFER_SIZE, 0)) != 0) {
    if (bytes_recieved == -1) {
      write_error("recv failed", 0);
      return;
    } printf("just recieved bytes: %d\n", bytes_recieved / 4);

    for (int i = 0; i < bytes_recieved; i += 4) {
      int byte = 0;
      for (int b = 0; b < 4; b++) {
	*((unsigned char*)&byte + b) = buffer[b+i];
      }
      fputc(byte, write_file);
    }
  }
  printf("file recieved\n");
}


void send_file(int csocket, FILE* file, unsigned char* buffer) {
  int ires = 0;
  char end_reached = 0;
  int total_sent = 0;
  while(!end_reached) {
    int to_send = BUFFER_SIZE;
    for (int i = 0; i < BUFFER_SIZE; i += 4, total_sent += 1) {
      int byte = fgetc(file);
      if (byte == EOF) {
	end_reached = 1;
	to_send = i;
	break;
      }
      for(int b = 0; b < 4; ++b) {
	buffer[b+i] = *((unsigned char *)&byte + b);
      }
    }

    ires = send(csocket, buffer, to_send, 0);
    if (ires == -1) {
      write_error("send failed", 0);
      return;
    } printf("just sent bytes: %d\n", to_send / 4);
  }

  printf("uploaded, total sent bytes: %d\n", total_sent);
}
