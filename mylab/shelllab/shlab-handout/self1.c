#include<stdio.h>
#include"csapp.h"
void handler2(int sig)
{
	printf("2");
	return;
}
int main()
{
	signal(SIGTTOU,handler2);
	printf("strat\n");	
    while(1);
}
