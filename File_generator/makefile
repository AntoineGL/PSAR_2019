all: generator

file_generator.o: file_generator.c file_generator.h
	gcc -Wall -c -o $@ $<

generator: file_generator.o generator.c
	gcc -Wall -o $@ $^

clean:
	rm -f *.o generator *~
