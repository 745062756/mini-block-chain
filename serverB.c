#define SERVER_B

#include "serverM.h"

static void setupSocket();

static int FindPersonHandler(const char*);
static int FetchRecordHandler(const char*);
static void PushRecordHandler(const char*, const char*, int, int);
static void cacheFile();

struct obj anchor;
FILE* filePTR;
int curMax = 0;

int main() {
   printf("The ServerB is up and running using UDP on port %d.\n", PORT_NUM);
   cacheFile();
   setupSocket();

   struct request request;

   struct sockaddr_in serverMAddress;
   socklen_t serverMAddressSize = sizeof serverMAddress;

   struct response response;

   while(1) {
      recvfrom(fd, &request, sizeof request, 0, (struct sockaddr*) &serverMAddress, &serverMAddressSize);
      if (request.requestCode==FindPerson) {
         response.statusCode = FindPersonHandler(request.sender);
      }
      else if (request.requestCode==FetchRecord){
         printf("The ServerB received a request from the Main Server.\n");
         response.netGain = FetchRecordHandler(request.sender);
         response.statusCode = Success;
      }
      else if (request.requestCode==PushRecord) {
         printf("The ServerB received a request from the Main Server.\n");
         response.statusCode = Success;
         PushRecordHandler(request.sender, request.receiver, request.amount, request.nextSeq);
      } else if (request.requestCode==FetchCurMax) {
         response.statusCode = Success;
         response.curMax = curMax;
      } else if (request.requestCode==FetchList) {
         printf("The ServerB received a request from the Main Server.\n");
         response.statusCode = ERR;
         for(struct obj* curPTR = anchor.next; curPTR->head!=1; curPTR=curPTR->next) {
            response.statusCode = Success;
            response.objItem.sequence = curPTR->sequence;
            strcpy(response.objItem.sender, curPTR->sender);
            strcpy(response.objItem.receiver, curPTR->receiver);
            response.objItem.amount = curPTR->amount;
            if (curPTR->next->head==1) {
               response.atEnd = True;
               break;
            }
            response.atEnd = False;
            sendto(fd, &response, sizeof response, 0, (struct sockaddr*) &serverMAddress, serverMAddressSize);
         }
      }
      sendto(fd, &response, sizeof response, 0, (struct sockaddr*) &serverMAddress, serverMAddressSize);
      if (request.requestCode==FetchRecord || request.requestCode==PushRecord || request.requestCode==FetchList) printf("The ServerB finished sending the response to the Main Server.\n");
   }
}

static
void setupSocket() {
   fd = socket(PF_INET, SOCK_DGRAM, 0);
   struct sockaddr_in myAddress;
   myAddress.sin_family = AF_INET;
   myAddress.sin_port = htons(PORT_NUM);
   inet_pton(AF_INET, localhost, &myAddress.sin_addr);

   if (bind(fd, (struct sockaddr *) &myAddress, sizeof myAddress)<0) {
      char warning[40];
      sprintf(warning, "Server B fails to bind port to %d", PORT_NUM);
      perror(warning);
      exit(1);
   }
}

static
int FindPersonHandler(const char* name) {
   for(struct obj* curPTR = anchor.next; curPTR->head!=1; curPTR=curPTR->next) {
      if (strcmp(curPTR->sender, name)==0 || strcmp(curPTR->receiver, name)==0) return Success;
   }
   return ERR;
}

static
int FetchRecordHandler(const char* name) {
   int net = 0;
   for(struct obj* curPTR = anchor.next; curPTR->head!=1; curPTR=curPTR->next) {
      if (strcmp(curPTR->sender, name)==0) net-=curPTR->amount;
      if (strcmp(curPTR->receiver, name)==0) net+= curPTR->amount;
   }
   return net;
}

static
void PushRecordHandler(const char* sender, const char* receiver, int amount, int seq) {
   fprintf(filePTR, "%d %s %s %d\n", seq, sender, receiver, amount);
   fflush(filePTR);
   struct obj* newObjPTR = malloc(sizeof (struct obj));
   newObjPTR->sequence = seq;
   strcpy(newObjPTR->sender, sender);
   strcpy(newObjPTR->receiver, receiver);
   newObjPTR->amount = amount;
   newObjPTR->head=0;
   newObjPTR->next = &anchor;
   newObjPTR->prev = anchor.prev;
   anchor.prev->next = newObjPTR;
   anchor.prev = newObjPTR;
   curMax = seq;
}

static
void cacheFile() {
   anchor.head=1;
   struct obj* cur = &anchor;

   char buf[1024];
   if ((filePTR = fopen("block2.txt", "r+"))==NULL) {
      perror("block2.txt");
      exit(1);
   }

   long totalByte = 0;
   while (fgets(buf, 1024, filePTR)!=NULL) {
      int slashN = 0;

      char* charPTR;
      if ((charPTR = memchr(buf, '\n', strlen(buf))) != NULL) {
         slashN = 1;
         *charPTR = '\0';
      }
      if (strlen(buf)==0) continue;
      totalByte+= (long) strlen(buf);
      if (slashN==0) fprintf(filePTR, "\n");
      totalByte++;

      struct obj* objPTR = malloc(sizeof (struct obj));
      if (sscanf(buf, "%d %s %s %d", &(objPTR->sequence), objPTR->sender, objPTR->receiver, &(objPTR->amount))!=4) {
         fprintf(stderr, "fail reading row.\n");
         exit(1);
      }
      curMax = MAX(curMax, objPTR->sequence);
      objPTR->head = 0;
      cur->next = objPTR;
      objPTR->prev = cur;
      cur = objPTR;
   }
   fseek(filePTR, totalByte, SEEK_SET);
   cur->next = &anchor;
   anchor.prev = cur;
}