#ifndef _SBUF_H_
#define _SBUF_H_
#include"csapp.h"

typedef struct {
	int n;
	int *buf;
	int start;
	int end;
	sem_t s_mux;
	sem_t s_empty;
	sem_t s_useful;
}sbuf_t;

void sbuf_init(sbuf_t*sbuf, int n);
void sbuf_free(sbuf_t* sbuf);
void sbuf_insert(sbuf_t *sbuf,int connfd);
int sbuf_delete(sbuf_t *sbuf);

#endif 
