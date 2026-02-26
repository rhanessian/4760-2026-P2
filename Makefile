CC = gcc

all: oss worker

oss: oss.c
	gcc -std=c99 -o oss oss.c

worker: worker.c
	gcc -std=c99 -o worker worker.c

clean:
	$(RM) worker oss