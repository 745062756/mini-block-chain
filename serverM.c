#define SERVER_M

#include "serverM.h"


static int fdChild;
static struct sockaddr_in ChildSocketAddress;
static socklen_t ChildSocketAddressSize = sizeof ChildSocketAddress;

static void setupParentSocket(int*, int*);
static void setupUDPSocket(int *);
static void clientHandler(const int*);
static void talkBackend(int, const int*, const struct request*, struct response*);

int curTCP;
int curMax;

int main() {
   srand(time(0));
   printf("The main server is up and running.\n");
   int fdA, fdB, myFd;
   setupParentSocket(&fdA, &fdB);
   setupUDPSocket(&myFd);

   while (1) {
      if ((fdChild = accept(fdA, (struct sockaddr*) & ChildSocketAddress, &ChildSocketAddressSize)) <0) {
         perror("Server M fails to accept client A");
         exit(1);
      }
      curTCP = PORT_TCP_A;
      clientHandler(&myFd);
      close(fdChild);

      if ((fdChild = accept(fdB, (struct sockaddr*) & ChildSocketAddress, &ChildSocketAddressSize)) <0) {
         perror("Server M fails to accept client B");
         exit(1);
      }
      curTCP = PORT_TCP_B;
      clientHandler(&myFd);
      close(fdChild);
   }
}

static
void setupParentSocket(int *fdA, int *fdB) {
   // for client A
   struct sockaddr_in socketAAddress;
   socketAAddress.sin_family = AF_INET;
   socketAAddress.sin_port = htons(PORT_TCP_A);
   inet_pton(AF_INET, localhost, &socketAAddress.sin_addr);

   *fdA = socket(PF_INET, SOCK_STREAM, 0);
   if (bind(*fdA, (struct sockaddr *) &socketAAddress, sizeof socketAAddress)<0) {
      char warning[40];
      sprintf(warning, "Server M fails to bind port to %d", PORT_TCP_A);
      perror(warning);
      exit(1);
   }
   listen(*fdA, 5);

   // for client B
   struct sockaddr_in socketBAddress;
   socketBAddress.sin_family = AF_INET;
   socketBAddress.sin_port = htons(PORT_TCP_B);
   inet_pton(AF_INET, localhost, &socketBAddress.sin_addr);

   *fdB = socket(PF_INET, SOCK_STREAM, 0);
   if (bind(*fdB, (struct sockaddr *) &socketBAddress, sizeof socketBAddress)<0) {
      char warning[40];
      sprintf(warning, "Server M fails to bind port to %d", PORT_TCP_B);
      perror(warning);
      exit(1);
   }
   listen(*fdB, 5);
}

static void setupUDPSocket(int* myFd) {
   *myFd = socket(PF_INET, SOCK_DGRAM, 0);
   struct sockaddr_in myAddress;
   myAddress.sin_family = AF_INET;
   myAddress.sin_port = htons(PORT_NUM);
   inet_pton(AF_INET, localhost, &myAddress.sin_addr);

   if (bind(*myFd, (struct sockaddr *) &myAddress, sizeof myAddress)<0) {
      char warning[40];
      sprintf(warning, "Server M fails to bind port to %d", PORT_NUM);
      perror(warning);
      exit(1);
   }
}

static
void talkBackend(int backend, const int* myFd, const struct request* request, struct response* response) {
   char whichServer;
   struct sockaddr_in backendAddress;
   backendAddress.sin_family = AF_INET;
   if (backend==backendA) {
      whichServer = 'A';
      backendAddress.sin_port = htons(21777);
   } else if (backend==backendB) {
      whichServer = 'B';
      backendAddress.sin_port = htons(22777);
   } else if (backend==backendC) {
      whichServer = 'C';
      backendAddress.sin_port = htons(23777);
   }
   inet_pton(AF_INET, localhost, &backendAddress.sin_addr);
   socklen_t backendAddressSize = sizeof backendAddress;

   sendto(*myFd, request, sizeof *request, 0, (const struct sockaddr *)&backendAddress, backendAddressSize);
   if (request->requestCode==FetchRecord || request->requestCode==PushRecord) printf("The main server sent a request to server %c.\n", whichServer);

   recvfrom(*myFd, response, sizeof *response, 0, (struct sockaddr *)&backendAddress, &backendAddressSize);
   if (request->requestCode==FetchRecord) printf("The main server received transactions from Server %c using UDP over port %d.\n", whichServer, PORT_NUM);
   if (request->requestCode==PushRecord) printf("The main server received the feedback from server %c using UDP over port %d.\n", whichServer,  PORT_NUM);
}

