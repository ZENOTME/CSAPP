#include "csapp.h"

void handler2(int sig)
{
	int olderrno=errno;
	while(waitpid(-1,NULL,0)>0){
		printf("Handler reaped child\n");
	}
	if(errno!=ECHILD)
		printf("waitpid error");
	sleep(1);
	errno=olderrno;
}

int main()
{
	int n;
	char buf[MAXBUF];
	if(signal(SIGCHLD,handler2)==SIG_ERR)
		printf("signal error");
	if(fork()==0)
	{
		printf("Hello fromm pid%d\n",(int)getpid());
		exit(0);
	}
	if(fork()==0)
	{
		printf("Hello fromm pid%d\n",(int)getpid());
		exit(0);
	}
	if(fork()==0)
	{
		printf("Hello fromm pid%d\n",(int)getpid());
		while(1);
	}
	if((n=read(STDIN_FILENO,buf,sizeof(buf)))<0)
	  printf("read error");

	printf("input");
	while(1);
	exit(0);
}
