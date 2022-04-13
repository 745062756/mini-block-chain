CFLAG = -g -Wall -o

all: serverM serverA serverB serverC

serverM: serverM.c serverM.h
	gcc $(CFLAG) serverM serverM.c

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

clean:
	rm -f *.o serverM serverA serverB serverC clientA clientB
	rm -r *.dSYM

Zombie:
	ps aux | grep $PWD

kill:
	kill -s CHLD