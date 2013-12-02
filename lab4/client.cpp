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
#include <signal.h>
#include <fcntl.h>

using namespace std;
void sigurg_handler(int);
int byte_recieved = 0, client_descriptor;

int showErrorMessage (const char* message) {
  cout << message;
  return 0;
}

int main(int argc,char *argv[]) {

  int n = 1, temp = 1;
  struct sockaddr_in client_address;
  struct timespec timeout;
  const int kBufferSize = 128;
  char buffer[kBufferSize];
  ofstream file_stream;
  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;
  fd_set server_fds;

  if (argc < 2) {
    return showErrorMessage ("Error: no path to save file.\n");
  }

  file_stream.open (argv[1],ios::binary);
  client_descriptor = socket (AF_INET,SOCK_STREAM, 0);
  if (client_descriptor == -1) {
    return showErrorMessage ("Unable to create socket.\n");
  }
  FD_ZERO (&server_fds);
  FD_SET (client_descriptor, &server_fds);

  memset (&client_address, 0, sizeof(client_address));
  client_address.sin_family = AF_INET;
  client_address.sin_port = htons(3333);

  if (inet_pton (AF_INET, "127.0.0.1", &client_address.sin_addr) < 0) {
    return showErrorMessage ("inet_pton error\n");
  }

  while(1) {
    if (connect (client_descriptor, (const sockaddr*) &client_address,
                 sizeof (client_address)) == -1) {
      continue;
    } else {
        signal (SIGURG, sigurg_handler);
        fcntl (client_descriptor, F_SETOWN, getpid());
        while  (1) {
          n = recv (client_descriptor, buffer, kBufferSize,0);
          if ( n <= 0)
          { break;}
          temp = pselect (client_descriptor + 1, &server_fds,
                         NULL, NULL, &timeout, NULL);
          if (temp == 0 || temp == -1){
            close (client_descriptor);
            file_stream.close();
            return showErrorMessage ("connection lost\n");
          }
          byte_recieved += n;
          file_stream.write (buffer, n);
        }
        if (n == -1){
         return showErrorMessage ("recieve error\n");
        }
      }
    break;
  }

  file_stream.close();
  close (client_descriptor);
  return 0;
}

void sigurg_handler (int param){
  char buff[10];
  printf ("%i\n", byte_recieved);
}
