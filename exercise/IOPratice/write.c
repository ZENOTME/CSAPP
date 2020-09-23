#include <csapp.h>

int main()
{
	int file=open("test.txt",O_RDWR,0);
	char buf[MAXLINE];
	//lseek(file,0,SEEK_SET);
	while(fgets(buf,MAXLINE,stdin)!=NULL)
	{
		write(file,buf,strlen(buf));
		pread(file,buf,MAXLINE,0);
		fputs(buf,stdout);
	}
	exit(0);
}
