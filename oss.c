//Rebecca Hanessian
//CS4760
//Project 2: Adding functionality to our system
// oss file

#include "worker.h"

int shmid;
struct sharedMem *shm = NULL;
int printNano = 500000000;
int printSec = 0;
int lastLaunchNano = 0;

typedef struct {
    int proc;
    int simul;
    float time;
    float inter;
} options_t;

void printProcessTable(struct sharedMem *shm) {
    printf("OSS PID:%d  SysClock Seconds: %d SysClock Nanoseconds: %d\n", getpid(), shm->seconds, shm->nanoseconds);
    printf("Process Table:\n");
    printf("%-6s %-9s %-10s %-7s %-15s %-12s %-12s %-10s\n", "Entry:", "Launch #:", "Occupied:", "PID:", "Start Seconds:", "Start Nano:", "End Seconds:", "End Nano:");
    
    for (int i = 0; i < MAXPROC; i++) {
        printf("%-6d %-9d %-10d %-7d %-15d %-12d %-12d %-10d\n", 
            i, 
            shm->table[i].launched,
            shm->table[i].occupied, 
            shm->table[i].pid, 
            shm->table[i].startS, 
            shm->table[i].startN,
            shm->table[i].termS,   
            shm->table[i].termN);
    }
    printf("---------------------------------------------------------------------------------\n");
}

void cleanTerm (int signal) {
	if (signal == SIGALRM) {
		fprintf(stderr, "\n60 seconds passed. Terminating.\n");
	} else if (signal == SIGINT) {
		fprintf(stderr, "\nCtrl-C entered. Terminating.\n");
	}
	
	for (int i = 0; i < MAXPROC; i++) {
		if (shm->table[i].occupied == 1 && shm->table[i].pid > 0) {
			kill(shm->table[i].pid, SIGTERM);
		}
	}
	
	while(wait(NULL) > 0);
	
	shmdt(shm);
	shmctl(shmid, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}

int getEmpty() {
	for (int i = 0; i < MAXPROC; i++) {
		if (shm->table[i].pid == 0 || shm->table[i].occupied == 0) {
			return i;
		}
	}
	exit(EXIT_FAILURE);
}

void print_usage (const char* argmt){
	fprintf(stderr, "Usage: %s [-h] [-n proc] [-s simul] [-t timelimitForChildren] [-i intervalInSeconds]\n", argmt);
	fprintf(stderr, "	proc is the number of user processes to launch\n");
	fprintf(stderr, "	simul is the number of processes that can run simultaneously\n");
	fprintf(stderr, "	timelimitForChildren is simulated time that should pass before child process terminates.\n");
	fprintf(stderr, "	intervalInSeconds is the minimum interval between launching child processes.\n");
	fprintf(stderr, "Default proc is 10, default simul is 3, default timelimitForChildren is 3.5, default intervalInSeconds is 0.2.\n");
}

int main (int argc, char *argv[]){
	signal(SIGINT, cleanTerm);
	signal(SIGALRM, cleanTerm);

	pid_t pid;
    char opt;
    options_t options;

    options.proc = 10;
    options.simul = 3;
    options.time = 3.5;
    options.inter = 0.2;
	
	opterr = 0;

	while ((opt = getopt (argc, argv, "hn:s:t:i:")) != -1)
		switch (opt) {
            case 'h':
                print_usage (argv[0]);
                return (EXIT_SUCCESS);
			case 'n':
				options.proc = atoi(optarg);
				break;
			case 's':
				options.simul = atoi(optarg);
				break;
			case 't':
				options.time = atof(optarg);
				break;
			case 'i':
				options.inter = atof(optarg);
				break;
			default:
				printf ("Invalid option %c\n", opt);
				print_usage (argv[0]);
				return (EXIT_FAILURE);		
		}
	
	int timeSec = (int)options.time;
	int timeNano = (int)((options.time - (float)timeSec) * 1e9);
	
	int interNano = options.inter * 1e9;

	int lastLaunchNano = -interNano;

	alarm(60);
	
	pid_t osspid = getpid();
	
	printf("OSS starting, PID: %d\n", osspid);
	printf("Called with:\n-n %d\n-s %d\n-t %f\n-i %f\n", options.proc, options.simul, options.time, options.inter);
	
	key_t ossKey = ftok("oss.c", 'c');
	shmid = shmget(ossKey, sizeof(shm), 0644 | IPC_CREAT);
	shm = shmat(shmid, 0, 0);
	
	int activeWorkers = 0;
	int totalWorkers = 0;
	
	int nanosecInc = 250000;
	
	shm->seconds = 0;
	shm->nanoseconds = 0;
	
	while (totalWorkers < options.proc || activeWorkers > 0) {
		// Increment system clock
		shm->nanoseconds += nanosecInc;
		if (shm->nanoseconds >= 1000000000) {
			shm->seconds++;
			shm->nanoseconds = 0;
		}
		
		// Print process table
		if (shm->seconds > printSec || (shm->seconds == printSec && shm->nanoseconds >= printNano)) {
			printProcessTable(shm);
			
			printNano += 500000000;
			if (printNano >= 1000000000) {
				printSec++;
				printNano -= 1000000000;
			}
		}
		
		// Check if worker terminated
		int status;
		pid_t termWorker = waitpid(-1, &status, WNOHANG);
		
		// If terminated, update pcb
		if (termWorker > 0) {
			for (int i = 0; i < options.proc; i++) {
        		if (shm->table[i].pid == termWorker && shm->table[i].occupied == 1) {
           			shm->table[i].termS = shm->seconds;
           			shm->table[i].termN = shm->nanoseconds;
           			shm->table[i].occupied = 0;
           			activeWorkers--;
            		break;
        		}
    		}
		}
		
		// Launch new worker
		if (totalWorkers < options.proc && activeWorkers < options.simul) {
			int emptySlot = getEmpty();
			int sysInNano = (shm->seconds * 1e9) + shm->nanoseconds;
			if (sysInNano >= lastLaunchNano + interNano) {
				totalWorkers++;
				pid = fork();
				if (pid == 0) {
					char sec[20];
					char nano[20];
					snprintf(sec, sizeof(sec), "%d", timeSec);
					snprintf(nano, sizeof(nano), "%d", timeNano);
           			char *newargv[] = {"./worker", sec, nano, NULL};
            		execvp(newargv[0],newargv);
            		perror("Execvp error\n");
            		exit(EXIT_FAILURE);
				} else if (pid > 0) {
					shm->table[emptySlot].launched = totalWorkers;
					shm->table[emptySlot].occupied = 1;
					shm->table[emptySlot].pid = pid;
    				shm->table[emptySlot].startS = shm->seconds;
    				shm->table[emptySlot].startN = shm->nanoseconds;
    				shm->table[emptySlot].termS = 0;   
    				shm->table[emptySlot].termN = 0;
    				sysInNano = (shm->seconds * 1e9) + shm->nanoseconds;
    				lastLaunchNano = sysInNano;
					activeWorkers++;
				} else {
					perror("Fork error\n");
					exit(EXIT_FAILURE);
				}
			}  
		}
	}
	
	printProcessTable(shm);
	
	fprintf(stderr, "OSS PID: %d Terminating\n", osspid);
	fprintf(stderr, "%d workers were launched and terminated\n", totalWorkers);
	fprintf(stderr, "Workers ran for a combined time of %d seconds %d nanoseconds\n", shm->seconds, shm->nanoseconds);

	shmdt(shm);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}		
		