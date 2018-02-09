/*-------------------------------------------------------
File: persons.c

Name: Muhammad Owais Bawany
Student Number: 7993647
Assignment 1 -- Operating Systems
 
Description: This program is designed to test the talk
             agent program using pipes.  It simulates
	     two persons by spawning a child to act
	     as person B, while the parent acts as Person
	     A.  To enable the dialog, two talkagent
	     processes are spawned and connected using
	     two pipes.  An additional 2 pipes are used
	     to connect the Person A process to one 
	     talkagent standard input and the Person B
	     process to the other talk agent standard
	     input.
--------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

char *persAmessages[] = /* Person A messages */
{
   "Hello Person B.",
   "I am doing great - want to meet for lunch?",
   "How about tomorrow?",
   "Ok - let's make it on Friday.",
   "Good luck with your assignment. Bye for now.",
   "exit",
   NULL
};

char *persBmessages[] = /* Person B messages */
{
   "How are you Person A?",
   "Good idea - what day are you free this week?",
   "Sorry - can't make it tomorrow - I have an assignment due the next day.",
   "I am free on Friday - see you then.",
   "exit",
   NULL
};
  
/* function prototypes */
int setupPerB(void);;
void initTalkBufferB(int, int [], int []);
int initTalkBufferA(int [], int []);
void setupPerA(int);

/*---------------------------------------------------------------
Function: main

Description:  The main function calls the following four functions:
                 setupPerB: spawns a child to act as Person B.
		 initTalkBufferB: spawns talkagent process for
		                  servicing Person B
		 initTalkBufferA: spawns talkagent process for
		                  servicing Person A
		 setupPerA: acts as Person A (no process is 
		            spawned).

Assignment: Complete this function to ensure proper calls to 
            the other functions.
---------------------------------------------------------------*/
int main(int argc, char *argv[])
{
   int perBrqueue;  /* read end of pipe */
   int perAwqueue;  /* write end of pipe */
   int persAtoBfds[2];  /* pipe for local to remote connection */ 
   int persBtoAfds[2];  /* pipe for remote to local connection */ 

   printf("Simulation starting\n");

   perBrqueue = setupPerB(); /* setup Person B and its pipe */

   /* Setup the two pipes common to both talkagent processes */
   pipe(persAtoBfds);
   pipe(persBtoAfds);
   
   /* Create talkagent for Person B*/
   /* note that stdin connected to persB pipe */
   /* should close perBrqueue for this process as well */
   initTalkBufferB(perBrqueue, persAtoBfds, persBtoAfds);
   /* Create talkagent for Person A*/
   /* note that stdin connected to persB pipe */
   /* should close perBrqueue for this process as well */
   perAwqueue = initTalkBufferA(persAtoBfds, persBtoAfds);
   /* close the files descriptors to pipes between talkagent processes */
   close(persAtoBfds[0]);
   close(persAtoBfds[1]);
   close(persBtoAfds[0]);
   close(persBtoAfds[1]);
   /* Lets do person A now */
   setupPerA(perAwqueue);
   fprintf(stderr,"Simulation complete\n");
}

/*---------------------------------------------------------------
Function: setupPerB

Description: This function spawns a child process to act as
             Person B.  It must create a pipe and attach the
	     write end of the pipe to the standard output of the 
             Person B process. It returns the file descriptor of 
             the read end so that it may be attached to the 
	     standard input of a talkagent process.


Assignment: Complete this function.  The code for generating
            messages has been provided.
---------------------------------------------------------------*/
int setupPerB(void)
{
	   int pipefds[2]; /* for pipe file descriptors */
   int pid;  /* for return value of fork() */
   int i;
   
   pipe(pipefds); /* create a pipe */
   pid = fork();  /* fork a process for Person B */
   if(pid == 0)  
   {  /* in the child */
      dup2(pipefds[1],1); /* setup stdout to the pipe */
      close(pipefds[0]);  /* don't need ref to read end */
      close(pipefds[1]);  /* do not need this ref to write end */
      /* now we can start the conversation */
      sleep(2);  /* wait for first message from Person A */
      for(i=0 ; persBmessages[i] != NULL ; i++)
      {
         printf("%s (%d)\n",persBmessages[i], getpid());
	 fflush(stdout);
	 sleep(2);  /* wait for response */
      }
      exit(1);  /* terminate the child process */
   }
   
   /* in the parent */
   close(pipefds[1]);  /* close the write end */
   return(pipefds[0]); /* return for connecting to talkagent */
}

