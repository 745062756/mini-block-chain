# **Student Profile:**
Name: Yida Tong

ID: 3807206777

Finished: phase 1, 2, 3 and 4


# **Abstract:**

The blockchain consists of a central server (ServerM), 3 backend servers (ServerA, ServerB, ServerC), 2 clients (clientA, clientB).
Client starts a TCP connection with central server M, which will query info with help of backend servers via UDP connections. 
About each backend server, they each manage a text file as a backing store all history of money transactions. Client is able 
to check wallet balance, request money transfer to another client, ask central server to download all transactions and stat summary
of all transactions related with certain user. 


# File Structures:
Self coded HashTable: 
1. Yida_HashMap.c (Yida_HashMap.h)

(serverM.h is the header file for all servers)
1. serverM.c
2. serverA.c
3. serverB.c
4. serverC.c


(clientA.h is the header file for client A and B)
1. clientA.c
2. clientB.c

# Code Copy:
`uint32_t jenkins_one_at_a_time_hash(const uint8_t*, size_t)`

Hash function Source: [https://en.wikipedia.org/wiki/Jenkins_hash_function](https://en.wikipedia.org/wiki/Jenkins_hash_function)

# Usage: 
Compile: make all

Run servers in this order: ./serverM ./serverA ./serverB ./serverC (to stop, press ctrl+c)

**(Please run clientA and clientB interchangeably)**
1. To check balance: ./clientA <user name> or ./clientB <user name>
2. To request amount transfer: ./clientA <sender name> <receiver name> <amount>  or ./clientB <sender name> <receiver name> <amount>
3. To download a sorted transaction history from beginning: ./clientA <TXLIST> or ./clientB <TXLIST>
4. To view transfer frequency report at client console: ./clientA <user name> stats


# Implementation Strategy:

I created several fixed size data structure to transfer over TCP/UDP: 

Between client and central server:
```
struct clientMSG {
char clientID;
int requestCode;
char senderName[256];
char receiverName[256];
int amount;
};

struct serverMMSG {
int statusCode;
int errorCode;
int balance;
int amount;
char senderName[256];
char receiverName[256];
};
```
Between central server and backend servers:
```
struct request {
    int requestCode;
    char sender[256];
    char receiver[256];
    int amount;
    int nextSeq;
};

struct response {
    int statusCode;
    int netGain;
    int curMax;
    int atEnd;
    struct obj objItem;
};
```
Assuming your length of name doesn't exceed 255 bytes!

# Project summary: 
Finish phase 1, 2, 3 and 4