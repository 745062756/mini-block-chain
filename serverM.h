#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>


#define localhost "127.0.0.1"

#ifndef MAX
#define MAX(a,b) (((a)>=(b))? (a):(b))
#endif

#define True 1
#define False 0

struct obj {
    int sequence;
    char sender[256];
    char receiver[256];
    int amount;
    struct obj* next;
    struct obj* prev;
    int head;
};

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

// request code and operation code
#define FindPerson 1
#define FetchRecord 2
#define PushRecord 3
#define FetchCurMax 4
#define FetchList 5

// status code
#define ERR (-1)
#define Success 0

#ifdef SERVER_M
#include "Yida_HashMap.h"
#define PORT_NUM 24777
#define PORT_TCP_A 25777
#define PORT_TCP_B 26777
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
//request code
#define CheckWallet 1
#define TresCOIN 2
#define TxLIST 3
#define Rank 4
// error code (only if statusCode == ERR)
#define senderNotMember 2
#define receiverNotMember 3
#define bothNotMember 4
#define insufficientFund 5

#define backendA 1
#define backendB 2
#define backendC 3

typedef struct dataBlock {
    char usrName[256];
    int transFreq;
    int amount;
    struct dataBlock* next;
} dataBlock;

typedef struct stringMSG {
    int theEnd;
    char msg[1024];
} stringMSG;
#endif

#ifdef SERVER_A
#define PORT_NUM 21777
int fd;
#endif


#ifdef SERVER_B
#define PORT_NUM 22777
int fd;
#endif

#ifdef SERVER_C
#define PORT_NUM 23777
int fd;
#endif