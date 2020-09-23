#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main(int argv,char** args)
{
	int size=atoi(args[2]);
	printf("%s %d\n",args[1],size);
	int fd=open(args[1],O_RDONLY,0);
	if(fd==-1)
	{
	printf("Error open\n");
	return;
	}
	char* string=(char *)mmap(NULL,size,PROT_READ,MAP_PRIVATE,fd,0);
	if(string==(char *)-1)
	{
	printf("Error start\n");
	return;
	}
	printf("%s\n",string);
	if(munmap(string,size)==0)
	{
	printf("Successful\n");
	}
	else
	{
	printf("Fail\n");
	}
	return;

}
