#include "common.h"
#include <string.h> 

#include <sys/socket.h>
#include <sys/types.h>
#include "helpers.h"


// functions inspired from lab 7
int recv_all(int sockfd, void *buffer, size_t len) {
  size_t bytes_received = 0;
  size_t bytes_remaining = len;
  size_t res;
  char *buff = buffer;

  do {
    res = recv(sockfd, buff + bytes_received, bytes_remaining - bytes_received, 0);
    DIE(res < 0, "send");
    bytes_received += res;
  } while (res && bytes_received < bytes_remaining);

  return bytes_received;
}

int send_all(int sockfd, void *buffer, size_t len) {
  size_t bytes_sent = 0;
  size_t bytes_remaining = len;
  size_t res;
  char *buff = buffer;

  do {
    res = send(sockfd, buff + bytes_sent, bytes_remaining - bytes_sent, 0);
    DIE(res < 0, "recv");
    bytes_sent += res;
  } while (res && bytes_sent < bytes_remaining);

  return bytes_sent;
}

// this function is used to print debug messages
// very helpfull and considering not deleting it
void debug(char *message, int value) {

	FILE *f = fopen("debug.txt", "a");

	if (message != NULL) {
		// fprintf(f, "%s", message);

    // maybe message has not \0 at the final
    for (int i = 0; i < strlen(message); i++) {
      fprintf(f, "%c", message[i]);
    }

	}

	if (value != -1) {
		fprintf(f, "%d", value);
	}

	fprintf(f, "\n");
	fclose(f);
}
