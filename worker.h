#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAXPROC 20
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

struct PCB{
	int launched;
    int occupied;        
    pid_t pid;            
    int startS;
    int startN;
    int termS;
    int termN;
};

struct sharedMem{
	int seconds;
	int nanoseconds;
	struct PCB table[MAXPROC];
};