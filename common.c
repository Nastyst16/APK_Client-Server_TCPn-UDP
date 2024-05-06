#include "common.h"
#include <string.h> 

#include <sys/socket.h>
#include <sys/types.h>
// #include "../common/include/utils.h"
#include "helpers.h"

/*
    TODO 1.1: Rescrieți funcția de mai jos astfel încât ea să facă primirea
    a exact len octeți din buffer.
*/
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

  /*
    TODO: Returnam exact cati octeti am citit
  */
  return bytes_received;
}

/*
    TODO 1.2: Rescrieți funcția de mai jos astfel încât ea să facă trimiterea
    a exact len octeți din buffer.
*/

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


  /*
    TODO: Returnam exact cati octeti am trimis
  */
  return bytes_sent;
}

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
