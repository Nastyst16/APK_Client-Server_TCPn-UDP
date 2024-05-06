/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include "common.h"
#include "helpers.h"

#include <netinet/tcp.h> // oare am voie???????


#define MAX_CONNECTIONS 32

#define IP "127.0.0.1"

int ok_debug = 0;


tcp_client clients[MAX_CONNECTIONS];
int clients_count = 0;
// char topics[50][50]; // pe fiecare linie se afla un topic
////// inca ceva aici

int wildcards(char *topic, char *client_topic) {


  int string_ok = 0;

  int j = 0, k = 0;
  while (j < strlen(topic) && k < strlen(client_topic)) {

    if (topic[j] == client_topic[k]) {
        j++;
        k++;
        continue;
    }

    if (client_topic[k] == '*') {
        k++;
        if (client_topic[k] == '/')
            k++;

      while (client_topic[k] != topic[j] && j < strlen(topic)) {
          j++;
      }

    }

    if (client_topic[k] == '+') {
      k++;

      while (topic[j] != '/' && j < strlen(topic)) {
          j++;
      }
    }

    if (client_topic[k] != topic[j]) {
      break;
    }

  }

  if (k == strlen(client_topic) && j == strlen(topic)) {
    string_ok = 1;
  }
  

  return string_ok;
}


// Primeste date de pe connfd1 si trimite mesajul receptionat pe connfd2
int receive_and_send(int connfd1, int connfd2, size_t len) {
  int bytes_received;
  char buffer[len];

  // Primim exact len octeti de la connfd1
  bytes_received = recv_all(connfd1, buffer, len);
  // S-a inchis conexiunea
  if (bytes_received == 0) {
    return 0;
  }
  DIE(bytes_received < 0, "recv");

  // Trimitem mesajul catre connfd2
  int rc = send_all(connfd2, buffer, len);
  if (rc <= 0) {
    perror("send_all");
    return -1;
  }

  return bytes_received;
}

void run_chat_server(int listenfd) {
  struct sockaddr_in client_addr1;
  struct sockaddr_in client_addr2;
  socklen_t clen1 = sizeof(client_addr1);
  socklen_t clen2 = sizeof(client_addr2);

  int connfd1 = -1;
  int connfd2 = -1;
  int rc;

  // Setam socket-ul listenfd pentru ascultare
  rc = listen(listenfd, 2);
  DIE(rc < 0, "listen");

  // Acceptam doua conexiuni
  printf("Astept conectarea primului client...\n");
  connfd1 = accept(listenfd, (struct sockaddr *)&client_addr1, &clen1);
  DIE(connfd1 < 0, "accept");

  printf("Astept connectarea clientului 2...\n");
  connfd2 = accept(listenfd, (struct sockaddr *)&client_addr2, &clen2);
  DIE(connfd2 < 0, "accept");

  while (1) {
    printf("Primesc de la 1 si trimit catre 2...\n");
    int rc = receive_and_send(connfd1, connfd2, sizeof(struct chat_packet));
    if (rc <= 0) {
      break;
    }

    printf("Primesc de la 2 si trimit catre 1...\n");
    rc = receive_and_send(connfd2, connfd1, sizeof(struct chat_packet));
    if (rc <= 0) {
      break;
    }
  }

  // Inchidem conexiunile si socketii creati
  close(connfd1);
  close(connfd2);
}

