CFLAG = -g -Wall -o

all: serverM serverA serverB serverC clientA clientB

serverM: serverM.o Yida_HashMap.o
	gcc $(CFLAG) serverM serverM.o Yida_HashMap.o

Yida_HashMap.o: Yida_HashMap.c Yida_HashMap.h
	gcc -g -Wall -c Yida_HashMap.c

serverM.o: serverM.c serverM.h
	gcc -g -Wall -c serverM.c

serverA: serverA.c serverM.h
	gcc $(CFLAG) serverA serverA.c

serverB: serverB.c serverM.h
	gcc $(CFLAG) serverB serverB.c

serverC: serverC.c serverM.h
	gcc $(CFLAG) serverC serverC.c

clientA: clientA.c clientA.h
	gcc $(CFLAG) clientA clientA.c

clientB: clientB.c clientA.h
	gcc $(CFLAG) clientB clientB.c