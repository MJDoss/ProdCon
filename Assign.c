/* 
* Name: Matthew Doss
* Title: Prod Con
* 
* The program reads an entered command as input,  
* The available commands are: showjobs, submit <arguments>
*
* submit <job>: Adds a job to the queue list to completed
* 
* showjobs: Iterates through the list of jobs submitted and prints it to the screen.
*
* To Compile: gcc -O -Wall -o <filename> Assign.c -lpthread
* To Run: ./a.out <# of jobs to run at one time>
*
* Once running you will be prompted for a command.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>

// Declarations and initializations
#define CMD_SIZE 256
#define BUFSIZE 256

int Pjobs;

typedef struct job{ // Job has to be queued this can be done through a Linked List
	int jobid;
	char *Command;
	char *Status;
	struct job * next; // one connection linked list.
}job_t;
job_t * head;

// This goes through the jobs queue to the fist one that says it's Waiting gives it's info to the process.
job_t * getWaitingJob(job_t * head){ 
	job_t * current = head;
	while(strcmp(current->Status,"Waiting") != 0){
		current = current->next;
	}
	current->Status = "Running";
	return current;
}


// This adds the submitted input into the queue.
void AddJob(job_t * head, int jid, char *cmnd, char *stat){ 
	job_t * current = head;
	while(current->next != NULL){
		current = current->next;
	}
	current->next = (job_t *) malloc(sizeof(job_t));
	current->next->jobid = jid;
	current->next->Command = cmnd;
	current->next->Status = stat;
	current->next->next = NULL;
}

// if the current head is the job we need to remove it needs an unique job removal procedure.
void pop(job_t ** head){
	job_t * nextJob = NULL;
	if(*head == NULL){
		return (NULL);
	}
	nextJob = (*head)->next;
	free(*head);
	*head = nextJob;
}

// Once the given job has been ran it needs to be removed from the front ofthe list.
// Because multiple jobs can be ran this has to by specific index determined through the jobid.
void RemoveJob(job_t ** head, int jobid){
	job_t * current = *head;
	job_t * tempJob = NULL;
	
	if((*head)->jobid == jobid){ // if the head process's jobid matches the jobid we need to remove just pop it.
		return pop(head);
	}
	while(current->next->jobid != jobid){ // Get to the job just before the one we're removing from the linked list.
		if(current->next == NULL){
			return (NULL);
		}
		current = current->next;
	}
	tempJob = current->next;
	current->next = tempJob->next;
	free(tempJob);
}

// This is a producer process that reads the linked list to see what is running and what is enqueue.
void ShowJobs(job_t * head){
	printf("JobID\t Command\t\t\t\t Status\n");
	job_t * current = head;
	while(current != NULL){
		printf("%d\t %s \t\t\t\t %s\n",current->jobid, current->Command, current->Status);
		current = current->next;
	}
}


void *producer(void * args){ // Producer will take in jobs to add to queue
	char command[CMD_SIZE];
	char *subcmd, *remcmd;
	int count = 0;
	char delim[] = " ";

	
	int loop=1;
	while(loop == 1){ // producer process will interact with user as child process handles the background submissions.
		printf("Enter command: ");
		fgets(command, CMD_SIZE, stdin);
		strtok(command, "\n");
		
		if((strcmp(command, "quit")) == 0){
			printf("Exiting program. . . bye!\n");
			loop = 0;
			exit(EXIT_SUCCESS);
		}
		
		if((strcmp(command, "showjobs")) == 0){
			ShowJobs(head);	
		}
		
		remcmd = command;
		subcmd = strsep(&remcmd, delim);
		if((strcmp(subcmd, "submit")) == 0){
			if(head == NULL){
				head = (job_t *) malloc(sizeof(job_t));
				head->jobid = count;
				head->Command = remcmd;
				head->Status = "Waiting";
				head->next = NULL;
			}
			else{
				AddJob(head, count, remcmd, "Waiting");
			}
			count++;
		}
		
		if((strcmp(command, "submit")) != 0 && (strcmp(command, "showjobs")) != 0){
			printf("Command %s not recognized.\n",command);
		}
	}
	return (NULL);
}

// Number of consumers should be the number intaked in argv[1]
void *consumer(void *args){ // Consumer takes in jobs from the queue and process them.
	int loop = 1;
	int fdout, fderr;
	char outFile[BUFSIZE], errFile[BUFSIZE];
	while(loop == 1){
		if(head != NULL){
			head->Status = "Running";

			sprintf(outFile, "%d.out", head->jobid);
			sprintf(errFile, "%d.out", head->jobid);
			
			if((fdout = open(outFile, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1){
				printf("Error opening file %s for output\n", outFile);
				exit(-1);
			}
			if((fderr = open(errFile, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1){
				printf("Error opening file %s for output\n", errFile);
				exit(-1);
			}
			
			dup2(fdout, 1);
			dup2(fderr, 2);
			
			execvp(head->Command,outFile);
			
			pop(head);
		}
	}
	return (NULL);
}


// Main function to build and set up the producer and consumer.
int main(int argc, char **argv) {
	pthread_t tid_prod, tid_con;
	head = NULL;
	
	if(argc != 2){
		printf("%s <# of jobs to run at one time>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	Pjobs = atoi(argv[1]);
	if(Pjobs > 8){
		printf("Too many threads requested. Please use a number less than 8\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_create(&tid_prod, NULL, producer, NULL);
	pthread_create(&tid_con, NULL, consumer, NULL);
	
	pthread_join(tid_prod, NULL);
	pthread_join(tid_con, NULL);
	
	return 0;
}