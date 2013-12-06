#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

int showErrorMessage(const char* message){
  cout << message;
  return -1;
}


int main(int argc,char *argv[]) {

  int client_descriptor, n = 1, temp = 1;
  struct sockaddr_in client_address;
  struct timespec timeout;
  const int kBufferSize = 256;
  char buffer[kBufferSize];
  FILE *pfile;

  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;
  fd_set server_fds;

  if (argc < 2) {
    return showErrorMessage("Error: no path to save file.\n");
  }

  pfile = fopen (argv[1], "w");
  client_descriptor = socket(AF_INET,SOCK_STREAM, 0);
  FD_ZERO(&server_fds);
  FD_SET(client_descriptor, &server_fds);
  if (client_descriptor == -1) {
    return showErrorMessage("Unable to create socket.\n");
  }

  memset (&client_address, 0, sizeof(client_address));
  client_address.sin_family = AF_INET;
  client_address.sin_port = htons(3333);
  if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) < 0) {
    return showErrorMessage("inet_pton error\n");
  }

  while(1) {
    if (connect (client_descriptor, (const sockaddr*) &client_address,
                 sizeof (client_address)) == -1) {
      continue;
    } else {
        while  ((n = recv (client_descriptor, buffer, kBufferSize,0)) > 0) {
          temp = pselect (client_descriptor + 1, &server_fds,
                         NULL, NULL, &timeout, NULL);
          if (temp == 0 || temp == -1){
            close(client_descriptor);
            fclose(pfile);
            return showErrorMessage("connection lost\n");
          }
          fwrite (buffer, n, 1, pfile);
        }
        if (n == -1) {
          fclose(pfile);
          close(client_descriptor);
          return showErrorMessage("recieve error\n");
        }
      }
    break;
  }

  close(client_descriptor);
  return 0;
}
