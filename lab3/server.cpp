#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>

using namespace std;

int main(int argc,char *argv[]) {
  int server_descriptor, client_descriptor;
  struct sockaddr_in server_address;
  char *memory_block;
  int file_size;

  if (argc < 2) {
    cout<<"There is no filename of file to send.\n";
    return 0;
  }
  ifstream file_stream(argv[1], ios::in|ios::ate|ios::binary);
  server_descriptor = socket(AF_INET,SOCK_STREAM, 0);
  if (server_descriptor == -1) {
    cout<<("Unable to create socket.\n");
    return 0;
  }
  memset(&server_address, 0, file_sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_descriptor,(sockaddr *) &server_address,file_sizeof(server_address)) == -1) {
    cout<<("Binding error.\n");
    return 0;
  }
  listen(server_descriptor,1);
  cout<<"Server started\n";
  client_descriptor = accept(server_descriptor,NULL,NULL);
  if (client_descriptor == -1) {
    cout<<("Accept error.\n");
    close(server_descriptor);
    return 0;
  }
  if (file_stream.is_open())  {
    file_size = file_stream.tellg();
    memory_block = new char [file_size];
    file_stream.seekg (0, ios::beg);
    file_stream.read (memory_block, file_size);
    file_stream.close();
    write(client_descriptor,memory_block,file_size); //here
    cout << "File copied.\n";
    delete[] memory_block;
  } else {
    cout<<"unable to open file\n";
  }
  close(client_descriptor);
  close(server_descriptor);
  return 0;
}
