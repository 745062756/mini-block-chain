#include "Yida_HashMap.h"

#include <stdlib.h>
#include <string.h>


// hash func copy from source: https://en.wikipedia.org/wiki/Jenkins_hash_function
static
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
   size_t i = 0;
   uint32_t hash = 0;
   while (i != length) {
      hash += key[i++];
      hash += hash << 10;
      hash ^= hash >> 6;
   }
   hash += hash << 3;
   hash ^= hash >> 11;
   hash += hash << 15;
   return hash;
}

void init(element** bucketArrPTR, unsigned int size) {
   *bucketArrPTR = malloc(sizeof (element)*size);
   for (unsigned int i = 0; i<size; i++) {
      (*bucketArrPTR)[i].anchor = True;
      (*bucketArrPTR)[i].next = &(*bucketArrPTR)[i];
      (*bucketArrPTR)[i].prev = &(*bucketArrPTR)[i];
   }
}

void destructor(element* bucketArrPTR, unsigned int size) {
   for (unsigned int i = 0; i<size; i++) {
      element* cur = bucketArrPTR[i].next;
      while (cur->anchor==False) {
         element* next = cur->next;
         free(cur->tag);
         free(cur);
         cur = next;
      }
   }
   free(bucketArrPTR);
}

void* lookup(const char* key, unsigned int size, element* bucketArrPTR) {
   element* cur;
   cur = &bucketArrPTR[(unsigned int) jenkins_one_at_a_time_hash((const uint8_t*) key, strlen(key)) % size];
   for (cur=cur->next; cur->anchor==False; cur=cur->next) {
      if (strcmp(cur->tag, key)==0) return cur->data;
   }
   return NULL;
}

int add(const char* key, void* dataPTR, unsigned int size, element* bucketArrPTR) {
   element* cur;
   cur = &bucketArrPTR[(unsigned int) jenkins_one_at_a_time_hash((const uint8_t*) key, strlen(key)) % size];
   for (cur=cur->next; cur->anchor==False; cur=cur->next) {
      if (strcmp(cur->tag, key)==0) return False;
   }
   element* newObj = malloc(sizeof (element));
   newObj->anchor = False;
   newObj->tag = malloc(strlen(key)+1);
   strcpy(newObj->tag, key);
   newObj->data = dataPTR;
   newObj->next = cur;
   newObj->prev = cur->prev;
   cur->prev->next = newObj;
   cur->prev = newObj;
   return True;
}