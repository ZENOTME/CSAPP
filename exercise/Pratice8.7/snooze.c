#include"csapp.h"
unsigned int wakeup(unsigned int secs);
unsigned int time1=0;
void sigint_handler(int sig)
{
	return;
}
unsigned int wakeup(unsigned int secs)
{
	return secs-sleep(secs);
}
int main(int argc,char*argv[],char *envp[])
{
	if(signal(SIGINT,sigint_handler)==SIG_ERR)
	  printf("signal_errpr");
	int time1=wakeup(atoi(argv[1]));
	printf("Woke up at %d",time1);
	return 0;
}

