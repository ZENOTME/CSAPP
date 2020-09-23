#define DEBUG
#include <stdio.h>
#include "csapp.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define PTHREAD_NUM 4
#define SBUF_SIZE  16

/*******s_buf********/
typedef struct {
	int n;
	int *buf;
	int start;
	int end;
	sem_t s_mux;
	sem_t s_empty;
	sem_t s_useful;
}sbuf_t;

void sbuf_init(sbuf_t*sbuf,int n)
{
	sbuf->n=n;
	sbuf->buf=(int *)Calloc(n,sizeof(int));
	sbuf->start=0;
	sbuf->end=0;
	Sem_init(&sbuf->s_mux,0,1);
	Sem_init(&sbuf->s_empty,0,n);
	Sem_init(&sbuf->s_useful,0,0);
}

void sbuf_free(sbuf_t*sbuf)
{
	Free(sbuf->buf);
}

void sbuf_insert(sbuf_t*sbuf,int connfd)
{
	P(&sbuf->s_empty);
	P(&sbuf->s_mux);
	sbuf->buf[(++sbuf->end)%(sbuf->n)]=connfd;
	V(&sbuf->s_mux);
	V(&sbuf->s_useful);
}

int sbuf_delete(sbuf_t *sbuf)
{
	int fd=0;
	P(&sbuf->s_useful);
	P(&sbuf->s_mux);
	fd=sbuf->buf[(++sbuf->start)%(sbuf->n)];
	V(&sbuf->s_mux);
	V(&sbuf->s_empty);
	return fd;
}

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr="Connection: close\r\n";
static const char *prox_hdr="Proxy-Connection: close\r\n";
/* Function*/
void *thread(void *vargp);
/***********/
static void doit(int connfd);
void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg);
//connfd -> hostname,uri
static void one_geturl(int connfd,char*,char*,int*,char*,char*);
static void url_parse(char* url,char *host,int*port,char* uri);
//hostname,uri->httpmsg
static int two_getmsg(char*,char*,int,char *,char *);
//httpmsg->writeback
static int three_writeback(int connfd,char* msg);
/***********/
sbuf_t sbuf;
cache_t cache;

int main(int argc,char **argv)
{
	int listenfd,connfd;
	struct sockaddr_storage clientaddr;
	socklen_t clientlen;
	pthread_t tid;

	char client_hostname[MAXLINE],client_port[MAXLINE];

	if(argc!=2)
	{
		fprintf(stderr,"usage: %s <port>\n",argv[0]);
		exit(0);
	}

	listenfd=Open_listenfd(argv[1]);
	sbuf_init(&sbuf,SBUF_SIZE);
	cache_init(&cache);

	for(int i=0;i<PTHREAD_NUM;i++)
	{
		Pthread_create(&tid,NULL,thread,NULL);
	}

	while(1)
	{
		clientlen=sizeof(clientaddr);
		connfd=Accept(listenfd,(SA*)&clientaddr,&clientlen);
		Getnameinfo((SA *)&clientaddr,clientlen,client_hostname,MAXLINE,client_port,MAXLINE,0);
		printf("Accept connection from (%s %s)\n",client_hostname,client_port);
		/*Main thing*/
		sbuf_insert(&sbuf,connfd);
		/************/
	}
	return 0; 
}
//============================================================
void* thread(void *vargp)
{
	int connfd=0;
	Pthread_detach(pthread_self());
	while(1)
	{
		connfd=sbuf_delete(&sbuf);
		doit(connfd);
		Close(connfd);

	}
}
//============================================================
static void doit(int connfd)
{
	char method[10],host[100],uri[1024],url[1024];
	int port;
	char msg[MAXLINE];
	char* t_buf=NULL;
	one_geturl(connfd,method,host,&port,uri,url);
	
	t_buf=cache_read(&cache,url);
#ifdef DEBUG
printf("url:%s\n",url);
#endif
	if(t_buf==NULL)
	{
		if(!two_getmsg(method,host,port,uri,msg))
		{
			clienterror(connfd,"URL","xxx","can't access","Proxy can't access the url");
			return;
		}
		else
		{
			if(!three_writeback(connfd,msg))
			{
				clienterror(connfd,"URL","xxx","can't send","Proxy can't writeback the message");
				return;			
			}
	//		cache_write(&cache,msg,url);
		}
	}
	else
	{
		strcpy(msg,t_buf);
		if(!three_writeback(connfd,msg))
		{
			clienterror(connfd,"URL","xxx","can't send","Proxy can't writeback the message");
			return;			
		}
	}
}
	

