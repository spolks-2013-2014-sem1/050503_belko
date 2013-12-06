#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <poll.h>

using namespace std;
const int kBufferSize = 256;
const int kMaxClients = 15;

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


int StartTCPMultiplexedServer (int server_descriptor, pollfd clients[],
                               int connected_clients, char *file_name) {

  int rdy_descriptors, maxfd = server_descriptor, client_descriptor, i = 0, n = 0;
  int buffer_descriptor, file_length;
  struct sockaddr_in client_address;
  char buffer[kBufferSize];
  int file_positions[kMaxClients];

  socklen_t client_address_length = sizeof (client_address);
  FILE *pfile;
  pfile = fopen (file_name, "r");
  if (pfile == NULL){
    return showErrorMessage("can not open file\n");
  }
  file_length = GetFileLength (pfile);

  while (1) {
    rdy_descriptors = poll (clients, maxfd + 1, 0);

    if (clients[0].revents & POLLIN) {
      client_descriptor = accept (server_descriptor,
                                  (sockaddr *) &client_address,
                                  &client_address_length);
      for (i = 1; i < kMaxClients; i++) {
        if (clients[i].fd < 0){
          clients[i].fd = client_descriptor;
          break;
        }
      }
      if (i == kMaxClients){
        cout << "reached clients limit\n";
        continue;
      }
      clients[i].events = POLLIN;
      file_positions[i] = 0;
      if (i > connected_clients) {
        connected_clients = i;
      }
      if (--rdy_descriptors <= 0) {
        continue;
      }
    }

    for (i = 1; i <= connected_clients; i++) {
      if ((buffer_descriptor = clients[i].fd) < 0){
        continue;
      }
      if (clients[i].revents & POLLHUP) {
         close (clients[i].fd);
         clients[i].fd = -1;
         file_positions[i] = 0;
        continue;
      }
      fseek (pfile, file_positions[i], SEEK_SET);
      n = fread (buffer, 1, kBufferSize, pfile);
      file_positions[i] += n;
      n = send (buffer_descriptor, buffer, n, 0);

      if (file_positions[i] >= file_length) {
        close (clients[i].fd);
        clients[i].fd = -1;
        file_positions[i] = 0;
        continue;
      }
      if (--rdy_descriptors <= 0){
        break;
      }
    }

  }
}


int main(int argc,char *argv[]) {

  int server_descriptor, yes = 1, i = 0;
  struct pollfd clients[kMaxClients];
  struct sockaddr_in server_address;

  if (argc < 2) {
    return showErrorMessage ("error: no path to save file.\n");
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

  clients[0].fd = server_descriptor;
  clients[0].events = POLLIN;

  for (i = 1; i < kMaxClients; i++) {
    clients[i].fd = -1;
  }

  return StartTCPMultiplexedServer(server_descriptor, clients, 0, argv[1]);
}
