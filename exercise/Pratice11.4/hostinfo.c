#include<csapp.h>

int main(int argc,char**argv)
{
	struct addrinfo *p,*listp,hints;
	char buf[MAXLINE];
	char serverbuf[MAXLINE];
	int rc,flags;

	if(argc<2)
	{
		fprintf(stderr,"usage:%s<dimain name>\n",argv[0]);
		exit(0);
	}

	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	if((rc=getaddrinfo(argv[1],"http",&hints,&listp))!=0)
	{
		fprintf(stderr,"getaddrinfo error:%s\n",gai_strerror(rc));
		exit(1);
	}

	flags=NI_NUMERICHOST;
	for(p=listp;p;p=p->ai_next)
	{
		getnameinfo(p->ai_addr,p->ai_addrlen,buf,MAXLINE,serverbuf,MAXLINE,flags);
		printf("%s  %s\n",buf,serverbuf);
	}

	freeaddrinfo(listp);

	exit(0);
}
