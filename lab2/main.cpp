#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>

using namespace std;

int main() {
  int server_descriptor, client_descriptor;
  struct sockaddr_in server_address;
  const int kBufferSize = 256;
  char buffer[kBufferSize];

  server_descriptor = socket(AF_INET,SOCK_STREAM, 0);
  if (server_descriptor == -1) {
    cout<<("Unable to create socket.\n");
    return 0;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_descriptor,(sockaddr *) &server_address,sizeof(server_address)) == -1) {
    cout<<("Binding error.\n");
	close(server_descriptor);
    return 0;
  }
  if (listen(server_descriptor,1) == -1){
    cout<<("Listen error.\n");
	close(server_descriptor);
	return 0;
  }
  cout<<"Server started\n";

  while(1) {
    client_descriptor = accept(server_descriptor,NULL,NULL);
	if(client_descriptor < 0){
	  cout<<"Accept error.\n";
	  close(server_descriptor);
	  return 0;
	}
      while(1) {
        memset(buffer,0,kBufferSize);
        read(client_descriptor,buffer,kBufferSize);
        cout<<buffer;
        if (strncmp(buffer,"exit",4) == 0) {
          cout<<("Server closed.\n");
          break;
        }
        write(client_descriptor,buffer,strlen(buffer));
      }
    shutdown(client_descriptor, SHUT_RDWR);
    close(client_descriptor);
    break;
  }
  close(server_descriptor);
  return 0;
}
