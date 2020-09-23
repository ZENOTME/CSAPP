#include<csapp.h>

int main(int argc,char **argv)
{
	FILE *fout,*fin;
	int clientfd;
	char *host,*port,buf[MAXLINE];
	rio_t rio;

	if(argc!=3)
	{
		fprintf(stderr,"usage:%s<host><port>\n",argv[0]);
		exit(0);
	}
	host=argv[1];
	port=argv[2];

	clientfd=Open_clientfd(host,port);
	printf("Connect!\n");
	Rio_readinitb(&rio,clientfd);
	while(fgets(buf,MAXLINE,stdin)!=NULL)
	{
		if(!strcmp(buf,"\n"))
		  strcpy(buf,"\r\n");
		Rio_writen(clientfd,buf,strlen(buf));
		if(!strcmp(buf,"\r\n"))
		{
			break;
		}
	}
	Rio_readnb(&rio,buf,MAXLINE);
	Fputs(buf,stdout);
	close(clientfd);
	exit(0);
}
