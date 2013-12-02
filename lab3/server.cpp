#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

int showErrorMessage (const char* message){
  cout << message;
  return 0;
}


int main(int argc,char *argv[]) {
  int server_descriptor, client_descriptor, temp = 1;
  struct sockaddr_in server_address;
  const int kBufferSize = 128;
  char buffer[kBufferSize];
  struct timespec timeout;
  fd_set server_fds;
  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;

  if (argc < 2) {
    return showErrorMessage ("Error: no path to save file.\n");
  }
  int yes=1;
  server_descriptor = socket (AF_INET, SOCK_STREAM, 0);
  if (server_descriptor == -1) {
    return showErrorMessage ("Unable to create socket.\n");
  }
  if (setsockopt(server_descriptor, SOL_SOCKET,
                 SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    showErrorMessage ("setsockopt error");
  }
  memset (&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind (server_descriptor,(sockaddr *) &server_address,
             sizeof(server_address)) == -1) {
    return showErrorMessage ("Binding error.\n");
  }
  while(1) {
    listen(server_descriptor,1);
    ifstream file_stream (argv[1], ios::in|ios::ate|ios::binary);
    if (!file_stream.is_open()){
      return showErrorMessage ("Can't open file.\n");
    }
    client_descriptor = accept (server_descriptor,NULL,NULL);
    if (client_descriptor == -1) {
      close (server_descriptor);
      return showErrorMessage ("Accept error.\n");
    }
    FD_ZERO(&server_fds);
    FD_SET(client_descriptor, &server_fds);
    file_stream.seekg (0, ios::beg);

    while (file_stream.tellg() != -1) {
      temp = pselect (client_descriptor + 1, NULL, &server_fds,
                          NULL, &timeout, NULL);
      if (temp == 0 || temp == -1){
        close(server_descriptor);
        file_stream.close();
        return showErrorMessage("connection lost\n");
      }
      file_stream.read (buffer, kBufferSize);
      send (client_descriptor,buffer,kBufferSize,0);
    }
    file_stream.close();
    close(client_descriptor);
  }
}
