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
const int kBufferSize = 1024;

int showErrorMessage (const char* message){
  cout << message;
  return -1;
}


int StartTCPServer (char* file_name) {
  int server_descriptor, client_descriptor, temp = 1, i = 0, byte_sent = 0;
  int OOB_interval = 0, file_length = 0;
  struct sockaddr_in server_address;
  const int kBufferSize = 128;
  char buffer[kBufferSize];
  struct timespec timeout;
  fd_set server_fds;
  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;

  int yes=1;
  server_descriptor = socket (AF_INET, SOCK_STREAM, 0);
  if (server_descriptor == -1) {
    return showErrorMessage ("Unable to create socket.\n");
  }
  if (setsockopt (server_descriptor, SOL_SOCKET,
                 SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    showErrorMessage ("setsockopt error");
  }
  memset (&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind (server_descriptor, (sockaddr *) &server_address,
             sizeof(server_address)) == -1) {
    return showErrorMessage ("Binding error.\n");
  }
  while(1) {
    listen (server_descriptor,1);
    ifstream file_stream (file_name, ios::in|ios::ate|ios::binary);
    if (!file_stream.is_open ()){
      return showErrorMessage ("Can't open file.\n");
    }
    client_descriptor = accept (server_descriptor,NULL,NULL);
    if (client_descriptor == -1) {
      close (server_descriptor);
      return showErrorMessage ("Accept error.\n");
    }
    FD_ZERO (&server_fds);
    FD_SET (client_descriptor, &server_fds);
    file_stream.seekg (0, ios::end);
    file_length = file_stream.tellg();
    OOB_interval = (int) file_length / 768;
    file_stream.seekg (0, ios::beg);

    while (file_stream.tellg() != -1) {
      temp = pselect (client_descriptor + 1, NULL, &server_fds,
                          NULL, &timeout, NULL);
      if (temp == 0 || temp == -1) {
        close (server_descriptor);
        file_stream.close ();
        return showErrorMessage("connection lost\n");
      }
      file_stream.read (buffer, kBufferSize);
      byte_sent += send (client_descriptor,buffer,kBufferSize,0);
      i++;
      if (i == OOB_interval){
        sleep(1);
        send (client_descriptor,"5",1,MSG_OOB);
        i = 0;
        printf ("%i \n", byte_sent);
      }
    }
    file_stream.close();
    close (client_descriptor);
    i = 0;
    byte_sent = 0;
  }
}


int SendFileUDP(int socket,FILE *pfile,struct sockaddr_in client_address) {
  socklen_t len = sizeof(client_address);
  char buffer[kBufferSize];
  int n = 0, temp = 0, n2 = 0;
  struct timespec timeout;
  fd_set server_fds;
  timeout.tv_sec = 8;
  timeout.tv_nsec = 0;

  FD_ZERO(&server_fds);
  FD_SET(socket, &server_fds);

  while (!feof(pfile)) {
    n = fread (buffer, 1, sizeof(buffer), pfile);

    if(n != 0) {
      n = sendto(socket,buffer, n, 0, (sockaddr *) &client_address, len);
      if (n < 0) {
        return showErrorMessage("sending failed\n");
      }
      temp = pselect (socket + 1, &server_fds, NULL,
                          NULL, &timeout, NULL);
      if (temp == 0 || temp == -1){
        close(socket);
        return showErrorMessage("connection lost\n");
      }
      recv(socket, buffer, 1, 0);
      if (buffer[0] != 'q'){
        return showErrorMessage("recieve error\n");
      }
    }
  }
  return 0;
}


int GetFileLength (FILE* pfile){
  int file_size;

  fseek (pfile, 0, SEEK_END);
  file_size = ftell (pfile);
  fseek (pfile, 0, SEEK_SET);
  return file_size;
}


char* Itoa (int input) {

  char *buffer = new char[20];
  char buffer2[20];
  int i = 0, j = 0;

  while ((input / 10) >= 1) {
    buffer2 [i] = '0' + input % 10;
    input /= 10;
    i++;
  }

  buffer2 [i] = '0' + input % 10;

  while (i != -1) {
    buffer[j] = buffer2[i];
    i--;
    j++;
  }
  return buffer;
}


int StartUDPServer(char *file_name) {

  int server_descriptor, n = 0, file_size;
  struct sockaddr_in server_address, client_address;
  char buffer[kBufferSize];
  socklen_t len = sizeof (client_address);
  FILE *pfile;

  pfile = fopen (file_name, "r");

  server_descriptor = socket(AF_INET,SOCK_DGRAM, 0);
  if (server_descriptor == -1) {
    cout<<("unable to create socket.\n");
    return 0;
  }

  memset (&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(3333);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind (server_descriptor, (sockaddr *)
            &server_address, sizeof(server_address)) == -1) {
    return showErrorMessage("bindingError\n");
  }

  while (1){
    recvfrom (server_descriptor, buffer, 1,
                       0, (struct sockaddr *) &client_address, &len);

    sendto (server_descriptor, Itoa (GetFileLength (pfile)),
          20, 0, (sockaddr *) &client_address, len);

    if (SendFileUDP (server_descriptor, pfile, client_address) == -1){
      return -1;
    }
  }
}


int main (int argc, char *argv[]){


  if (argc < 2) {
    return showErrorMessage ("error: no path to save file.\n");
  }

  if (argc == 3){
    if (strncmp(argv[2],"-u",2) == 0){
      return StartUDPServer (argv[1]);
    }
  }

  return StartTCPServer (argv[1]);
}
