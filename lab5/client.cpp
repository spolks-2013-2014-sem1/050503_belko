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
const int kBufferSize = 1024;
void sigurg_handler(int);
int byte_recieved = 0, client_descriptor;

int showErrorMessage(const char* message){
  cout << message;
  return -1;
}


int StartTCPClient (char* file_name) {
  int n = 1, temp = 1;
  struct sockaddr_in client_address;
  struct timespec timeout;
  const int kBufferSize = 128;
  char buffer[kBufferSize];
  ofstream file_stream;
  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;
  fd_set server_fds;

  file_stream.open (file_name, ios::binary);
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


int GetFileUDP(int socket, FILE *pfile, struct sockaddr_in address, int file_length) {
  socklen_t len = sizeof(address);
  int n = 0, bytes_read = 0, temp = 0;
  char buffer[kBufferSize];

  while (1) {

    n = recv(socket, buffer, kBufferSize, 0);
    fwrite (buffer, n, 1, pfile);

    if (sendto (socket, "q", 1, 0, (sockaddr *) &address, len) < 0){
      return -1;
    }
    if (bytes_read == file_length) {
      return 0;
    }
    bytes_read ++;
  }
}


int StartUDPClient (char *file_name) {
  int server_descriptor, file_size;
  struct sockaddr_in server_address;
  socklen_t len;
  char buffer[kBufferSize];
  FILE *pfile;

  pfile = fopen (file_name, "w");

  memset (&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);

  if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) < 0) {
    return showErrorMessage("inet_pton error\n");
  }

  server_descriptor = socket(AF_INET,SOCK_DGRAM, 0);
  if (server_descriptor == -1) {
    return showErrorMessage("Unable to create socket.\n");
  }

  len = sizeof (server_address);
  sendto (server_descriptor, "x", 1, 0, (sockaddr *) &server_address, len);

  recv (server_descriptor, buffer, 20, 0);

  GetFileUDP(server_descriptor, pfile, server_address, atoi (buffer)/1024 );

  fclose (pfile);
  close (server_descriptor);
  return 0;
}


int main (int argc, char *argv[]){

  if (argc < 2) {
    return showErrorMessage ("Error: no path to save file.\n");
  }

  if (argc == 3){
    if (strncmp (argv[2], "-u", 2) == 0) {
      return StartUDPClient (argv[1]);
    }
  }
  return StartTCPClient (argv[1]);
}

