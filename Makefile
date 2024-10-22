
a.out: main.o thread.o thread.h queue.h switch.o chan.o 
	gcc -m32 main.o thread.o switch.o chan.o  -o a.out -I.

main.o: main.c 
	gcc -m32 -c -g main.c -o main.o -I.

chan.o: chan.c chan.h
	gcc -m32 -c -g chan.c -o chan.o -I.

thread.o: thread.c thread.h
	gcc -m32 -c -g thread.c -o thread.o  -I.

switch.o: switch.s
	gcc -m32 -c -g switch.s

clean:
	rm -rf *.o a.out