#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

using namespace std;
const int kBufferSize = 256;
const int kMaxClients = 15;
FILE *pfile;
int file_size;

int showErrorMessage (const char* message){
  cout << message;
  return 0;
}


int GetFileLength (FILE* pfile){
  int file_size;
  fseek (pfile, 0, SEEK_END);
  file_size = ftell (pfile);
  fseek (pfile, 0, SEEK_SET);
  return file_size;
}


static void* SendFileTCP (void *arg) {
  pthread_detach (pthread_self());
  int file_position = 0, n;
  char buffer[kBufferSize];

  while (file_size > file_position) {
    fseek (pfile, file_position, SEEK_SET);
    n = fread (buffer, 1, kBufferSize, pfile);
    file_position += n;
    send ((int) arg, buffer, n, 0);
  }

  close ((int) arg);
  return NULL;
}


int main(int argc,char *argv[]) {

  int server_descriptor, client_descriptor, yes = 1, i = 0;
  pthread_t tid;
  socklen_t addrlen, len;
  struct sockaddr_in server_address;
  struct sockaddr *cliaddr;

  if (argc < 2) {
    return showErrorMessage ("error: no path to save file.\n");
  }

  pfile = fopen (argv[1], "r");
  if (pfile == NULL){
    return showErrorMessage("can not open file\n");
  }

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

  listen(server_descriptor, kMaxClients);

  file_size = GetFileLength(pfile);

  while (1) {
    len = addrlen;
    client_descriptor = accept (server_descriptor, cliaddr, &len);
    if (pthread_create (&tid,NULL,&SendFileTCP,(void *)client_descriptor) > 0){
      close (client_descriptor);
      cout << "unable to create thread\n";
    }
  }

}

