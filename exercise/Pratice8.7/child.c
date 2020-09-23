#include"csapp.h"
void sigint_handler(int sig)
{
	//printf("1");
	return;
}
void handler2(int sig)
{
	printf("2");
	return;
}
int main()
{
	signal(SIGTTIN, sigint_handler);
	signal(SIGTTOU, handler2);
//	int spid = getsid(STDIN_FILENO);
    //tcsetpgrp(0,getpgrp()); 
    char s[20];
    //char q[20]="quit";

  //  while(1)
  // {
//	 write(1,"s",1);
  // }
	while(read(0,s,1)!=0)
		write(1,s,1);
	//tcsetpgrp(0,spid);
	return 0;

}
