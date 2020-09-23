//#define DEBUG
//#define MEMWATCH
/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*Basic constants and macros*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y)((x)>(y)?(x):(y))
#define PACK(size,alloc) ((size)|(alloc))

#define GET_LAST(p) (!((GET(p)>>1)&0x1))
#define SET_LAST(p) (*(unsigned int*)(p)|=(0x1<<1))
#define CLEAR_LAST(p) (*(unsigned int*)(p)&=(0xfffffffd))
/*opt Lev1*/
#define GET(p) (*(unsigned int*)p)
#define GET_L(p) (*(void **)p)
#define PUT(p,val) (*(unsigned int *)(p)=(val))
#define PUT_L(p,val) (*(void **)(p)=(val))
/*opt Lev2*/
#define GETSIZE(p) (GET(p)&(~0x7))
#define GETALLOC(p) (GET(p)&0x1)
/*opt Lev3*/
#define HEAD(p)	((char *)(p)-WSIZE)
#define FOOT(p) ((char *)(p)+GETSIZE(HEAD(p))-DSIZE)
#define POINT(p) p

#define NEXT(p) ((char *)(p)+GETSIZE(HEAD(p)))
#define LAST(p) ((char *)(p)-GETSIZE(((char *)(p)-DSIZE)))
/*table*/
#define TABLE_NUM 8
#define TABLE_START p_table_start
#define NEXT_TABLE(p,val) ((void **)(p)+val)
#define TABLE(x) (NEXT_TABLE(TABLE_START,x))

static void *p_mm_start;
static void *p_table_start;

static size_t algin_size(size_t size);
static void *heap_coalesce(void* ptra);
static void *heap_extent(size_t wsize);
static void heap_place(void *ptr,size_t size);
static void heap_free(void* ptr);
static void *heap_fit(size_t size);
/****check***/
static void heap_checkheap(int verbose);
static void checkblock(void * ptr);
static void printfblock(void * ptr);
static void watchmemory(char* ptr,size_t size);
static void compare_memory(char *newptr,char *oldptr,size_t size);
/****table**/
static int table_init();
static void table_insert(int,void *ptr);
static void table_out(void *);
static int  table_index(size_t);
static void*table_peek(int);
//=====================================================================//
//==========================algin==================================
static size_t algin_size(size_t size) { return ALIGN(size+WSIZE); }
//==========================table==================================
static int table_init()
{
	void *ptr;
	if((p_table_start=mem_sbrk(TABLE_NUM*DSIZE*sizeof(void *)))==NULL)
	  return -1;
	ptr=p_table_start;
	for(int i=0;i<TABLE_NUM;i++)
	{
		ptr=TABLE(i);
		PUT_L(ptr,NULL);
	}
	return 0;
}

static int table_index(size_t size)
{
	if(size<=(1<<5))
	  return 0;
	else if(size<=(1<<7))
	  return 1;
	else if(size<=(1<<8))
	  return 2;
	else if(size<=(1<<9))
	  return 3;
	else if(size<=(1<<10))
	  return 4;
	else if(size<=(1<<11))
	  return 5;
	else if(size<=(1<<12))
	  return 6;
	else
	  return 7;
}

static void table_insert(int index,void *ptr)
{
	void **table_ptr=TABLE(index);
	void *oldptr=GET_L(table_ptr);
#ifdef DEBUG
	if(oldptr==ptr)
	{
		printf("Insert\n");
		while(1);
	}
#endif
	PUT_L(table_ptr,ptr);
	PUT_L(POINT(ptr),oldptr);
}

static void table_out(void * ptr)
{
	void **table_ptr;
	void *oldptr=NULL;
	void *newptr;
	size_t size=GETSIZE(HEAD(ptr));
	int index=table_index(size);
	table_ptr=TABLE(index);
	oldptr=table_ptr;
	while(oldptr!=NULL)
	{
		if(GET_L(POINT(oldptr))==ptr)
		{
			newptr=GET_L(POINT(GET_L(POINT(oldptr))));
			PUT_L(oldptr,newptr);
			break;
		}
		oldptr=GET_L(POINT(oldptr));
	}
}

static void *table_peek(int index)
{	
	return GET_L(TABLE(index));
}

