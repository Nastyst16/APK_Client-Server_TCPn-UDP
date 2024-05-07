# Copyright Nastase Cristian-Gabriel 325CA

---------------------COMMUNICATIONS PROTOCOLS--Homework 2--------------------------

    TCP and UDP client-server application for message management

First of all i will start by telling that i used the laboratory 7 skeleton and
implementation, then modifyed the code to solve the requirements of this homework

My implementation uses TCP protocol and IO multiplexing.

In the file common.h i created the tcp_request and tcp_client structs and made my
implementation using these.

In tcp_client the field named "subscribed_topics" a line stores a topic

1. subscriber.c

    - main copied from laboratory 7
    - run_client function that:
        - uses IO multiplexing: poll_fds stores STDIN_FILENO and sockfd
        - has a while that accepts only the following commands:
          exit, subscribe and unsubscribe; every other commands it displays
          "Invalid command.".
        - client receives packets from the server with the topics that he
          is subscribed -> this is managed by function "printing_subscription"
          where every case is treated: INT, SHORT_REAL, FLOAT, STRING

2. server.c

    - main copied from laboratory 7
    - run_chat_multi_server function that:
        - uses IO multiplexing: listenfd which is the server socket
          fd_udp_client which is the socket for the udp client, STDIN_FILENO
          and the clients sockets that are connected to the server. the poll
          updates every time a client connects or disconnects
        - has a while that treats every case of the poll_fds:
          - if the server receives a new client, it stores it in the clients
            array and adds it to the poll_fds (disabling the Nagle's algorighm)
          - if the server receives a request from the udp client, it sends
            the message to the clients that are subscribed to the topic
          - if the server receives a message from stdin accepts only
            the "exit" command and for the other commands it displays
            "Invalid command."
          - if the server receives a request from clients it accepts
            "CONNECT" -> which connects a client -> stores it if is new
            "SUBSCRIBE" -> searching for the corresponding client who subscribed
            and updating his topic list of subscriptions
            "UNSUBSCRIBE" -> searching for the corresponding client who
            unsubscribed and deleting the subscription from his list
            "EXIT" -> closing the connection for the client and updating
            the poll_fds array
    - wildcards function which i tried implement but failed. it works partially :)

3. common.c
    - recv_all -> from laboratory
    - send_all -> from laboratory

-----------------------------------------------------------------------------------

