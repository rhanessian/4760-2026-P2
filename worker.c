//Rebecca Hanessian
//CS4760
//Project 2: Adding functionality to our system
// worker file

#include "worker.h"

int shmid;
struct sharedMem *shm;

int main (int argc, const char *argv[]){
	int minSeconds = atoi(argv[1]);
	int minNano = atoi(argv[2]);
	
	int startSeconds;
	int startNano;
	
	int sysSeconds = 0;
	int sysNano = 0;
	float sysTime = 0;
	
	int termSeconds;
	int termNano;
	float termTime;
	
	int timePassed;
	
	pid_t pid = getpid();
	pid_t ppid = getppid();
	
	fprintf(stderr, "\nWorker starting, PID: %d  PPID: %d\n", pid, ppid);
	fprintf(stderr, "Called with:\n\tProcess Time: %d seconds, %d nanoseconds\n\n", minSeconds, minNano);
	
	key_t ossKey = ftok("oss.c", 'c');
	shmid = shmget(ossKey, sizeof(struct sharedMem), 0);
	shm = shmat(shmid, 0, 0);
	
	startSeconds = shm->seconds;
	startNano = shm->nanoseconds;
	
	termSeconds = startSeconds + minSeconds;
	termNano = startNano + minNano;
	termTime = (float)termSeconds + ((float)termNano / (1e9));
	
	fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
	fprintf(stderr, "SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n", sysSeconds, sysNano, termSeconds, termNano);
	fprintf(stderr, "--Just Starting\n\n");

	while (sysTime < termTime ) {
		sysSeconds = shm->seconds;
		sysNano = shm->nanoseconds;
		sysTime = (float)sysSeconds + ((float)sysNano / (1e9));
		timePassed = sysSeconds - startSeconds;
		fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
		fprintf(stderr, "SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n", sysSeconds, sysNano, termSeconds, termNano);
		fprintf(stderr, "--%d seconds have passed since starting\n\n", timePassed);		
	}

	fprintf(stderr, "WORKER PID: %d PPID: %d\n", pid, ppid);
	fprintf(stderr, "SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n", sysSeconds, sysNano, termSeconds, termNano);
	fprintf(stderr, "--Terminating\n\n");	
	
	shmdt(shm);
	return 0;
}






