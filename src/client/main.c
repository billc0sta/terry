#include "../utils/utils."
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdbool.h>

const int port = 12223;
const int buffer_size = 26400;
const char* IP = "192.168.1.13";
sockaddr_in server_addr;

void connect_to_server() {
  int csocket = socket(AF_INET, SOCK_STREAM, 0);
  if (csocket == -1) {
    write_error("couldn't create_socket", true);
  }
  if (connect(csocket, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
    write_error("connection to server failed", true);
  }
}

void init_server() {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(IP);
}

int main() {
  init_server();
  
  return 0;
}
