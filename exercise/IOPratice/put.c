#include <csapp.h>

int main()
{
    FILE* f = fopen("io.txt", "a+"); // open for read/write
	FILE* f2=fopen("io.txt","r");
	if (f == NULL) 
	{
		perror("fopen");
		return 0;
	}

	char* c1 = "efgh";
	fwrite(c1, 1, 4, f); // make sure the stream has 4 bytes
	fflush(f); // reset position indicator
	//fwrite(c1, 1, 1, f);
    
	char c2 = '\0';
	fread(&c2, 1, 1, f2); // read after write	
	printf("%c\n", c2); // what's in c2?

	//fwrite(&c2, 1, 1, f); // write after read
	fclose(f); // what's in the file now?

	return 0;
}
