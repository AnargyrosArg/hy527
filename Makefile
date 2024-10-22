examples: sieve sieve2 sieve4 sink sort spin spin2 spin3


sieve: ./examples/sieve.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/sieve.c -o sieve.o -I.
	gcc -m32 sieve.o thread.o switch.o chan.o  -o sieve -I.

sieve2: ./examples/sieve2.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/sieve2.c -o sieve2.o -I.
	gcc -m32 sieve2.o thread.o switch.o chan.o  -o sieve2 -I.
sieve4: ./examples/sieve4.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/sieve4.c -o sieve4.o -I.
	gcc -m32 sieve4.o thread.o switch.o chan.o  -o sieve4 -I.
sink: ./examples/sink.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/sink.c -o sink.o -I.
	gcc -m32 sink.o thread.o switch.o chan.o  -o sink -I.
sort: ./examples/sort.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/sort.c -o sort.o -I.
	gcc -m32 sort.o thread.o switch.o chan.o  -o sort -I.
spin: ./examples/spin.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/spin.c -o spin.o -I.
	gcc -m32 spin.o thread.o switch.o chan.o  -o spin -I.
spin2: ./examples/spin2.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/spin2.c -o spin2.o -I.
	gcc -m32 spin2.o thread.o switch.o chan.o  -o spin2 -I.
spin3: ./examples/spin3.c thread.o thread.h queue.h switch.o chan.o
	gcc -m32 -c -g ./examples/spin3.c -o spin3.o -I.
	gcc -m32 spin3.o thread.o switch.o chan.o  -o spin3 -I.


chan.o: chan.c chan.h
	gcc -m32 -c -g chan.c -o chan.o -I.

thread.o: thread.c thread.h
	gcc -m32 -c -g thread.c -o thread.o  -I.

switch.o: switch.s
	gcc -m32 -c -g switch.s

clean:
	rm -rf *.o sieve sieve2 sieve4 sink sort spin spin2 spin3