//=======================ONE=================================:
static void url_parse(char* url,char *host,int*port,char* uri)
{
	*port=80;
	char *ptr1=NULL;
	char *ptr2=NULL;

	ptr1=strstr(url,"//");
	ptr1=ptr1==NULL?url:ptr1+2;

	if((ptr2=strstr(ptr1,":")))
	{
		*ptr2='\0';
		sscanf(ptr1,"%s",host);
		sscanf(ptr2+1,"%d%s",port,uri);
		if(!strcmp(uri,""))
		  strcpy(uri,"/");
	}
	else
	{
		if((ptr2=strstr(ptr1,"/")))
		{
			strcpy(uri,ptr2);
			strncpy(host,ptr1,ptr2-ptr1);
		}
		else
		{
			strcpy(host,ptr1);
			strcpy(uri,"/");
		}
	}
}


static void one_geturl(int connfd,char *method,char *host,int *port,char *uri,char *url)
{
	rio_t rio;
	char t_buf[MAXLINE],t_method[10],t_url[1024],t_version[20];
	Rio_readinitb(&rio,connfd);
	Rio_readlineb(&rio,t_buf,MAXLINE);
	printf("Request headers:%s",t_buf);
	sscanf(t_buf,"%s %s %s",t_method,t_url,t_version);
	strcpy(method,t_method);
	strcpy(url,t_url);
	url_parse(t_url,host,port,uri);
#ifdef DEBUG
printf("%d %s %s\n",*port,host,uri);
#endif  
} 

//========================TWO======================================
static int two_getmsg(char *method,char *host,int port,char *uri,char *msg)
{
	char s_port[20];
	int clientfd;
	rio_t rio;
	char buf[MAXLINE];

	sprintf(s_port,"%d",port);
	if((clientfd=Open_clientfd(host,s_port))==-1)
	{
		return 0;
	}
	printf("Connect Servive\n\n");
	Rio_readinitb(&rio,clientfd);
	
/********Send request*********/
	strcpy(buf,method);
	strcat(buf," ");
	strcat(buf,uri);
	strcat(buf," HTTP/1.0\r\n");
	Rio_writen(clientfd,buf,strlen(buf));

	strcpy(buf,"Host:");
	strcat(buf,host);
	strcat(buf,"\r\n");
	Rio_writen(clientfd,buf,strlen(buf));

	strcpy(buf,user_agent_hdr);
	Rio_writen(clientfd,buf,strlen(buf));
	
	strcpy(buf,conn_hdr);
	Rio_writen(clientfd,buf,strlen(buf));

	strcpy(buf,prox_hdr);
	Rio_writen(clientfd,buf,strlen(buf));

	Rio_writen(clientfd,"\r\n",3);
/*******************************/
	Rio_readnb(&rio,buf,MAXLINE);
	strcpy(msg,buf);

	return 1;
}

//========================THREE=====================================
static int three_writeback(int connfd,char* msg)
{
	if(Rio_writen(connfd,msg,strlen(msg))==-1)
		return 0;
	return 1;
}


//=================================================================
void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg)
{
	char buf[MAXLINE],body[MAXBUF];

	/*Build the HTTP responese body*/
	sprintf(body,"<html><title>Proxy Error</title>");
	sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
	sprintf(body,"%s%s:%s\r\n",body,errnum,shortmsg);
	sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
	sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body);

	/*Print the HTTP response*/
	sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,shortmsg);
	Rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Conten-type: text/html\r\n");
	Rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Conten-length:%d\r\n\r\n",(int)strlen(body));
	Rio_writen(fd,buf,strlen(buf));
	Rio_writen(fd,body,strlen(body));
}
