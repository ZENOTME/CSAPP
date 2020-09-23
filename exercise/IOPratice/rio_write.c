#include<csapp.h>

int main()
{
	char buf[MAXLINE];
	while(rio_readn(0,buf,2)!=0)
	  rio_writen(1,buf,2);



}
