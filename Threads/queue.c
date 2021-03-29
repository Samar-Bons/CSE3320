#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 5
#define NONSHARED 1

sem_t char_in_buffer, char_printed_out;
int char_waiting = 1;
pthread_mutex_t mutex;

char queue[SIZE];
int front = -1, rear = -1;

// Function to check if the queue is full
int isFull() 
{
  if ((front == rear + 1) || (front == 0 && rear == SIZE - 1))
  { 
    return 1;
  }

  return 0;
}

// Function to check if the queue is empty
int isEmpty() 
{
  if (front == -1)
  {
    return 1;
  }
  return 0;
}

// Function to add an element to the queue
void enQueue(char element) 
{
  if (isFull())
    printf("\n Queue is full!! \n");
  else 
  {
    if (front == -1) front = 0;
    rear = (rear + 1) % SIZE;
    queue[rear] = element;
    
  }
}

// Function to remove an element
char deQueue() 
{
  char element;
  if (isEmpty()) 
  {
    printf("\n Queue is empty !! \n");
    return (-1);
  } 
  else 
  {
    element = queue[front];
    if (front == rear) 
    {
      front = -1;
      rear = -1;
    } 
    
    else 
    {
      front = (front + 1) % SIZE;
    }
    
    return (element);
  }
}

// Function for Producer thread
void * Producer (void * arg)
{
  printf("Producer Created\n");
  FILE *fp;
  char ch;

  // opening the file to read
  fp = fopen("message.txt", "r");

  if(fp == NULL)
  {
    printf("File not available\n");
    exit(1);
  }

// An infinite while loop which will read and enqueue every character 
// from the file and then break once it reaches EOF
  while(1)
  {
    ch = fgetc(fp);
    if(ch == EOF)
    {
      fclose(fp);
      printf("\nExiting Producer\n");
      sem_post(&char_in_buffer);
      break;
    }

    sem_wait( &char_printed_out );

    enQueue(ch);

    sem_post(&char_in_buffer);

   
  }
}


// Function for Consumer thread
void * Consumer( void * arg ) 
{
  
  printf( "Consumer created\n" );

  // infinite while loop which will take values from the Queue 
  // and print it. Once the producer thread has ended and the queue is empty,
  // the loop breaks. 
  while( 1 )
  {
    
    sem_wait( &char_in_buffer ); 

    char ch = deQueue();

    if(ch == -1)
    {
      printf("\nExiting Consumer\n");
      break;
    }
    
    
    printf("%c\n",ch);

    
    sem_post( &char_printed_out );

    
  }

}

int main( int argc, char *argv[] ) 
{
  //time_t t;

  //srand( ( unsigned int ) time( & t ) );

  // Adding a dummy value to the queue to start the program.
  // A truly empty queue causes a deadlock. 
  enQueue(' ');
  
  pthread_t producer_tid;  
  pthread_t consumer_tid;  

  // Initializing semaphores
  sem_init( & char_printed_out, NONSHARED, 0 );  
  sem_init( & char_in_buffer, NONSHARED, 1 );   

  // creating producer and consumer threads
  pthread_create( & producer_tid, NULL, Producer, NULL );
  pthread_create( & consumer_tid, NULL, Consumer, NULL );
  
  // joining the threads
  pthread_join( producer_tid, NULL );
  pthread_join( consumer_tid, NULL );
  

}
