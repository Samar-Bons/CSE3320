/* 
  Name: Samarjit Singh Bons
  ID:   1001623236 
  Optional: compile and run with "image.txt" in the same directory 
  to add visual appeal.
 
*/


/*
        __  ___            _____ __         ____   ___ ____ 
       /  |/  /___ __   __/ ___// /_  ___  / / /  <  // __ \
      / /|_/ / __ `/ | / /\__ \/ __ \/ _ \/ / /   / // / / /
     / /  / / /_/ /| |/ /___/ / / / /  __/ / /   / // /_/ / 
    /_/  /_/\__,_/ |___//____/_/ /_/\___/_/_/   /_(_)____/  
                                                                 
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Mav shell only supports 10 arguments and one command

#define MAX_LEN 1000            // Max length for image file to print (optional) 

#define MAX_HISTORY_SIZE 15     // Linked List for command history 
                                // will have capacity of 15

// Linked List node to store cmd_str and PID 
// of a process for history/listpids for last 15 commands

typedef struct node
{
  char cmd_str_node[MAX_COMMAND_SIZE];
  pid_t process_id;
  struct node *next;  
} node;


// Function to add current cmd_str and pid 
// to the fixed capacity Linked List of last 15 commands
void print_image(FILE *fptr)
{
  char read_string[MAX_LEN];
  while(fgets(read_string, sizeof(read_string), fptr)!= NULL)
  printf("%s",read_string);
}

void add_command(node **head, pid_t pid, char *cmd_str)
{
  node *new = (node*) malloc (sizeof(node));      // Initialize a new node
  strcpy(new->cmd_str_node,cmd_str);              // with the given process id,
  new->process_id = pid;                          // command string and NULL next pointer
  new->next = NULL;

  if(*head == NULL)
  {
    *head = new;                          // If Linked list is empty
  }                                       // new node is head

  else
  {
                               
    int size = 1;                        // Current confirmed size of Linked List

    
    node *temp = *head;             // temp node to iterate the LL

    while(temp->next != NULL)       // Iterating through the Linked List 
    {                               // to reach the last node
      size++;
      temp = temp->next;
    }

    temp->next = new;               // Attach the latest node to the Linked List

    
    if(size == MAX_HISTORY_SIZE)
    {
      node *temp_head = *head;       // Creating a temp node holding original head
      *head = (*head)->next;         // so that we can set the 2nd element as new head
      free(temp_head);               // and free the first node
    }       
  }
}


// Function to go through the historic command linked list if it exists 
// and return the command string at index position. (linked list starts at 0)

char *retrieve_command(int index,node *head)
{
    
  node *temp = head;                   // Temp node pointing at head for iteration
  
  int i;
  for(i=1;i<=index;i++)                // If index 0 then no iteration loop  
  {                                       
    temp = temp->next;
    if(temp == NULL) return NULL;      // If no command found
  }
  
  return temp->cmd_str_node;
}


// Function to print all the commands in linked list
void print_command_history(node *head)
{
  if ( head == NULL)
  {
    return;
  }

  node *temp = head;                    //Iterator

  int index = 0;
  while(temp != NULL)
  {
    printf("%d: %s\n",index,temp->cmd_str_node);
    temp = temp->next;
    index++;
  }
}


// Function to print process IDS of commands in linked list. 
// Note - Since cd, history, listpids, showpids are functions
// of the MavShell 1.0 and not processes, they have the same 
// process ID as MavShell

void print_command_pid(node *head)
{
  if ( head == NULL )
  {
    return;
  }

  node *temp = head;
  int index = 0;

  while(temp != NULL)
  {
    printf("%d: %ld\n",index,(long)temp->process_id);
    temp = temp->next;
    index++;
  }
}


int main()
{
  pid_t current_pid = getpid();            // Obtain current pid for MavShell
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  node *head = NULL;                       // Empty head for Linked List
  
 
  // Added optional functionality to print visually appealing graphic at 
  // startup of MavShell 1.0
  // can be enabled by storing "image.txt" in the same directory

  char* filename = "image.txt";
  FILE *fptr = NULL;

  if((fptr = fopen(filename,"r")) != NULL)
  {
    print_image(fptr);
  }

  printf("\n\n");

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline. The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );


    // Quitly and with no other output, shell prints
    // another prompt by skipping through the rest of the loop if 
    // input is \n

    if(cmd_str[0] == '\n')
    {   
      continue;
    }


    // When command is !n where n is a number between 0 and 15
    // and makes our shell re-run the nth command in the Linked List
    else if ( cmd_str[0] == '!')
    {
      int integer;
      if(cmd_str[2] != '\0')              // Cases for parsing n
      {                                   // when it is single and double digit
        char a = cmd_str[1];
        char b = cmd_str[2];
        char str1[3] = {a,b,'\0'};
        integer = atoi(str1); 
      }
      else
      {
        char c = cmd_str[1];
        char str2[2] = {c,'\0'};
        integer = atoi(str2);
      }

     
      
      char *res_str = retrieve_command(integer,head);  // Retrieving the nth command
                                                       // string OR NULL if it does 
      if(res_str == NULL)                              // not exist
      {
        printf("\nCommand not found.\n");
        print_command_history(head);
        continue;
      }
      strcpy(cmd_str,res_str);
      free(res_str);

    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Shell will exit with status 0 if command is "exit" or "quit"
    if(strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0)
    {
      exit(0);                 
    }

    // Shell will print command history if command is "history"
    else if(strcmp(token[0],"history") == 0) 
    {
      print_command_history(head);
      add_command(&head,current_pid,cmd_str);
      continue;
    }

    // Shell will print past pids from history if command is 
    // "showpids" OR "listpids"
    else if(strcmp(token[0],"listpids") == 0 || strcmp(token[0],"showpids") == 0)
    {
      print_command_pid(head);
      add_command(&head,current_pid,cmd_str);
      continue;
    }

    // Shell will change directory depending on arguments if command is "cd"
    else if(strcmp(token[0],"cd") == 0)
    {
      if(token[1] == NULL)
      {
        chdir(getenv("HOME"));       // if command = cd then go to /home
      }
      else 
      {
        chdir(token[1]);             // else go to the directory in argument or ..
      }
      add_command(&head,current_pid,cmd_str);
      continue;
    }


    // Forking a child process will then will be used to create a new process

    pid_t pid = fork();
    if ( pid == -1)
    {
      // When fork() returns -1, an error happened.
      perror("fork failed: ");
      exit( EXIT_FAILURE );
    }
    else if ( pid == 0)
    {
      // We are inside the child process
      // We call execvp with arguments token[0] which is the command
      // and the entire command string
      int ret = execvp(token[0],&token[0]);
      
      if ( ret == -1 )
      {
        // When command is invalid
        printf("%s: Command not found.\n\n",token[0]);
      }
    }
    else
    {
      // We are in the parent process
      add_command(&head,pid,cmd_str);     // Add the child's
      int status;                         // PID and command string
      wait( &status );                    // to the Linked List
    }


    free( working_root );
    

  }
  return 0;
}
