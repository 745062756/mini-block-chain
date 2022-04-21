#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

typedef struct element {
    short int anchor;
    char* tag;
    struct element* next;
    struct element* prev;
    void* data;
} element;

extern void init(element**, unsigned int);
extern void destructor(element*, unsigned int);
extern void* lookup(const char*, unsigned int, element*);
extern int add(const char*, void*, unsigned int, element*);