static
int peekMember(const int* myFd, const char* name, int server) {
   struct request request;
   request.requestCode = FindPerson;
   strcpy(request.sender, name);

   struct response response;
   talkBackend(server, myFd, &request, &response);
   return response.statusCode;
}

static
int checkBalance(const int* myFd, const char* name) {
   struct request request;
   request.requestCode = FetchRecord;
   strcpy(request.sender, name);

   struct response response;

   int balance=1000;
   int exist=ERR;

   // query server A
   if (peekMember(myFd, name, backendA)==Success) {
      talkBackend(backendA, myFd, &request, &response);
      balance+= response.netGain;
      exist = Success;
   }

   // query B
   if (peekMember(myFd, name, backendB)==Success) {
      talkBackend(backendB, myFd, &request, &response);
      balance+= response.netGain;
      exist = Success;
   }

   // query C
   if (peekMember(myFd, name, backendC)==Success) {
      talkBackend(backendC, myFd, &request, &response);
      balance+= response.netGain;
      exist = Success;
   }
   if (exist==ERR) return ERR;
   return balance;
}

static
void clientHandler(const int* myFd) {
   struct clientMSG clientMSG;
   recv(fdChild, &clientMSG, sizeof clientMSG, 0);

   struct serverMMSG reply;
   strcpy(reply.senderName, clientMSG.senderName);
   strcpy(reply.receiverName, clientMSG.receiverName);

   if (clientMSG.requestCode==CheckWallet) {
      printf("The main server received input=%s from the client using TCP over port %d.\n", clientMSG.senderName, curTCP);
      int result;
      if ((result = checkBalance(myFd, clientMSG.senderName))==ERR) {
         reply.statusCode = ERR;
      } else {
         reply.statusCode = Success;
         reply.balance = result;
      }
   }

   if (clientMSG.requestCode==TresCOIN) {
      printf("The main server received from %s to transfer %d coins to %s using TCP over port %d.\n", clientMSG.senderName, clientMSG.amount, clientMSG.receiverName, curTCP);
      // check member in network
      int senderIn=ERR;
      int receiverIn=ERR;

      if (peekMember(myFd, clientMSG.senderName, backendA)==Success || peekMember(myFd, clientMSG.senderName, backendB)==Success || peekMember(myFd, clientMSG.senderName, backendC)==Success) {
         senderIn = Success;
      }

      if (peekMember(myFd, clientMSG.receiverName, backendA)==Success || peekMember(myFd, clientMSG.receiverName, backendB)==Success || peekMember(myFd, clientMSG.receiverName, backendC)==Success) {
         receiverIn = Success;
      }

      if (senderIn==ERR && receiverIn==ERR) {
         reply.statusCode = ERR;
         reply.errorCode=bothNotMember;
      } else if (senderIn==ERR) {
         reply.statusCode = ERR;
         reply.errorCode=senderNotMember;
      } else if (receiverIn==ERR) {
         reply.statusCode = ERR;
         reply.errorCode=receiverNotMember;
      } else {
         // both in network, check sufficient balance
         int curBalance;
         if ((curBalance = checkBalance(myFd, clientMSG.senderName)) < clientMSG.amount) {
            reply.statusCode = ERR;
            reply.errorCode=insufficientFund;
            reply.amount = clientMSG.amount;
            reply.balance = curBalance;
         } else {
            // push transaction
            struct request queryMax;
            queryMax.requestCode = FetchCurMax;
            struct response curMaxRes;
            for (int i=1; i<4;i++) {
               talkBackend(i, myFd, &queryMax, &curMaxRes);
               curMax = MAX(curMax, curMaxRes.curMax);
            }

            struct request pushRequest;
            pushRequest.requestCode = PushRecord;
            strcpy(pushRequest.sender, clientMSG.senderName);
            strcpy(pushRequest.receiver, clientMSG.receiverName);
            pushRequest.amount = clientMSG.amount;
            pushRequest.nextSeq = curMax+1;

            struct response response;
            talkBackend(rand()%3+1, myFd, &pushRequest, &response);

            reply.statusCode = Success;
            reply.balance = checkBalance(myFd, clientMSG.senderName);
            reply.amount = clientMSG.amount;
         }
      }
   }
   send(fdChild, &reply, sizeof reply, 0);
   if (clientMSG.requestCode==CheckWallet) printf("The main server sent the current balance to client %c.\n", clientMSG.clientID);
   if (clientMSG.requestCode==TresCOIN) printf("The main server sent the result of the transaction to client %c.\n", clientMSG.clientID);
}