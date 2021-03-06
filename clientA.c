#define CLIENT_A

#include "clientA.h"

static void connectionSetup();
static void balanceHandler(const char*);
static void transHandler(const char* , const char* , int );
static void listHandler();
static void statHandler(const char*);

int main(int argc, char* argv[]) {
   printf("The client A is up and running.\n");
   connectionSetup();
   if (argc==2) {
      if (strcmp(argv[1], "TXLIST")==0) listHandler();
      else balanceHandler(argv[1]);
   } else if (argc==3) statHandler(argv[1]);
   else if (argc==4) transHandler(argv[1], argv[2], atoi(argv[3]));
   close(fd);
   return 0;
}

static
void connectionSetup() {
   struct sockaddr_in targetSockAddress;
   targetSockAddress.sin_family = AF_INET;
   targetSockAddress.sin_port = htons(TargetPort);
   inet_pton(AF_INET, localhost, &targetSockAddress.sin_addr);

   fd = socket(PF_INET, SOCK_STREAM, 0);
   if (connect(fd, (struct sockaddr*) &targetSockAddress, sizeof targetSockAddress)<0) {
      perror("Client connection fails");
      exit(1);
   }
}

static
void balanceHandler(const char* userName) {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'A';
   clientMSG.requestCode = CheckWallet;
   strcpy(clientMSG.senderName, userName);

   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("%s sent a balance enquiry request to the main server.\n", clientMSG.senderName);

   struct serverMMSG reply;
   recv(fd, &reply, sizeof reply, 0);

   if (reply.statusCode == ERR) {
      printf("Unable to proceed with the request as %s is not part of the network.\n", reply.senderName);
   } else {
      printf("The current balance of %s is %d coins.\n", reply.senderName, reply.balance);
   }
}

static
void transHandler(const char* senderName, const char* receiverName, int amount) {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'A';
   clientMSG.requestCode = TresCOIN;
   strcpy(clientMSG.senderName, senderName);
   strcpy(clientMSG.receiverName, receiverName);
   clientMSG.amount = amount;

   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("%s has requested to transfer %d coins to %s.\n", clientMSG.senderName, amount, clientMSG.receiverName);

   struct serverMMSG reply;
   recv(fd, &reply, sizeof reply, 0);

   if (reply.statusCode == ERR) {
      if (reply.errorCode==insufficientFund) printf("%s was unable to transfer %d coins to %s because of insufficient balance.\nThe current balance of %s is : %d coins.\n", reply.senderName, reply.amount, reply.receiverName, reply.senderName, reply.balance);
      else if (reply.errorCode==senderNotMember) printf("Unable to proceed with the transaction as %s is not part of the network.\n", reply.senderName);
      else if (reply.errorCode==receiverNotMember) printf("Unable to proceed with the transaction as %s is not part of the network.\n", reply.receiverName);
      else if (reply.errorCode==bothNotMember) printf("Unable to proceed with the transaction as %s and %s are not part of the network.\n", reply.senderName, reply.receiverName);
   } else {
      printf("%s successfully transferred %d coins to %s.\nThe current balance of %s is : %d coins.\n", reply.senderName, reply.amount, reply.receiverName, reply.senderName, reply.balance);
   }
}

static
void listHandler() {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'A';
   clientMSG.requestCode = TxLIST;
   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("ClientA sent a sorted list request to the main server.\n");
}

static
void statHandler(const char* usrName) {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'A';
   clientMSG.requestCode = Rank;
   strcpy(clientMSG.senderName, usrName);
   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("%s sent a statistics enquiry request to the main server.\n", usrName);

   // print result
   printf("%s statistics are the following:\n", usrName);
   stringMSG buf;

   while(1) {
      recv(fd, &buf, sizeof buf, 0);
      if (buf.theEnd==True) break;
      printf("%s\n", buf.msg);
   }
}