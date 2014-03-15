int dataStart[1] = {1}; // initialized
int bssStart[1]; // unitialized

#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>
#include <malloc.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0

// # of bytes in a MB
#define MB (1024 * 1024)

char _start ,__bss_start;

/* 
* repeatedly uses alloca to get space off the stack
* logs successful allocations to file for inspection by parent process
* note that this function alwasy throws a seg fault
*/
void logStackAllocation(){
	int pid = fork();

	if (pid == 0){
		close(1);
		open("stack", O_TRUNC | O_WRONLY);

		while (TRUE){
			printf("%10p\n", alloca(MB));
			fflush(stdout);
		}
		exit(EXIT_SUCCESS);
	} else {
		waitpid(pid, &pid, 0);
	}
}

/*
counts the number of lines a given log file
*/
int countLog(char *name){
	FILE *file = fopen(name, "r");
	int lineCount = 0;
	char *line = NULL;
	size_t len = 0;

	while ( getline(&line, &len, file) != -1) lineCount++;

	return lineCount;
}

/*
forks a child and repeatedly calls malloc to get space off the heap
child logs allocations to file, parent waits for child to finish
*/
void logHeapAllocation(){
	int pid = fork();

	if (pid == 0){
		close(1);
		open("heap", O_TRUNC | O_WRONLY);

		while(TRUE){
			printf("%10p\n", malloc(MB));
			fflush(stdout);
		}
		exit(EXIT_SUCCESS);
	} else {
		waitpid(pid, &pid, 0);
	}
}
/*
forks a child and repeatedly calls mmap to get a piece of mmap memory
parent waits for child to finish logging
*/
void logMmapAllocation(){
	int pid = fork();

	if (pid == 0){
		close(1);
		open("mmap", O_TRUNC | O_WRONLY);

		while(TRUE){
			char *addr = mmap(NULL, MB, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
			if (addr == MAP_FAILED) exit(EXIT_FAILURE);
			printf("%10p\n", addr);
			fflush(stdout);
		}
		exit(EXIT_SUCCESS);
	} else {
		waitpid(pid, &pid, 0);
	}
}

int main(void){
	char *heapStart = malloc(MB);
	char *mmapStart = mmap(NULL, MB, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE,-1,0);

	logHeapAllocation();
	int heapSize = countLog("heap");
	printf("The heap is: %d mb\n", heapSize);
	printf("The heap starts at: %10p\n", heapStart);

	logStackAllocation();
	int stackSize = countLog("stack");
	printf("The stack is: %d mb\n", stackSize);
	printf("The stack starts at: %10p\n", &heapSize); // first variable in main

	logMmapAllocation();
	int mmapSize = countLog("mmap");
	printf("The mmap segments is: %d mb\n", mmapSize);
	printf("The mmap starts at: %10p\n", mmapStart);
	
	printf("The bss starts at: %10p\n", &__bss_start);
	printf("The data starts at: %10p\n", &dataStart);

	printf("The text segment begins at: %10p\n", &_start);

	return EXIT_SUCCESS;
}
