#include"csapp.h"
#include<stdio.h>
#include<signal.h>
int pid=0;
void sigchld_handler(int sig)
{
	if((pid=waitpid(-1,NULL,0))<0)
	{
		printf("error");
	}
	pid=0;
	return;
}

int main(int argc,char **argv)
{
	/*if(signal(SIGCHLD,sigchld_handler))
	{
		printf("signal error");
	}*/
	if((pid=fork())==0)
	{
		printf("child:\n");
		setpgid(0,0);
		execve("./child",argv,environ);
		exit(0);
	}
	// tcsetpgrp(0,pid);
	 if((pid=waitpid(pid,NULL,0))<0)
	 {
		 printf("error");
	 }
	//printf("pid:%d",pid);
	//while(pid);

	printf("end\n");
	return 0;
}
