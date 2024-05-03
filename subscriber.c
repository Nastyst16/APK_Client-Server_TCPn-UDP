/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP si mulplixare
 * client.c
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "helpers.h"
#define MAX_CONNECTIONS 32

void run_client(int sockfd, char *argv[]) {
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);
  int rc;

  // struct chat_packet sent_packet;
  // struct chat_packet recv_packet;

  tcp_request request;
  memset(&request, 0, sizeof(tcp_request));
  strcpy(request.client_id, argv[1]);
  request.request_type = CONNECT;
  strcpy(request.client_ip, argv[2]);
  request.client_port = atoi(argv[3]);

  // Trimitem requestul la server
  send_all(sockfd, &request, sizeof(tcp_request));



  /*
    TODO 2.2: Multiplexati intre citirea de la tastatura si primirea unui
    mesaj, ca sa nu mai fie impusa ordinea.
    
    Hint: server::run_multi_chat_server
  */
  struct pollfd poll_fds[MAX_CONNECTIONS];
  int num_sockets = 2;

  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[0].events = POLLIN;

  poll_fds[1].fd = sockfd;
  poll_fds[1].events = POLLIN;

  while (1) {
    // Asteptam sa primim ceva pe unul dintre cei num_sockets socketi
    rc = poll(poll_fds, num_sockets, -1);
    DIE(rc < 0, "poll");

    for (int i = 0; i < num_sockets; i++) {
      if (poll_fds[i].revents & POLLIN) {
        if (i == 0) {

          memset(buf, 0, sizeof(buf));
          fgets(buf, MSG_MAXSIZE, stdin);
          
          if(isspace(buf[0])) {
            continue;
          }


          if (strncmp(buf, "exit", 4) == 0) {
            request.request_type = EXIT;
            send_all(sockfd, &request, sizeof(tcp_request));
            return;
          }

/// pentru subscribe si unsubscripe poti sa le pui la comun, dupa verifici daca e subscribe sau unsubscribe
/// ca sa nu ai cod duplicat
          if (strncmp(buf, "subscribe", 9) == 0) {
            request.request_type = SUBSCRIBE;
            char *token = strtok(buf, " ");
            token = strtok(NULL, " ");
            strcpy(request.topic, token);
            request.topic_len = strlen(request.topic);
            send_all(sockfd, &request, sizeof(tcp_request));

            printf("Subscribed to topic.\n");
          }

          if (strncmp(buf, "unsubscribe", 11) == 0) {
            request.request_type = UNSUBSCRIBE;
            char *token = strtok(buf, " ");
            token = strtok(NULL, " ");
            strcpy(request.topic, token);
            request.topic_len = strlen(request.topic);
            send_all(sockfd, &request, sizeof(tcp_request));

            printf("Unsubscribed from topic.\n");
          }

        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("\n Usage: %s <ip> <port>\n", argv[0]);
    return 1;
  }

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[3], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtinem un socket TCP pentru conectarea la server
  const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // Ne conectăm la server
  rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "connect");

  run_client(sockfd, argv);

  // Inchidem conexiunea si socketul creat
  close(sockfd);

  return 0;
}