//==========================heap===================================
static void *heap_coalesce(void* ptr)
{
	size_t prev_alloc=GET_LAST(HEAD(ptr));
	size_t next_alloc=GETALLOC(HEAD(NEXT(ptr)));
	size_t cur_size=GETSIZE(HEAD(ptr));
	int index;
	if(prev_alloc&&next_alloc)
	{
	}
	else if(prev_alloc&&!next_alloc)
	{
		table_out(NEXT(ptr));
		cur_size+=GETSIZE(HEAD(NEXT(ptr)));
		PUT(HEAD(ptr),PACK(cur_size,0));
		PUT(FOOT(ptr),PACK(cur_size,0));
	}
	else if(!prev_alloc&&next_alloc)
	{
		table_out(LAST(ptr));
		cur_size+=GETSIZE(HEAD(LAST(ptr)));
		PUT(FOOT(ptr),PACK(cur_size,0));
		ptr=LAST(ptr);
		PUT(HEAD(ptr),PACK(cur_size,0));
	}
	else if(!prev_alloc&&!next_alloc)
	{
		table_out(NEXT(ptr));
		table_out(LAST(ptr));
		cur_size+=GETSIZE(HEAD(LAST(ptr)));
		cur_size+=GETSIZE(HEAD(NEXT(ptr)));
		PUT(FOOT(NEXT(ptr)),PACK(cur_size,0));
		PUT(HEAD(LAST(ptr)),PACK(cur_size,0));
		ptr=LAST(ptr);
	}
#ifdef DEBUG
	printf("addr:%p\n",NEXT(ptr));
	printf("%d\n",GET_LAST(HEAD(NEXT(ptr))));
#endif
	SET_LAST(HEAD(NEXT(ptr)));
#ifdef DEBUG
	printf("%d\n",GET_LAST(HEAD(NEXT(ptr))));
#endif
	index=table_index(GETSIZE(HEAD(ptr)));
	table_insert(index,ptr);
	return ptr;
}

static void *heap_extent(size_t wsize)
{
	char* ptr;
	size_t size;
	
	size = (wsize%2) ? ((wsize+1)*WSIZE):(wsize*WSIZE);
	if((ptr=mem_sbrk(size))==(void *)-1)
	  return NULL;
	PUT(HEAD(ptr),PACK(size,0));
	PUT(FOOT(ptr),PACK(size,0));
	PUT(FOOT(ptr)+WSIZE,PACK(0,1));
	return heap_coalesce(ptr);
}

static void heap_place(void *ptr,size_t size)
{
	size_t oldsize=GETSIZE(HEAD(ptr));
	size_t oldlast=GET_LAST(HEAD(ptr));
	table_out(ptr);
	if(size>=oldsize*3/4)
	{
		PUT(HEAD(ptr),PACK(oldsize,1));
		if(!oldlast)
		  SET_LAST(HEAD(ptr));
		CLEAR_LAST(HEAD(NEXT(ptr)));
	}
	else
	{
		PUT(HEAD(ptr),PACK(size,1));
		PUT(HEAD(NEXT(ptr)),PACK(oldsize-size,0));
		PUT(FOOT(NEXT(ptr)),PACK(oldsize-size,0));
		heap_coalesce(NEXT(ptr));
	}
}

static void heap_free(void* ptr)
{
	size_t size =GETSIZE(HEAD(ptr));
	size_t oldlast=GET_LAST(HEAD(ptr));
	PUT(HEAD(ptr),PACK(size,0));
	PUT(FOOT(ptr),PACK(size,0));
	if(!oldlast)
		SET_LAST(HEAD(ptr));
	heap_coalesce(ptr);
}

static void *heap_fit(size_t size)
{
	int flag=0;
	int index=table_index(size);
	void* table_ptr=NULL;
	void* oldptr=NULL;
	for(;index<TABLE_NUM;index++)
	{
		table_ptr=TABLE(index);
		oldptr=GET_L(table_ptr);
		while(oldptr!=NULL)
		{
			if(GETSIZE(HEAD(oldptr))>size)
			{
				flag=1;
				break;
			}
			oldptr=GET_L(POINT(oldptr));
		}
		if(flag)
		  break;
	}
	
	return oldptr;
}

