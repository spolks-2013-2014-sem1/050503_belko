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

using namespace std;

int showErrorMessage(const char* message){
  cout << message;
  return 0;
}



int main(int argc,char *argv[]) {

  int client_descriptor;
  struct sockaddr_in client_address;
  const int kBufferSize = 8;
  char buffer[kBufferSize];
  int temp = 0;
  long file_size = 0;
  ofstream file_stream;

  if (argc < 2) {
    return showErrorMessage("Error: no path to save file.\n");
  }

  file_stream.open(argv[1],ios::binary);

  client_descriptor = socket(AF_INET,SOCK_STREAM, 0);
  if (client_descriptor == -1) {
    return showErrorMessage("Unable to create socket.\n");
  }

  memset (&client_address, 0, sizeof(client_address));
  client_address.sin_family = AF_INET;
  client_address.sin_port = htons(3333);
  if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) < 0) {
    return showErrorMessage("inet_pton error\n");
  }

  cout << "looking for server\n";
  while(1) {
    if (connect (client_descriptor, (const sockaddr*) &client_address,
                 sizeof (client_address)) == -1) {
      continue;

    } else {
        cout << "connected\n";
        memset (buffer,0,kBufferSize);

        recv (client_descriptor,buffer,kBufferSize,0);

        file_size = strtol (buffer, NULL, 10);
        cout << file_size;
        send (client_descriptor, buffer, kBufferSize,0);
        cout << "recieving started\n";

        while  (temp < file_size) {

          memset (buffer,0,kBufferSize);
          recv (client_descriptor, buffer, kBufferSize,0);
          file_stream << buffer;
          send (client_descriptor, buffer, kBufferSize,0);
          temp += kBufferSize;
          cout << "\n";
          cout << temp;
        }
      }

    break;
  }

  cout << "file saved.\n";
  file_stream.close();
  close(client_descriptor);
  return 0;
}
