all: tooManyCooks

tooManyCooks: tooManyCooks.o linkedList.o
	gcc -g -pthread tooManyCooks.o linkedList.o -o tooManyCooks

tooManyCooks.o: tooManyCooks.c linkedList.h
	gcc -g -c tooManyCooks.c

linkedList.o: linkedList.c linkedList.h
	gcc -g -c linkedList.c

clean:
	rm -f *.o tooManyCooks
