CC = g++ 
CFLAGS = -Wall -g

all:	main.o FileSystem.o
	$(CC) $(CFLAGS) main.o FileSystem.o -o lab6
	
main.o:	main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

FileSystem.o:	FileSystem.cpp FileSystem.h
	$(CC) $(CFLAGS) -c FileSystem.cpp -o FileSystem.o

clean:
	rm -f *.o
