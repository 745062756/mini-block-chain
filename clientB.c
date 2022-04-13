#define CLIENT_B

#include "clientA.h"


static void connectionSetup();
static void balanceHandler(const char*);
static void transHandler(const char* , const char* , int );

int main(int argc, char* argv[]) {
   printf("The client B is up and running.\n");
   connectionSetup();
   if (argc==2) {
      balanceHandler(argv[1]);
   } else if (argc==4) {
      transHandler(argv[1], argv[2], atoi(argv[3]));
   }
   close(fd);
   return 0;
}

static
void connectionSetup() {
   struct sockaddr_in targetSockBddress;
   targetSockBddress.sin_family = AF_INET;
   targetSockBddress.sin_port = htons(TargetPort);
   inet_pton(AF_INET, localhost, &targetSockBddress.sin_addr);

   fd = socket(PF_INET, SOCK_STREAM, 0);
   if (connect(fd, (struct sockaddr*) &targetSockBddress, sizeof targetSockBddress)<0) {
      perror("Client connection fails");
      exit(1);
   }
}

static
void balanceHandler(const char* userName) {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'B';
   clientMSG.requestCode = CheckWallet;
   strcpy(clientMSG.senderName, userName);

   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("%s sent a balance enquiry request to the main server.\n", clientMSG.senderName);

   struct serverMMSG reply;
   recv(fd, &reply, sizeof reply, 0);

   if (reply.statusCode == ERR) {
      printf("Unable to proceed with the request as %s is not part of the network.\n", reply.senderName);
   } else {
      printf("The current balance of %s is %d alicoins.\n", reply.senderName, reply.balance);
   }
}

static
void transHandler(const char* senderName, const char* receiverName, int amount) {
   struct clientMSG clientMSG;
   clientMSG.clientID = 'B';
   clientMSG.requestCode = TresCOIN;
   strcpy(clientMSG.senderName, senderName);
   strcpy(clientMSG.receiverName, receiverName);
   clientMSG.amount = amount;

   send(fd, &clientMSG, sizeof clientMSG, 0);
   printf("%s has requested to transfer %d coins to %s.\n", clientMSG.senderName, amount, clientMSG.receiverName);

   struct serverMMSG reply;
   recv(fd, &reply, sizeof reply, 0);

   if (reply.statusCode == ERR) {
      if (reply.errorCode==insufficientFund) printf("%s was unable to transfer %d alicoins to %s because of insufficient balance.\nThe current balance of %s is : %d alicoins.\n", reply.senderName, reply.amount, reply.receiverName, reply.senderName, reply.balance);
      else if (reply.errorCode==senderNotMember) printf("Unable to proceed with the transaction as %s is not part of the network.\n", reply.senderName);
      else if (reply.errorCode==receiverNotMember) printf("Unable to proceed with the transaction as %s is not part of the network.\n", reply.receiverName);
      else if (reply.errorCode==bothNotMember) printf("Unable to proceed with the transaction as %s and %s are not part of the network.\n", reply.senderName, reply.receiverName);
   } else {
      printf("%s successfully transferred %d alicoins to %s.\nThe current balance of %s is : %d alicoins.\n", reply.senderName, reply.amount, reply.receiverName, reply.senderName, reply.balance);
   }
}
