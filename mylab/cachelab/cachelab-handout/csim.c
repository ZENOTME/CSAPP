#include "cachelab.h"
#include "unistd.h"
#include "getopt.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "stdint.h"
int read_file(char* path,char** string);

typedef struct {
	uint8_t  valid;
	uint64_t   tag;
	int8_t  LRUnum;
}CacheLine;

typedef struct {
	uint8_t vs;
	uint8_t vb;
	uint8_t vS;
	uint8_t vE;
	CacheLine** line;
	uint8_t parse_set;
	uint64_t parse_tag;
	uint32_t hit_count;
	uint16_t miss_count;
	uint16_t eviction_count;
}Cache;

Cache* newCache(Cache** moudle,uint8_t s,uint8_t E,uint8_t b)
{
	(*moudle)=(Cache*)calloc(1,sizeof(Cache));
	(*moudle)->vE=E;
	(*moudle)->vs=s;
	(*moudle)->vb=b;
	(*moudle)->vS=(1U<<s);
	(*moudle)->line=(CacheLine**)calloc((*moudle)->vS,sizeof(Cache*));
	for(int i=0;i<(*moudle)->vS;i++)
	{
		(*moudle)->line[i]=(CacheLine*)calloc((*moudle)->vE,sizeof(Cache));
	}
	return (*moudle);
}
void delCache(Cache** moudle)
{
	for(int i=0;i<(*moudle)->vS;i++)
	{
		free((*moudle)->line[i]);
	}
	free((*moudle)->line);
	free((*moudle));
}
void parseCache(Cache* moudle,uint64_t address)
{
	int set_complement=~((int)0x80000000>>(31-moudle->vs));
	moudle->parse_set=(address>>(moudle->vb))&set_complement;
	moudle->parse_tag=(address>>(moudle->vb)>>(moudle->vs));
}
int8_t checkCache(Cache* moudle)
{
	uint8_t flag=0;
	uint8_t index=0;
	uint8_t set=moudle->parse_set;
	uint64_t tag=moudle->parse_tag;
	for(int i=0;i<moudle->vE;i++)
	{
		if(moudle->line[set][i].valid==1)
		{
			if(moudle->line[set][i].tag==tag)
			{
				flag=1;
				index=i;
				break;
			}
		}
	}
	if(flag)
	{
		for(int i=0;i<moudle->vE;i++)
		{
			moudle->line[set][i].LRUnum+=1;
		}
		moudle->line[set][index].LRUnum=0;
		moudle->hit_count+=1;
		return 0;
	}
	return 1;
}
// 1 miss
// 0 eviction
int8_t addCache(Cache* moudle)
{
	uint8_t flag=0;
	uint8_t index=0;
	uint8_t set=moudle->parse_set;
	uint64_t tag=moudle->parse_tag;
	for(int i=0;i<moudle->vE;i++)
	{
		if(moudle->line[set][i].valid==0)
		{
			index=i;
			flag=1;
		}
		moudle->line[set][i].LRUnum+=1;
	}
	if(flag)
	{
		moudle->line[set][index].tag=tag;
		moudle->line[set][index].valid=1;
		moudle->line[set][index].LRUnum=0;
		moudle->miss_count+=1;
		return 1; 
	}
	int8_t biggest=moudle->line[set][0].LRUnum;
	index=0;
	for(int i=1;i<moudle->vE;i++)
	{
		if(biggest<moudle->line[set][i].LRUnum)
		{
			biggest=moudle->line[set][i].LRUnum;
			index=i;
		}
	}
	moudle->line[set][index].tag=tag;
	moudle->line[set][index].LRUnum=0;
	moudle->miss_count+=1;
	moudle->eviction_count+=1;
	return 0;
}
void processCache(Cache * moudle,uint64_t address,char ins)
{
	parseCache(moudle,address);
	if(ins=='S'||ins=='L'){
	if(checkCache(moudle)){addCache(moudle);}
	}
	if(ins=='M'){
	if(checkCache(moudle))
	{addCache(moudle);}
	if(checkCache(moudle))
	{addCache(moudle);}
	}
}

int read_file(char* path,char** string)
{
	FILE *fp;
	fp=fopen(path,"r");
	if(fp==0){
		return 1;
	}
	fseek(fp,0,SEEK_END);
	int file_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	*string=(char *)malloc(file_size*sizeof(char));
	fread(*string,file_size,sizeof(char),fp);
	fclose(fp);
	return 0;
}

int main(int argc,char *argv[])
{
	int opt;
	char* t_data=0;
	char vs,vb,vE;
	char visable=0;
	while((opt=getopt(argc,argv,"t:s:E:b:v"))!=-1)
	{
		switch(opt)
		{
			case 't':
				if(read_file(optarg,&t_data)){
					printf("Error to open the file");
					return 0;
				}
				break;
			case 's':
				vs=atoi(optarg);
				break;
			case 'E':
				vE=atoi(optarg);
				break;
			case 'b':
				vb=atoi(optarg);
				break;
			case 'v':
				visable=1;
				break;
			default:
				printf("Error to arrguement");
				return 0;
				break;
		}
	}
	printf("E=%d b=%d s=%d\n",vE,vb,vs);	
	//============Alreay get the data=======================
	//Cache Init
	Cache* cache;
	cache=newCache(&cache,vs,vE,vb);
	//analyse the trace
	char* command;
	const char s[2]="\n";
	char cins[2]="";
	long vaddress;
	unsigned int vusesize;
	command=strtok(t_data,s);
	while(command!=NULL)
	{
		sscanf(command,"%s %lx,%u",cins,&vaddress,&vusesize);
		processCache(cache,vaddress,cins[0]);
		command=strtok(NULL,s);
	}
	printSummary(cache->hit_count,cache->miss_count,cache->eviction_count);
	delCache(&cache);
	return 0;
}