void run_chat_multi_server(int listenfd, int fd_udp_client) {

  struct pollfd poll_fds[MAX_CONNECTIONS];
  int num_sockets = 3;
  int rc;
  
  struct chat_packet sending_packet;
  struct chat_packet received_packet;

  // Setam socket-ul listenfd pentru ascultare
  rc = listen(listenfd, MAX_CONNECTIONS);
  DIE(rc < 0, "listen");

  // Adaugam noul file descriptor (socketul pe care se asculta conexiuni) in
  // multimea poll_fds
  poll_fds[0].fd = listenfd;
  poll_fds[0].events = POLLIN;

  poll_fds[1].fd = fd_udp_client;
  poll_fds[1].events = POLLIN;

  poll_fds[2].fd = STDIN_FILENO;
  poll_fds[2].events = POLLIN;

  while (1) {
    // Asteptam sa primim ceva pe unul dintre cei num_sockets socketi
    rc = poll(poll_fds, num_sockets, -1);
    DIE(rc < 0, "poll");


    for (int i = 0; i < num_sockets; i++) {

      if (&poll_fds[i] == NULL) {
        debug("poll_fds[i] is NULL", -1);
        continue;
      }

      if (poll_fds[i].revents & POLLIN) {
        if (poll_fds[i].fd == listenfd) {
          // Am primit o cerere de conexiune pe socketul de listen, pe care
          // o acceptam
          struct sockaddr_in cli_addr;
          socklen_t cli_len = sizeof(cli_addr);
          const int newsockfd =
              accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);
          DIE(newsockfd < 0, "erroare la 'accept'");

          // nu stiu daca am nevoie.....
          int enable = 1;
          setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR | TCP_NODELAY, &enable,
                    sizeof(int));
          DIE(newsockfd < 0, "setsockopt() failed");
          /// ......portiunea asta de cod;

          // salvare client
          tcp_client *new_client = malloc(sizeof(tcp_client));
          new_client->sockfd = newsockfd;
          new_client->connected = 0;

          strcpy(new_client->ip, inet_ntoa(cli_addr.sin_addr));
          new_client->port = cli_addr.sin_port;


          // Adaugam noul socket intors de accept() la multimea descriptorilor
          // de citire
          poll_fds[num_sockets].fd = newsockfd;
          poll_fds[num_sockets].events = POLLIN;
          num_sockets++;
          

        } else if (poll_fds[i].fd == fd_udp_client) {

          struct sockaddr_in udp_cli_addr;
          socklen_t udp_cli_len = sizeof(udp_cli_addr);

          char message[50 + 1 + MSG_MAXSIZE + 1];
          memset(message, 0, 50 + 1 + MSG_MAXSIZE + 1);
          rc = recvfrom(fd_udp_client, message, 50 + 1 + MSG_MAXSIZE + 1, 0, (struct sockaddr *)&udp_cli_addr, &udp_cli_len);
          DIE(rc < 0, "recvfrom");

          char topic[50];
          memset(topic, 0, 50);
          memcpy(topic, message, 50);
          // topic[51] = '\0';

          sending_packet.len = rc;
          memcpy(sending_packet.message, message, rc);



///////////// 92 ---> 113 sffff !!!

          for (int j = 0; j < MAX_CONNECTIONS; j++) {
            if (&clients[j] == NULL || clients[j].connected == 0) {
              continue;
            }

            // debug(clients[j].id, -1); 
            // char deb_mes[50];
            // memset(deb_mes, 0, 10);
            // sprintf(deb_mes, "Client %s", clients[j].id);


            int string_ok = 0;

            int q = 0;
            while (strcmp(clients[j].subscribed_topics[q], "") != 0) {

              if (clients[j].subscribed_topics[q] == NULL ||
                strlen(clients[j].subscribed_topics[q]) == 0) {
                continue;
              }

              // debug("subscribed topic", -1);
              // debug(clients[j].subscribed_topics[q], -1);




              if (strchr(clients[j].subscribed_topics[q], '*') != NULL ||
                  strchr(clients[j].subscribed_topics[q], '+') != NULL) {


                char client_topic[50];
                memset(client_topic, 0, 50);
                memcpy(client_topic, clients[j].subscribed_topics[q], 50);
                client_topic[strlen(client_topic) - 1] = '\0'; // because it is \n

                string_ok = wildcards(topic, client_topic);
              }
              
              if (strncmp(clients[j].subscribed_topics[q], topic, strlen(topic)) == 0 || string_ok == 1) {

                rc = send_all(clients[j].sockfd, &sending_packet, sizeof(sending_packet));
                DIE(rc <= 0, "send_all");
              }

              q++;
            }
            
          }

        } else if (poll_fds[i].fd == STDIN_FILENO) {

          char stdin_message[100];
          fscanf(stdin, "%s", stdin_message);

          if (strncmp(stdin_message, "exit", 4) == 0) {
            
            for (int j = 2; j < num_sockets; j++) {
              close(poll_fds[j].fd);
              return;
            }

          }

        } else {

          tcp_request request;
          rc = recv_all(poll_fds[i].fd, &request, sizeof(request));
          DIE(rc <= 0, "recv");

          tcp_client *subs_client = NULL;

          switch(request.request_type) {
            
            case CONNECT:

              int already_connected = 0;
              for (int j = 0; j < MAX_CONNECTIONS; j++) {


                if (strcmp(clients[j].id, request.client_id) == 0 && clients[j].connected == 1) {
                  already_connected = 1;


                  printf("Client %s already connected.\n", request.client_id);
                  close(poll_fds[i].fd);

                  // nu stiu daca e bine
                  for (int j = i; j < num_sockets - 1; j++) {
                    poll_fds[j] = poll_fds[j + 1];
                  }
                  

                  break;
                }
              }

              if (!already_connected) {

                // verifying if the client exists in the client list but not connected

                int found = 0;

                for (int j = 0; j < MAX_CONNECTIONS; j++) {

                  if (&clients[j] == NULL) {
                    continue;
                  }

                  if (strcmp(clients[j].id, request.client_id) == 0 && clients[j].connected == 0) {
                    clients[j].connected = 1;
                    clients[j].sockfd = poll_fds[i].fd;


                    printf("New client %s connected from %s:%d.\n", request.client_id, request.client_ip, request.client_port);

                    found = 1;
                    break;
                  }
                }

                if (found) {
                  break;
                }
                

                // printing the following pattern: New client <ID_CLIENT> connected from IP:PORT.
                printf("New client %s connected from %s:%d.\n", request.client_id, request.client_ip, request.client_port);

                // adaugam clientul in lista de clienti
                tcp_client *new_client = malloc(sizeof(tcp_client));
                new_client->sockfd = poll_fds[i].fd;
                strcpy(new_client->id, request.client_id);
                strcpy(new_client->ip, request.client_ip);
                new_client->port = request.client_port;
                new_client->connected = 1;

                for (int i = 0; i < 100; i++) {
                  for (int j = 0; j < 50; j++) {
                    new_client->subscribed_topics[i][j] = 0;
                  } 
                }


                clients[clients_count] = *new_client;
                clients_count++;

                // adding the client to the poll_fds array
                poll_fds[num_sockets].fd = poll_fds[i].fd;
                poll_fds[num_sockets].events = POLLIN;
                num_sockets++;

              }

              break;


            case SUBSCRIBE:
              // adaugam clientul la lista de abonati
              // searching for the client

              char message[100];
              memset(message, 0, 100);


              subs_client = NULL;
              for (int j = 0; j < MAX_CONNECTIONS; j++) {

                if (&clients[j] == NULL || clients[j].connected == 0) {
                  continue;
                }

                if (strcmp(clients[j].id, request.client_id) == 0) {
                  subs_client = &clients[j];
                  break;
                }
              }

              if (subs_client) {

                int q = 0;
                while (strcmp(subs_client->subscribed_topics[q], "") != 0) {
                  q++;
                }

                memcpy(subs_client->subscribed_topics[q], request.topic, strlen(request.topic));

              }


              break;
            case UNSUBSCRIBE:
              // // scoatem clientul din lista de abonati
              // // searching for the client
              subs_client = NULL;
              for (int j = 0; j < MAX_CONNECTIONS; j++) {

                if (&clients[j] == NULL || clients[j].connected == 0) {
                  continue;
                }

                if (strcmp(clients[j].id, request.client_id) == 0) {
                  subs_client = &clients[j];
                  break;
                }
              }

              if (subs_client) {
                int q = 0;
                
                while (strcmp(subs_client->subscribed_topics[q], "") != 0) {

                  // if already subscribed
                  if (strncmp(subs_client->subscribed_topics[q], request.topic, strlen(request.topic)) == 0) {
                    // printf("Client %s unsubscribed from topic %s.\n", subs_client->id, request.topic);
                    // deleting the topic
                    for (int k = q; k < 100; k++) {
                      if (k == 99) {
                        memset(subs_client->subscribed_topics[k], 0, 50);
                      } else {
                        memcpy(subs_client->subscribed_topics[k], subs_client->subscribed_topics[k + 1], 50);
                      }
                    }

                    break;
                  }

                  q++;
                }

              }


              break;
            case EXIT:
              // inchidem conexiunea
              for (int j = 0; j < MAX_CONNECTIONS; j++) {
                if (clients[j].sockfd == poll_fds[i].fd) {

                  printf("Client %s disconnected.\n", clients[j].id);

                  ok_debug = 1;


                  clients[j].connected = 0;
                  close(poll_fds[i].fd);

                  // updating the poll_fds array
                  for (int j = i; j < num_sockets - 1; j++) {
                    poll_fds[j] = poll_fds[j + 1];
                  }
                  --num_sockets;

                  break;
                }
              }

              
              break;


            default:
              break;
                    
            }
        
        }

      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("\n Usage: %s <ip> <port>\n", argv[0]);

    return 1;
  }

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);


  memset(clients, 0, sizeof(clients));

  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[1], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtinem un socket TCP pentru receptionarea conexiunilor
  const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(listenfd < 0, "socket");

  const int fd_udp_client = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(fd_udp_client < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  // Facem adresa socket-ului reutilizabila, ca sa nu primim eroare in caz ca
  // rulam de 2 ori rapid
  // Vezi https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
  const int enable = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, IP, &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");


  // Asociem adresa serverului cu socketul creat folosind bind
  rc = bind(listenfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "bind");

  rc = bind(fd_udp_client, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));

  /*
    TODO 2.1: Folositi implementarea cu multiplexare
  */
  // run_chat_server(listenfd);
  run_chat_multi_server(listenfd, fd_udp_client);

  // Inchidem listenfd
  close(listenfd);

  return 0;
}
