/*

Samarjit Singh Bons
1001623236
gcc -pthread -o substring substring.c


*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000
#define NUM_THREADS 4

pthread_mutex_t mutex;

int total = 0;
int n1,n2; 
char *s1,*s2;

FILE *fp;

int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }
    
    s1=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }
    
    s2=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }
    
    /*read s1 s2 from the file*/
    
    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2)-1; /*length of s2*/
    
    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }
}

// Thread function to scan a sector of the string
void* num_substring ( void * _tid )
{
    int tid = *((int *) _tid);  
    int start = tid * (n1/NUM_THREADS);
    int end = (tid+1) * (n1/NUM_THREADS);

    if(tid == NUM_THREADS-1)
    {
        end = end - n2;
    }


    int i,j,k;
    int count ;
    int th_total = 0;

    // Loop to iterate from start to end for a given sector
    for (i = start; i <= end; i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2)
                // Incrementing the amount of substrings found by the current thread
                th_total++; /*find a substring in this step*/
         }
    }
    pthread_mutex_lock(&mutex);

    // Add the number of substrings found by the current thread 
    // to the grand total 
    total = total + th_total;

    pthread_mutex_unlock(&mutex);
    
}
    
int main(int argc, char *argv[])
{
    int count,i,status ;
    pthread_mutex_init(&mutex,NULL);

    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] ) ;

    struct timeval start, end;
    float mtime; 
    int secs, usecs;    

    gettimeofday(&start, NULL);

    
    int ints[NUM_THREADS]; 
    pthread_t tid[NUM_THREADS];
    
    // Starting threads
    for(i = 0; i < NUM_THREADS; i++)
    {
        ints[i] = i;
        if(pthread_create(&tid[i],NULL,num_substring,&ints[i]))
        {
            printf("Error creating thread %d\n",ints[i]);
        }  
    }

    // joining threads
    for(i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_join(tid[i],NULL))
        {
            printf("Error joining thread %d\n",i);
        }
    }


    count = total;

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , count) ;
    printf ("Elapsed time is : %f milliseconds\n", mtime );
    printf ("Number of threads used : %d \n", NUM_THREADS );
    if( s1 )
    {
      free( s1 );
    }

    if( s2 )
    {
      free( s2 );
    }

    return 0 ; 
}