//=========================usr====================================
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	table_init();
	if((p_mm_start=mem_sbrk(4*WSIZE))==(void *)-1)
	  return -1;
	PUT(p_mm_start,0);
	PUT((char *)(p_mm_start)+WSIZE,PACK(DSIZE,1));
	PUT((char *)(p_mm_start)+2*WSIZE,PACK(DSIZE,1));
	PUT((char *)(p_mm_start)+3*WSIZE,PACK(0,1));
	p_mm_start=(char *)(p_mm_start)+DSIZE;

	if(heap_extent(CHUNKSIZE/WSIZE)==NULL)
	  return -1;
#ifdef DEBUG
	printf("LIST INIT:\n");
	mm_checkheap(1);
#endif
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	char *ptr;
	if(size==0)
	  return NULL;

	size=algin_size(size);
	if((ptr=heap_fit(size))==NULL)
	{
		if((ptr=heap_extent(MAX(CHUNKSIZE,size)/WSIZE))==NULL)
		  return NULL;
	}
	heap_place(ptr,size);
#ifdef DEBUG
	printf("LIST MALLOC: %p SIZE: %d \n",ptr,size);
	mm_checkheap(1);
#endif
	return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	heap_free(ptr);
#ifdef DEBUG
	printf("LIST FREE: %p\n",ptr);
	mm_checkheap(1);
#endif
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t oldsize=GETSIZE(HEAD(ptr))-WSIZE;
	size_t newsize=algin_size(size);
	char copy[oldsize];
	if(size==0)
	{
		mm_free(ptr);
		return 0;
	}

	if(ptr==NULL)
	{
		ptr=mm_malloc(size);
	}

	if(newsize==oldsize)
		return ptr;
	
	if(newsize<oldsize)
	{
		heap_place(oldptr,newsize);
	}
	else if(newsize>oldsize)
	{
		memcpy(copy,oldptr,oldsize);
		mm_free(oldptr);
		if((newptr=mm_malloc(size))!=NULL)
		{
#ifdef MEMWATCH
			watchmemory(oldptr,oldsize);
#endif
			memcpy(newptr,copy,oldsize);
#ifdef MEMWATCH
			watchmemory(newptr,oldsize);
			compare_memory(newptr,oldptr,oldsize);
#endif
			ptr=newptr;		
		}
	}

#ifdef DEBUG
	printf("LIST REALLOC: %p\n ",oldptr);
	mm_checkheap(1);
#endif
	return ptr;
}


//-----------check
void mm_checkheap(int verbose)  
{ 
    heap_checkheap(verbose);
}

static void heap_checkheap(int verbose)
{
	int check_free=0;
	int check_free_table=0;
	void *ptr=p_mm_start;
	void *c_ptr;
	if(verbose)
	  printf("Heap Start:%p\n",p_mm_start);

	for(int i=0;GETSIZE(HEAD(ptr))>0;ptr=NEXT(ptr),i++)
	{
		printf("%d:\n",i);
		printfblock(ptr);
		checkblock(ptr);
		if(!GETALLOC(HEAD(ptr)))
		  check_free++;
	}
	for(int i=0;i<TABLE_NUM;i++)
	{
		printf("Table %d:\n",i);
		c_ptr=GET_L(TABLE(i));
		while(c_ptr!=NULL)
		{
		  check_free_table++;
		  printfblock(c_ptr);
		  c_ptr=GET_L(POINT(c_ptr));
		}
	}
	if(check_free_table!=check_free)
	  printf("Error:Free Block Fade\n");
	printf("END\n\n");
}

static void printfblock(void* ptr)
{
	printf("HEAD ADDR:%p\n",ptr);
	printf("SIZE     :%d\n",GETSIZE(HEAD(ptr)));
	printf("ALLOC    :%d\n",GETALLOC(HEAD(ptr)));
}

static void checkblock(void* ptr)
{
	//1.check align
	//2.check head and foot
	if((size_t)ptr%DSIZE)
	  printf("Error:No align\n");
}

static void watchmemory(char* ptr,size_t size)
{
	printf("%p : %d\n",ptr,size);
	for(int i=0;i<size;i++)
	  printf("%x ",(unsigned char)(*(ptr+i)));
	printf("\n");
}

static void compare_memory(char *newptr,char *oldptr,size_t size)
{
	for(int i=0;i<size;i++)
	{
		if(oldptr[i]!=newptr[i])
		{
			printf("Campare Error:");
			printf("New\naddr:%p content:%x\n",newptr,(unsigned char)(newptr[i]));
			printf("Old\naddr:%p content:%x\n",newptr,(unsigned char)(oldptr[i]));
			while(1);
		}
	}
}




