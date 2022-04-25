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

#define localhost "127.0.0.1"

#define True 1
#define False 0

int fd;

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

// status code
#define ERR (-1)
#define Success 0
// error code (only if statusCode == ERR)
#define senderNotMember 2
#define receiverNotMember 3
#define bothNotMember 4
#define insufficientFund 5

typedef struct stringMSG {
    int theEnd;
    char msg[1024];
} stringMSG;

#ifdef CLIENT_A
#define TargetPort 25777
#endif

#ifdef CLIENT_B
#define TargetPort 26777
#endif
