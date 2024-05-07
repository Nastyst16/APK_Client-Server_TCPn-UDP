# Copyright Nastase Cristian-Gabriel 325CA

---------------------COMMUNICATIONS PROTOCOLS--Homework 2-------------------------

    TCP and UDP client-server application for message management

First of all i will start by telling that i used the laboratory 7 skeleton and
implementation, then modifyed the code to solve the requirements of this homework

My implementation uses TCP protocol and IO multiplexing.

In the file common.h i created the tcp_request and tcp_client structs and made My
implementation using these.
In tcp_client the field named "subscribed_topics" stores