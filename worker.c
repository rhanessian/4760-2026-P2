//Rebecca Hanessian
//CS4760
//Project 2: Adding functionality to our system
// worker file

#include "worker.h"

int main (int argc, char *argv[]){
	if (argc < 3) {
		fprintf(stderr, "Worker missing arguments.\n");
		exit(EXIT_FAILURE);
	}

	int minSeconds = atoi(argv[1]);
	int minNano = atoi(argv[2]);
	
	pid_t pid = getpid();
	pid_t ppid = getppid();
	
	fprintf(stderr, "\nWorker starting, PID: %d  PPID: %d\n", pid, ppid);
	fprintf(stderr, "Called with:\n\tProcess Time: %d seconds, %d nanoseconds\n\n", minSeconds, minNano);
	
	key_t ossKey = ftok("oss.c", 'c');
	int shmid = shmget(ossKey, 0, 0);
	if (shmid == -1) {
		perror("Worker shmget failed.\n");
		exit(EXIT_FAILURE);
	}
	
	struct sharedMem *shm = (struct sharedMem *)shmat(shmid, NULL, 0);
	if (shm == (void *)-1) {
		perror("Worker shmat failed.\n");
		exit(EXIT_FAILURE);
	}
	
	int startSeconds = shm->seconds;
	int startNano = shm->nanoseconds;
	
	int termSeconds = startSeconds + minSeconds;;
	int termNano = startNano + minNano;
	
	if (termNano >= 1000000000) {
		termSeconds += (termNano / 1000000000);
		termNano %= 1000000000;
	}
	
	fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
	fprintf(stderr, "SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n", startSeconds, startNano, termSeconds, termNano);
	fprintf(stderr, "--Just Starting\n\n");
	
	int active = 1;
	while (active) {
		int sysSeconds = shm->seconds;
		int sysNano = shm->nanoseconds;
		if (sysSeconds > termSeconds || (sysSeconds == termSeconds && sysNano >= termNano)) {
			active = 0;
		} else {
			int timePassed = sysSeconds - startSeconds;
			fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
			fprintf(stderr, "System Clock: %ds %dns | Term: %ds %dns\n", sysSeconds, sysNano, termSeconds, termNano);
			fprintf(stderr, "--%d seconds have passed since starting\n\n", timePassed);		
		}
	}

	fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
	fprintf(stderr, "System Clock: %ds %dns | Term: %ds %dns\n", shm->seconds, shm->nanoseconds, termSeconds, termNano);
	fprintf(stderr, "--Terminating\n\n");	
	
	shmdt(shm);
	return 0;
}






