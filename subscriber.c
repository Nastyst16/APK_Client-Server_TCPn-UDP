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

double power(double base, int exponent) {
    double product = 1.0;

    // invert the base and the exponent if the exponent is negative
    if (exponent < 0) {
        base = 1.0 / base;
        exponent = -exponent;
    }

    // restlt = base^exponent
    for (int i = 0; i < exponent; i++) {
        product *= base;
    }

    return product;
}

// following the instructions from page 4 of the homework pdf
void print_int(char *topic, char *message) {

  uint8_t sign = message[0];
  message += 1;

  uint32_t integer = ntohl(*(uint32_t *)message);
  if (sign)
    integer = -integer;

  printf("%s - INT - %d\n", topic, integer);
}

// following the instructions from page 4 of the homework pdf
void print_shortReal(char *topic, char *message) {

  uint16_t shortReal = ntohs(*(uint16_t *)message);
  float shortRealFloat = shortReal / 100.0;

  printf("%s - SHORT_REAL - %.2f\n", topic, shortRealFloat);
}

// following the instructions from page 4 of the homework pdf
void print_float(char *topic, char *message) {

  uint8_t sign = message[0];
  message += 1;

  float nr = ntohl(*(uint32_t *)message);
  message += sizeof(uint32_t);

  uint8_t powerValue = message[0];

  if (sign)
    nr = -nr;

  printf("%s - FLOAT - %f\n", topic, nr / (float)power(10, powerValue));
}

void print_string(char *topic, char *message) {
  printf("%s - STRING - %s\n", topic, message);
}

// printing this format: <TOPIC> - <TIP_DATE> - <VALOARE_MESAJ>
void printing_subscription(char *topic_dataType_content) {

  char topic[51];
  memset(topic, 0, 50);
  memcpy(topic, topic_dataType_content, 50);
  topic[50] = '\0';

  topic_dataType_content += 50;

  uint8_t dataType = topic_dataType_content[0];
  topic_dataType_content += 1;

  switch (dataType) {
    case INT:
      print_int(topic, topic_dataType_content);
      break;

    case SHORT_REAL:
      print_shortReal(topic, topic_dataType_content);
      break;

    case FLOAT:
      print_float(topic, topic_dataType_content);
      break;

    case STRING:
      print_string(topic, topic_dataType_content);
      break;

    default:
      break;
  }
}



void run_client(int sockfd, char *argv[]) {
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);
  int rc;

  tcp_request request;
  memset(&request, 0, sizeof(tcp_request));
  strcpy(request.client_id, argv[1]);
  request.request_type = CONNECT;
  strcpy(request.client_ip, argv[2]);
  request.client_port = atoi(argv[3]);

  // sennding the request to the server
  send_all(sockfd, &request, sizeof(tcp_request));

  struct pollfd poll_fds[MAX_CONNECTIONS];
  int num_sockets = 2;

  // add the stdin and the socket to the poll_fds
  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[0].events = POLLIN;

  poll_fds[1].fd = sockfd;
  poll_fds[1].events = POLLIN;

  while (1) {
    // waiting for an event on one of the sockets
    rc = poll(poll_fds, num_sockets, -1);
    DIE(rc < 0, "poll");

    for (int i = 0; i < num_sockets; i++) {
      if (poll_fds[i].revents & POLLIN) {
        if (i == 0) {

          memset(buf, 0, sizeof(buf));
          fgets(buf, MSG_MAXSIZE, stdin);

          if (strncmp(buf, "exit", 4) == 0) {
            request.request_type = EXIT;
            send_all(sockfd, &request, sizeof(tcp_request));
            return;

          } else if (strncmp(buf, "subscribe", 9) == 0) {
            request.request_type = SUBSCRIBE;
            printf("Subscribed to topic.\n");

          } else if (strncmp(buf, "unsubscribe", 11) == 0) {
            request.request_type = UNSUBSCRIBE;
            printf("Unsubscribed from topic.\n");

          } else {
            printf("Invalid command.\n");
            continue;
          }

          char *token = strtok(buf, " ");
          token = strtok(NULL, " ");
          strcpy(request.topic, token);
          request.topic_len = strlen(request.topic);

          send_all(sockfd, &request, sizeof(tcp_request));


        } else if (i == 1) {

          struct chat_packet received_packet;
          memset(&received_packet, 0, sizeof(struct chat_packet));

          int rc = recv_all(sockfd, &received_packet, sizeof(struct chat_packet));
          DIE(rc <= 0, "recv_all received_packet");

          printing_subscription(received_packet.message);
        }
      }
    }
  }
}


// this is the main function copied from the laboratory 7 skel
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
