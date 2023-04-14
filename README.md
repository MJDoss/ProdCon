Name: Matthew Doss
Title: Prod Con

This program divides process in a producer, consumer manner in which the user is the lone producer,
who makes jobs that are added to a queue. The consumer(s), the number of which are based on argv[1],
then take these jobs from the queue and process them in the background.

The available commands are: showjobs, submit <job>

submit <job>: Adds a job to the queue list to completed

showjobs: Iterates through the list of jobs submitted and prints it to the screen.

To Compile: gcc -O -Wall -o <filename> Assign.c -lpthread
To Run: ./a.out <# of jobs to run at one time>

Once running you will be prompted for a command.
