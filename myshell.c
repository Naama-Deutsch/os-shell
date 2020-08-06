#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int prepare(void)
{
	return 0;
}

int finalize(void)
{
	return 0;
}

int process_arglist(int count, char** arglist)
{	
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	sigaction(SIGINT,&act, NULL);
	int index=0;
	int chekPipe=0;
	int status;
	for(int i=0;i<count; i++)
	{ 
		if(!strcmp(arglist[i],"|"))
		{  
			index=i;
			arglist[index]=NULL;
     			chekPipe=1;  
   		 }  
   	 } 
	if(chekPipe)
	{
		int to_pipe[2],son1_pid,son2_pid;
		if(pipe(to_pipe)<0)
		{
			perror("Failed to pipe\n");  
              		exit(1);
		}
		son1_pid =fork();
		if(son1_pid==-1)
		{
 			perror("Fork Failed\n");  
              		exit(1);
		}

		if(son1_pid==0)
		{
			act.sa_handler =SIG_DFL;
			sigaction(SIGINT, &act,NULL);
			close(to_pipe[0]);
			dup2(to_pipe[1], 1);
			close(to_pipe[1]);
			if (execvp( arglist[0], arglist) < 0) 
			{ 
          			 printf("\nCould not execute command 1"); 
           			 exit(1); 
			}
			
		}
		else
		{

			son2_pid=fork();
			switch (son2_pid)
			{
				case -1:
					perror("Fork Failed\n");  
              				exit(1);
					break;
				case 0:
					act.sa_handler = SIG_DFL;
					sigaction(SIGINT, &act,NULL);
					close(to_pipe[1]);
					dup2(to_pipe[0], 0);
					close(to_pipe[0]);
					if (execvp(arglist[index+1], arglist+index+1) < 0)
					{ 
          			 		printf("\nCould not execute command 1");
           					exit(1); 
					}
					break;
				default:
					close(to_pipe[0]);
					close(to_pipe[1]);
					waitpid(son2_pid,&status,0);
					return 1;

			}

		}

	}
	
	if (!strcmp(arglist[count-1],"&"))
	{  
		arglist[count-1]=NULL;
		int son_pid=fork();
		switch (son_pid)
		{
			case -1:
				perror("failed to fork");
				exit(1);
				break;
			case 0:
				if(execvp(arglist[0], arglist)<0)
				{ 
          			 	printf("\nCould not execute command 1");
           				exit(1); 
				}
				break;
			default:
				return 1;
		}

	}

	else 
	{
		int son_pid=fork();
		switch (son_pid)
		{
			case -1:
				perror("failed to fork");
				exit(1);
				break;
			case 0:
				act.sa_handler = SIG_DFL;
				sigaction(SIGINT, &act,NULL);
				if(execvp(arglist[0], arglist)<0)
				{ 
          			 	printf("\nCould not execute command 1");
           				exit(1); 
				}
				break;
			default:
				waitpid(son_pid,&status,0);
				return 1;
		}


	}
	return 1;
}

