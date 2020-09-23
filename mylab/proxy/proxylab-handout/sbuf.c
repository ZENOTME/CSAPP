#include "sbuf.h"

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
