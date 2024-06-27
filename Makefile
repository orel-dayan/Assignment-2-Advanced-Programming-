.PHONY: all
all: generator primeCounter newCounter 


newCounter: newCounter.c 
	gcc -g -o newCounter newCounter.c -pthread

generator:  generator.c
	gcc -o randomGenerator generator.c

primeCounter:	primeCounter.c
	gcc -o primeCounter primeCounter.c

.PHONY: clean
clean:
	-rm randomGenerator primeCounter 2>/dev/null  newCounter
