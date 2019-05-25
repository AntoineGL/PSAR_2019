.PHONY : all clean check
.ONESHELL:

CCOPTIONS=-Wall -O0

all : benchmark

%.o : %.c
	gcc $(CCOPTIONS) -c $<

benchmark : benchmark.o logger.o
	gcc $(CCOPTIONS) $^ -o $@

clean :
	rm -rf *.o benchmark *~
