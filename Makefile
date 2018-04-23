
output: main.o
	gcc main.o -o output -pthread

main.o: main.c
	gcc -c main.c

clean:
	rm *.o output textout.txt