/*---------------------------------------------------------------
Function: initTalkBufferB

Description: This function spawns a talkagent process for 
             Person B.  Two arguments are provided to the called 
	     talkagent program, the file descriptors for reading 
	     and writing to the talk agent servicing person A.
	     It must also attach read end of a pipe attached
	     to Person B process to its standard input.  

Assignment: Complete this function.
---------------------------------------------------------------*/
void initTalkBufferB(int persBrfd, int persAtoBfds[], int persBtoAfds[])
{
   char infd[10];
   char outfd[10];
   int pid;

   pid = fork();  /* fork process for talkagent B */
   if(pid == 0)
   { /* in the child */
      /* First setup the stdin */
      dup2(persBrfd,0);   /* reads from pipe */
      close(persBrfd);  /* no longer need this file descriptor */
      /* Setups the references to the pipes */
      sprintf(infd,"%d",persAtoBfds[0]); /* read end of pipe from Agent A */
      close(persAtoBfds[1]);  /* do not need write end of the pipe */
      sprintf(outfd,"%d",persBtoAfds[1]); /* write end of pipe to Agent A */
      close(persBtoAfds[0]);  /* do not need read end of the pipe */
      execl("talkagent","talkagent",infd,outfd,NULL);
   }
   close(persBrfd);  /* do not need this reference any more */
   /* parent returns to continue setup */
}

/*---------------------------------------------------------------
Function: initTalkBufferA

Description: This function spawns a talkagent process for 
             Person A.  Two arguments are provided to the called 
	     talkagent program, the file descriptors for reading 
	     and writing to the talk agent servicing person B.
	     It must also create a new pipe and attach the read 
	     end of the new pipe to the standard input of the
	     talk agent process. It shall return the write
	     end of the pipe to be attached to the Person A
	     process.

Assignment: Complete this function.
---------------------------------------------------------------*/
int initTalkBufferA(int persAtoBfds[], int persBtoAfds[])
{
   int pipefds[2];  /* pipe for connecting to person A */
   char infd[10];   /* string for the input file descriptor argument */
   char outfd[10];  /* string for the output file descriptor argument */
   int pid;         /* return value of fork */
   
   pipe(pipefds); /* create a pipe */
   pid = fork();  /* fork process for talkagent A */
   
   if(pid == 0)
   { /* in the child */
      /* First setup the stdin */
         dup2(pipefds[0], 0);/* read from pipe */
         close(pipefds[0]);/* close the file descriptors you no longer need */
         close(pipefds[1]);
	  
      /* Setup the references to the pipes */
             sprintf(infd,"%d",persBtoAfds[0]);/* implement read end of pipe from Person B--- Hint: sprintf */
			 close(persBtoAfds[1]);/* close the file descriptor you no longer need */
             sprintf(outfd,"%d",persAtoBfds[1]);/* implement write end of pipe from Person B--- Hint: sprintf */
             close(persAtoBfds[0]);/* close the file descriptor you no longer need */
             
			 execl("talkagent","talkagent",infd,outfd,NULL);
   }
   /* parent returns to continue setup */
   close(pipefds[0]);  /* no need for read end */
   return(pipefds[1]);  /* return write end for Person A process */
}

/*---------------------------------------------------------------
Function: setupPerA

Description: This function acts as Person A by sending the
             person A messages to the standard output.  The
	     standard output is attached to the pipe
	     attached to the talkagent standard input servicing
	     Person A. 

Assignment: Complete this function.  The code for generating
            messages has been provided.
---------------------------------------------------------------*/
void setupPerA(int persAwfd)
{
    int i;
	dup2(persAwfd, 1);
	close(persAwfd);
    /* Do the person A conversation */
    
	/* Step-1: attach to stdout */
    /* Step-2: close the file descriptor you no longer need */
    
	/* Below are the conversation steps */
    sleep(1);  /* wait before sending first message to Person B */
	
	/*** Implement a FOR loop here similar to that of personB ****/
	for(i=0 ; persAmessages[i] != NULL ; i++)
    {
       printf("%s (%d)\n",persAmessages[i], getpid());
       fflush(stdout);
       sleep(2);  /* wait for response */
    }    
}
