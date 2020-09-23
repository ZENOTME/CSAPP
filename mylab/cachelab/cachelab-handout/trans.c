/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void transpose_lastfun(int M,int N,int A[N][M],int B[M][N])
{
	int temp=0;
	int index=-1;
	for(int i=0;i<N;i+=8)
	{
	  for(int j=0;j<M;j+=8)
	  {
		  for(int i1=i;i1<i+8&&i<N;i1++)
		  {
			for(int j1=j;j1<j+8&&j<M;j1++)
			{
				if(j1==i1)
				{
					temp=A[i1][j1];
					index=j1;
				}
				else
				{
					B[j1][i1]=A[i1][j1];
				}
			}
			if(index!=-1)
			{
				B[index][index]=temp;
				temp=0;
				index=-1;
			}
		  }
		}
	}
}



char transpose_fun_desc[]="Function one";
void transpose_fun(int M,int N, int A[N][M],int B[M][N])
{
	int k=0;
	int temp0,temp1,temp2,temp3,temp4,temp5,temp6,temp7=0;
	for(int i=0;i<N;i+=8)
	{
		for(int j=0;j<M;j+=8)
		{
			for(k=i;k<i+4;k++)
			{
				temp0=A[k][j];
				temp1=A[k][j+1];
				temp2=A[k][j+2];
				temp3=A[k][j+3];
				temp4=A[k][j+4];
				temp5=A[k][j+5];
				temp6=A[k][j+6];
				temp7=A[k][j+7];
				B[j][k]=temp0;
				B[j+1][k]=temp1;
				B[j+2][k]=temp2;
				B[j+3][k]=temp3;
				B[j+3][k+4]=temp4;
				B[j+2][k+4]=temp5;
				B[j+1][k+4]=temp6;
				B[j][k+4]=temp7;
			}
			for(int g=j;g<j+4;g++)
			{
				temp0=A[k][j+3-(g-j)];
				temp1=A[k+1][j+3-(g-j)];
				temp2=A[k+2][j+3-(g-j)];
				temp3=A[k+3][j+3-(g-j)];
				temp4=A[k][g+4];
				temp5=A[k+1][g+4];
				temp6=A[k+2][g+4];
				temp7=A[k+3][g+4];

				B[g+4][k]=temp4;
				B[g+4][k+1]=temp5;
				B[g+4][k+2]=temp6;
				B[g+4][k+3]=temp7;
				B[g+4][k-4]=B[j+3-(g-j)][k];
				B[g+4][k-3]=B[j+3-(g-j)][k+1];
				B[g+4][k-2]=B[j+3-(g-j)][k+2];
				B[g+4][k-1]=B[j+3-(g-j)][k+3];

				B[j+3-(g-j)][k]=temp0;
				B[j+3-(g-j)][k+1]=temp1;
				B[j+3-(g-j)][k+2]=temp2;
				B[j+3-(g-j)][k+3]=temp3;
			}
		}
	}
}
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int i,j,k;
	for(i=0;i<N;i+=8){
		for(j=0;j<M;j+=8){
			for(k=i;k<i+8;k++){
				int temp0=A[k][j];
				int temp1=A[k][j+1];
				int temp2=A[k][j+2];
				int temp3=A[k][j+3];
				int temp4=A[k][j+4];
				int temp5=A[k][j+5];
				int temp6=A[k][j+6];
				int temp7=A[k][j+7];
				
				B[j][k]=temp0;
				B[j+1][k]=temp1;
				B[j+2][k]=temp2;
				B[j+3][k]=temp3;
				B[j+4][k]=temp4;
				B[j+5][k]=temp5;
				B[j+6][k]=temp6;
				B[j+7][k]=temp7;
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(transpose_fun, transpose_submit_desc); 
    registerTransFunction(transpose_lastfun, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

