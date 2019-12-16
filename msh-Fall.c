/*Name: Kashif Hussain
ID: 1001409065 */

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

#define MAX_NUM_ARGUMENTS 11   // Mav shell will support 10 arguments in addition to command

static void handle_SIGINT (int sig)
{
  //this will prevent ctrl-z signal from stopping the program
}

static void handle_SIGTSTP (int sig)
{
    //this will prevent crtl-c signal from killing the programming

}

int main()
{

 

    int listpids[15]; // array to store the pids for listpids command
    int pid_index=0; // intializing the index to 0 for use in storing pids in array
    int history_index=0; // initializing the index to 0 for use in storing commands in array
    int command=-1; // this is a check to see if !n command is run
    int max_pid=0; 
    int max_history=0;
    char *cmd_str2 =NULL;
    pid_t process=0; // to get the pid of process to use in backgrounding a suspended process
    char cmd_history[50][MAX_COMMAND_SIZE+1]; // array for storing history commands
   
    
  memset(listpids, 0, 15); // initializing listpids array to 0

  struct sigaction actINT;
  memset (&actINT, '\0', sizeof(actINT));

  struct sigaction actSTP;
  memset (&actSTP, '\0', sizeof(actSTP));
 
  //  Set the handler to use the function handle_signal()
   actINT.sa_handler = &handle_SIGINT;
   actSTP.sa_handler=&handle_SIGTSTP;

 // Installing signal handlers and check the return value.

   if (sigaction(SIGINT , &actINT, NULL) < 0) 
  {
    perror ("SIGINT not working: ");
    exit(1);
  }
  if (sigaction(SIGTSTP , &actSTP, NULL) < 0) 
  {
    perror ("SIGTSTP not working: ");
    exit(1);
  }

 

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  

  while( 1 )
  {

     if(command!=-1)   // if there is an !n command run, execute this if command
     {
         cmd_str2=cmd_history[command];
         command=-1;  
     }
     else
    {  
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
   //initializing array of history for commands
    memset(cmd_history[history_index],'\0',MAX_COMMAND_SIZE+1); 
      
    // intializing the array to null to be used for storing commands

    //To save a history of commands typed into the shell

     /* char delim[] = ";";
     char *ptr = strtok(cmd_str, delim);
    	while(ptr != NULL)
	    {
        process_command(ptr);
        ptr = strtok(NULL, delim);
      }
      */

    

    if(history_index<50)
    {
        strncpy(cmd_history[history_index],cmd_str, strlen(cmd_str));
        history_index++;
        max_history++;
    }
    else
    {
      
        history_index=0; //resetting the counter for taking the last 50 commands
        max_history=50;
        strncpy(cmd_history[history_index],cmd_str, strlen(cmd_str));
        history_index++;
        
    }

      cmd_str2=cmd_str;  // allocating the command to cmd_str2 if !n command was not run
                         
    }

 
    
    


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr; 
    //char *arg_ptr2;                                        
                                                           
    char *working_str  = strdup( cmd_str2 ); 

  //SHOULD BE LIKE THAT IN TH END
/*  char delim[] = ";";
  char *ptr = strtok(cmd_str, delim);
    	while(ptr != NULL)
	    {
        process_command(ptr);
        ptr = strtok(NULL, delim);
      }
*/
  
 

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

 
    
 
    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    
     if(token[0]==NULL)
     {
       // pressing enter on null command skips the loop for next iteration of msh>
     }
     else if(strcmp(token[0],"exit")==0 || strcmp(token[0],"quit")==0) // to exit the program
      {
          int t;
          for(t=0;t<token_count;t++)
          {
              free(token[t]);
          }
         
          free(working_str);
         
       
          exit(0);
      }
     else if(strcmp(token[0],"cd")==0) //changing directories
     {
      
        if (chdir(token[1]) != 0)
        {
        perror("No such directory");
        }
    }
     

     else if(strcmp(token[0],"listpids")==0) // listing all pids from listpids array
     {
          int i;
          int index=pid_index;
          if(max_pid<15)   // will print amount of pids without displaying any empty values
          {
              for(i=0;i<index;i++)
              {
                printf("%d:  %d\n",i,listpids[i]);
              }
          }
          else
          {
               for(i=0;i<15;i++)  // will print pids upto 15, and will overwrite older pids if needed.
               {
               printf("%d:  %d\n",i,listpids[i]);
               }
          }
          
    
            
    }
     

      else if(strcmp(token[0],"bg")==0) // backgrounding a suspended process
     {
         kill(process, SIGCONT);
     }
        
     

     else if(strcmp(token[0],"history")==0) //listing all commands printed by user
    {
        int j;
       if(max_history<=50)
       {
        for(j=0;j<history_index;j++)
        {
            printf("%d:  %s\n",j,cmd_history[j]);
        }
       }
       else
       {
          for(j=0;j<50;j++)
        {
            printf("%d:  %s\n",j,cmd_history[j]);
        }
       }
       

    }

    
   //   else if(token[0][0]=='!')   
   else if (strcmp("!0", token[0]) == 0 || strcmp("!1", token[0]) == 0 || strcmp("!2", token[0]) == 0 || strcmp("!3", token[0]) == 0 || strcmp("!4", token[0]) == 0 || strcmp("!5", token[0]) == 0 || strcmp("!6", token[0]) == 0 || strcmp("!7", token[0]) == 0 || strcmp("!8", token[0]) == 0 || strcmp("!9", token[0]) == 0 || strcmp("!10", token[0]) == 0 || strcmp("!11", token[0]) == 0 || strcmp("!12", token[0]) == 0 || strcmp("!13", token[0]) == 0 || strcmp("!14", token[0]) == 0 || strcmp("!15", token[0]) == 0 || strcmp("!16", token[0]) == 0)
    {    // these are for commands from !1 to !15 to be run again from history
        
          //char c[3] = {0};
          //c[0]=token[0][1];
          char *c;
		  	  c = strtok(token[0], "!");
          command=atoi(c);
          if(command>history_index)
          {
               perror(":Command not in history\n");
              
          }
        }

    else 
    {
        pid_t child_pid = fork();
       

        if(child_pid<0) 
        {

            perror("Fork Failed");
            exit(1);
        }

        else if(child_pid==0)
        {
            if((execvp(*token,token))<0)
            {
                perror(":Command not found");
                exit(1);
            }
        }
        else
        {
          
            pid_t pids=child_pid;
            process=child_pid;
          
            if(pid_index<=14)   
            {
            listpids[pid_index]=pids;
            pid_index++;
            max_pid++;
            }
            else      //this will overwrite older pids after the amount reaches 15
            {
                pid_index=0;
                listpids[pid_index]=pids;
                pid_index++;
                max_pid=15;

            }
          
            int status=0;

            waitpid(child_pid, &status,0);
        }
    }
    
        // freeing tokens for reuse

    int y;
    for (y=0;y<token_count;y++)
    {
     free(token[y]);
    }
    free( working_root );
     }
   
     
    return 0;

  }
 
