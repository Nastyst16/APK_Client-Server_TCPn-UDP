#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);
void debug(char *message, int value);

/* Dimensiunea maxima a mesajului */
#define MSG_MAXSIZE 1500

struct chat_packet {
  uint16_t len;
  char message[50 + 1 + MSG_MAXSIZE + 1];
};

#endif

enum request_type {
  SUBSCRIBE = 0,
  UNSUBSCRIBE = 1,
  CONNECT = 2,
  EXIT = 3
};

typedef struct tcp_request tcp_request;

struct tcp_request {
  char client_id[10];

  char client_ip[50];
  int client_port;

  uint8_t request_type;
  uint32_t topic_len;
  char topic[50];
};

typedef struct tcp_client tcp_client;

struct tcp_client {
  int sockfd;
  char id[10];
  int connected;
  
  // list of topics where the client is subscribed
  char subscribed_topics[100][50];

  // the ip and port of the client
  char ip[50];
  int port;
};


enum data_type {
  INT = 0,
  SHORT_REAL = 1,
  FLOAT = 2,
  STRING = 3
};
