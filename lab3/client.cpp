#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fstream>

using namespace std;

int main(int argc,char *argv[]) {
  int client_descriptor;
  struct sockaddr_in client_address;
  const int kBufferSize = 128;
  char buffer[kBufferSize];
  int n;
  ofstream file_stream;

  if (argc < 2) {
    cout<<"There is no filename of file to recieve.\n";
    return 0;
  }
  file_stream.open(argv[1],ios::binary);
  client_descriptor = socket(AF_INET,SOCK_STREAM, 0);
  if (client_descriptor == -1) {
    cout<<("Unable to create socket.\n");
    return 0;
  }
  memset(&client_address, 0, sizeof(client_address));
  client_address.sin_family = AF_INET;
  client_address.sin_port = htons(3333);
  if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) < 0) {
    cout<<"inet_pton error\n";
  }
  while(1) {
    if (connect(client_descriptor, (const sockaddr*) &client_address,
                 sizeof(client_address)) == -1) {
      continue;
    } else {
        cout<<"connected\n";
        memset(buffer,0,kBufferSize);
        while(1) {
          n = recv(client_descriptor,buffer,kBufferSize,0);
          if (n == -1){
            cout<<"Error while getting file.\n";
            break;
          }
          if (n == 0){
            cout<<"File recieved.\n";
            break;
         }
         file_stream << buffer;
        }
      }
    break;
  }
  file_stream.close();
  return 0;
}
