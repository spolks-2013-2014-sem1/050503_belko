#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

int showErrorMessage (const char* message){

  cout << message;
  return 0;
}


int main(int argc,char *argv[]) {

  int server_descriptor, client_descriptor;
  struct sockaddr_in server_address;
  const int kBufferSize = 8;
  int file_size;
  char buffer[kBufferSize];

  if (argc < 2) {
    return showErrorMessage ("Error: no path to save file.\n");
  }

  ifstream file_stream (argv[1], ios::in|ios::ate|ios::binary);
  if (!file_stream.is_open()){
    return showErrorMessage ("Can't open file.\n");
  }

  int yes=1;
  server_descriptor = socket (AF_INET, SOCK_STREAM, 0);
  if (server_descriptor == -1) {
    return showErrorMessage ("Unable to create socket.\n");
  }

  if (setsockopt(server_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    showErrorMessage ("setsockopt");
  }

  memset (&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind (server_descriptor,(sockaddr *) &server_address, sizeof(server_address)) == -1) {
    return showErrorMessage ("Binding error.\n");
  }

  listen(server_descriptor,1);
  cout << "Server started\n";

  client_descriptor = accept (server_descriptor,NULL,NULL);
  if (client_descriptor == -1) {
    close (server_descriptor);
    return showErrorMessage ("Accept error.\n");
  }
  cout << "client connected.\n";

  file_size = file_stream.tellg();
  file_stream.seekg (0, ios::beg);
  sprintf (buffer,"%i",file_size);
  send (client_descriptor, buffer, kBufferSize,0);
  recv (client_descriptor, buffer, kBufferSize,0);

  cout << "sending started.\n";
  while (file_stream.tellg() != -1) {

    memset (buffer,0,kBufferSize);
    file_stream.read (buffer, kBufferSize);
    send (client_descriptor,buffer,kBufferSize,0);
  }

  file_stream.close();
  close(client_descriptor);
  close(server_descriptor);
  return 0;
}
