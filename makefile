main2: main.o finder.o xxhash.o
	gcc main.o finder.o xxhash.o -lcrypto -o main2

main.o: main.c finder.h
	gcc -c main.c 

finder.o: finder.c finder.h xxhash.h
	gcc -c finder.c

xxhash.o: xxhash.c
	gcc -c xxhash.c

clean:
	rm *.o
