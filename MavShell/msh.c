// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 7f704d5f-9811-4b91-a918-57c1bb646b70
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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

#define MAX_HISTORY_LENGTH 15   // Linked List for command history will have capacity of 15 

//Linked List node to store cmd_str and PID 
//of a process for history/listpids for last 15 commands

typedef struct node
{
  char cmd_str_node[MAX_COMMAND_SIZE];
  pid_t process_id;
  struct node *next;  
} node;

//Function to add current cmd_str and pid 
//to the Linked List of last 15 commands

node *head = NULL;

void add_command(node **head, pid_t pid, char *cmd_str)
{
  node *new = (node*) malloc (sizeof(node));
  strcpy(new->cmd_str_node,cmd_str);
  new->process_id = pid;
  new->next = NULL;

  if(*head == NULL)
  {
    *head = new;
  }

  else
  {
    //Current confirmed number of nodes in Linked List
    int length = 1;

    //temp node to iterate the LL
    node *temp = *head;

    while(temp->next != NULL)
    {
      length++;
      temp = temp->next;
    }

    temp->next = new;

    //Removing the oldest command in history if adding new command
    // to max size linked list
    if(length == MAX_HISTORY_LENGTH)
    {
      node *temp_head = *head;
      *head = (*head)->next;
      free(temp_head);
    }       
  }
}

void print_history(node *head)
{
  if ( head == NULL)
  {
    return;
  }

  node *temp = head;

  
  printf("\n");

  int index = 0;
  
  //Print all the commands in history
  while(temp != NULL)
  {
    printf("%d: %s\n",index++,temp->cmd_str_node);
    temp = temp->next;
    
  }
}

void print_pid(node *head)
{
  if ( head == NULL )
  {
    return;
  }

  node *temp = head;
  int index = 0;

  while(temp != NULL)
  {
    if(temp->process_id)
    {
      printf("%d: %ld\n",index++,(long)temp->process_id);
    }
    temp = temp->next;
  }
}



int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );


    //Quitly and with no other output, shell prints
    //another prompt by skipping through the rest of the loop if 
    //input is \n

    if(cmd_str[0] == '\n')
    {   
      continue;
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

    //Shell will exit with status 0 if command is "exit" or "quit"
    if(strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0)
    {
      exit(0);
    }
    else if(strcmp(token[0],"history") == 0)
    {
      print_history(head);
      add_command(&head,0,cmd_str);
      continue;
    }

    else if(strcmp(token[0],"listpids") == 0 || strcmp(token[0],"showpids") == 0)
    {
      print_pid(head);
      add_command(&head,0,cmd_str);
      continue;
    }

    /* if ((strcmp(token[0],"quit") == 0) || (strcmp(token[0],"exit") == 0 ))
    {
        exit(0);
    }
    */
    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    //  int token_index  = 0;
    //for( token_index = 0; token_index < token_count; token_index ++ ) 
    //{
    //  printf("token[%d] = %s\n", token_index, token[token_index] );  
    //}
    pid_t pid = fork();
    if ( pid == -1)
    {
      // When fork() returns -1, an error happened.
      perror("fork failed: ");
      exit( EXIT_FAILURE );
    }
    else if ( pid == 0)
    {

      int ret = execvp(token[0],&token[0]);
      
      if ( ret == -1 )
      {
        printf("%s: Command not found.\n",token[0]);
      }
    }
    else
    {
      add_command(&head,pid,cmd_str);
      int status;
      wait( &status );
    }


    free( working_root );

  }
  return 0;
}
