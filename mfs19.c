/*Name: Kashif Hussain
ID: 1001409065
Name: Rasika Hedaoo
ID:1001770527  */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdint.h>

#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 128
#define MAX_FILE_SIZE 10240000
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define min(a, b) (a < b ? a : b) 

FILE *fd;

char * fsFile=NULL;

uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];

struct Directory_Entry
{

 uint8_t valid;
 char filename[255];
 uint32_t inode;
};

struct Inode
{
 uint8_t valid;
 uint8_t attributes; //1=hidden, 2=readonly, 3=hidden+readonly flags for attributes
 uint32_t size;
 //time_t time;
 char timeT[50];
 uint32_t blocks[1250];

};


// 128  * ( 1+1+4+1250*4) = 640768 approx 79 blocks

struct Directory_Entry * dir;
struct Inode * inodes;
uint8_t * freeBlockList;
uint8_t * freeInodeList;



void initializeBlockList()
{
 int i;
 for(i=0;i<NUM_BLOCKS;i++)
  {
    freeBlockList[i]=1; //set all blocks to 1, meaning the blocks are free to be used.
  }
}

void initializeInodeList()
{
 int i;
 for(i=0;i<NUM_FILES;i++)
 {
   freeInodeList[i]=1; //setting all inodes to 1, meaning the inodes are available to be used.
 }
}
void initializeDirectory()
{ 
  int i;
  for(i=0;i<NUM_FILES;i++)
  {
    dir[i].valid= 0; //means dir is not being used, it is free.
    dir[i].inode=-1; //means the inode is available to be used.
    memset(dir[i].filename,'\0',255); // clears array of filename to be used.
      
  }
}

void initializeInodes()
{
  int i;
  for(i=0;i<NUM_FILES;i++) //ensuring all inodes are free at the beginning of runtime
  {
    inodes[i].valid=0; //0 means not being used
    inodes[i].size=0; 
    inodes[i].attributes=0;

  int j;
   for (j=0;j<1250;j++)
   {
    inodes[i].blocks[j]=-1;
   }
  }
}

int df() //returns free space of the program
{
 int i; 
 int free_space=0;
 for(i=0;i<NUM_BLOCKS;i++)
 {
   if(freeBlockList[i]==1)
   {
    free_space=free_space+BLOCK_SIZE;
   }
 }

 return free_space;
}

int findDirectoryIndex(char * filename) //to search for file in the filesystem
{
 int i;
 printf("filename: %s (length %d)\n", filename, strlen(filename));
 for(i=0;i<NUM_FILES;i++)
 {

   if(dir[i].valid == 1 && strncmp(filename, dir[i].filename, strlen(filename))==0)
   {
    return i; //returns found directory index for the given filename
   }
 }

 for(i=0;i<NUM_FILES;i++) //else it allocate free directory
 {
   if(dir[i].valid==0)
   {
    return i;
   }
 }
 return -1;
}

int findFreeInode()
{
 int i;
 int ret=-1;
 for(i=10;i<NUM_FILES;i++)
 {
   if(inodes[i].valid==0) //if the inode is free
   {
    inodes[i].valid=1; //mark it in use, and return the inode index
    return i;
   }
 }

 return ret;
}

int findFreeBlock()
{
 int i;
 int ret=-1;
 for(i=10;i<NUM_BLOCKS;i++)
 {
   if(freeBlockList[i]==1) //if a block is free
   {
    freeBlockList[i]=0; //mark it in use, and return the index
    return i;
   }
 }

 return ret;
}

int put(char * filename)
{
 struct stat buf;
 int ret;
 ret=stat(filename,&buf);
 if(ret==-1)
 {
   printf("File not found\n");
   return -1;
 }
 int size= buf.st_size;
 if(size>MAX_FILE_SIZE)
 {
  printf("File size too big\n");
  return -1;
 }

 if(size>df())
 {
   printf("put error: Not enough disk space.\n");
   return -1;
 }

 if(strlen(filename) > 255)
 {
  printf("put error: File name too long.");
  return -1;
 }


 int directoryIndex=findDirectoryIndex(filename); //gets a valid directory index
 printf("put dir[%d]\n", directoryIndex);
 int inodeIndex=findFreeInode();

 if (inodeIndex == -1)
  {
     printf("Error\n");
     return -1;
  }

 struct Directory_Entry* dirent = &(dir[directoryIndex]);
 struct Inode * inode= &(inodes[inodeIndex]);
 strcpy(dirent->filename, filename); //copies filename to struct pointer to be saved into the filesystem
 printf("Copied filename as %s\n", dirent->filename);
 dirent -> valid = 1; //sets directory as being used
 dirent -> inode = inodeIndex; //gets the appropriate inode index for the directory
 


    FILE *ifp = fopen ( filename, "rb" ); //opens and reads given file
    printf("Reading %d bytes from %s\n", (int) buf.st_size, filename );
 
   
    int copy_size   = buf.st_size;
    int offset      = 0;               
    int block_index = 0;
    while( copy_size > 0 ) //copies data from file to filesystem one block at a time
    {
      int freeBlockIndex = findFreeBlock();
      inode->blocks[block_index] = freeBlockIndex;
      fseek( ifp, offset, SEEK_SET );
      int bytes  = fread(&blocks[freeBlockIndex], BLOCK_SIZE, 1, ifp);
      if(bytes == 0 && !feof(ifp ))
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }
      clearerr(ifp);
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;
    }
    inode->size=(int)buf.st_size;
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_mtime)); //obtaining last modified time for the file
    strcpy(inode->timeT, time);
    fclose( ifp );
 }

int get (char *filename, char *newfilename)
{
  int dir_index = findDirectoryIndex(filename);
  printf("dir: %d\n", dir_index);
  if(dir_index == -1)
  {
    printf("get error:File not found\n");
    return -1;
  }

  struct Directory_Entry* dirent = &dir[dir_index]; //obtains directory index for the file
  int inode_index = dirent->inode;
  printf("inode #: %d\n", inode_index);
  if(inode_index==-1)
  {
   return -1;
  }
  struct Inode* inode = &(inodes[inode_index]);
  if(inode->attributes==0 || inode->attributes==1) //if file is read-only or not hidden
  {

  if(!newfilename)
  {
    newfilename = filename; //if user did not provide filename, old filename is used the new filename
  }
  
  FILE *newfile;
  newfile = fopen(newfilename, "w");

  if(newfile == NULL)
  {
    printf("Could not open output file: %s\n", newfilename);
    perror("Opening output file returned");
    return -1;
  }

 // Initialize our offsets and pointers just we did above when reading from the file.
  int block_index = 0;
  int copy_size   = inode->size;
  int offset      = 0;

  printf("Writing %d bytes to %s\n", copy_size, newfilename );

  while( copy_size > 0 )
  {  
    int num_bytes = min(BLOCK_SIZE, copy_size); //checking if a block size might exceed the data needed to transfer to file
    int filledBlockIndex = inode->blocks[block_index];
    fwrite(&blocks[filledBlockIndex], num_bytes, 1, newfile); //writes from filled blocks to the new file
    copy_size -= num_bytes;
    offset    += num_bytes;
    block_index++;

    fseek(newfile, offset, SEEK_SET );
  }

  fclose(newfile);
 }
 else
 {
   printf("File is read Only\n");
 }
}



void createfs(char * filename)
{
  fd=fopen(filename,"w");
  dir=(struct Directory_Entry*)&blocks[0]; 
  freeInodeList=(uint8_t *)&blocks[7];
  freeBlockList=(uint8_t *)&blocks[8];
  inodes=(struct Inode *)&blocks[9]; // every inode requires 5k bytes.
   
  initializeDirectory(); //frees blocks for a new filesystem to be created and used by the user
  initializeBlockList();
  initializeInodeList();
  fsFile=filename;
  fwrite(&blocks[0],NUM_BLOCKS,BLOCK_SIZE,fd);
  fclose(fd);
}

void closefs(char * filename)
{

  fd=fopen(filename,"wb");
  fwrite(&blocks[0],NUM_BLOCKS,BLOCK_SIZE,fd); //writes blocks to file to save
 
  dir=(struct Directory_Entry*)&blocks[0];
  freeInodeList=(uint8_t *)&blocks[7];
  freeBlockList=(uint8_t *)&blocks[8];
  inodes=(struct Inode *)&blocks[9]; // every inode requires 5k bytes.
   
  initializeDirectory();
  initializeBlockList();
  initializeInodeList();
  

  fclose(fd);
}

void open(char * filename)
{
 fsFile=filename;
 fd=fopen(filename,"rb");
 if(fd==NULL)
 {
  printf("File not found\n");
 }
 else
 {
 fread(&blocks[0],NUM_BLOCKS,BLOCK_SIZE,fd); //reads file system image into allocated blocks
 fclose(fd);
 }
}

void list()
{

 int i;
 int count=0;
 for(i=0;i<NUM_FILES;i++)
 {
   if(dir[i].valid==1)
   {
     int inode_idx=dir[i].inode;
     if(inodes[inode_idx].attributes!=1 && inodes[inode_idx].attributes!=3) //list if file is not hidden
     {
     printf("Filename: [%s] Inode_Size: [%hhu] Time in seconds: [%s]\n",dir[i].filename,inodes[inode_idx].size,inodes[inode_idx].timeT);
     count++;
     }
   }
 }
 if(count==0)
 {
   printf("List:No files found\n");
 }
}

void hiddenList()
{

 int i;
 int count;
 for(i=0;i<NUM_FILES;i++)
 {
   if(dir[i].valid==1)
   {
     int inode_idx=dir[i].inode;
     if(inodes[inode_idx].attributes!=0 && inodes[inode_idx].attributes!=2) //list only if file is hidden
     {
     printf("Filename: [%s] Inode_Size: [%hhu] Time in seconds: [%s]\n",dir[i].filename,inodes[inode_idx].size,inodes[inode_idx].timeT);
     count++;
     }
   }
 }
 if(count==0)
 {
   printf("Hidden List:No files found\n");
 }
}



int del(char* filename)
{
  int dir_index = findDirectoryIndex(filename);
  if(dir_index == -1)
  {
    return -1;
  }

  struct Directory_Entry* dirent = &dir[dir_index];
  int inode_index = dirent->inode; 
  printf("inode #: %d\n", inode_index);
  if(inode_index==-1)
  {
   return -1;
  }
    struct Inode* inode = &(inodes[inode_index]);

    dir[dir_index].valid= 0; //means directory is not being used.
    dir[dir_index].inode=-1;
    memset(dir[dir_index].filename,'\0',255);
   
    
    inodes[inode_index].valid=0; //means inode is not being used.
    inodes[inode_index].size=0;
    inodes[inode_index].attributes=0;

   int j;
   for (j=0;j<1250;j++)
   {
    int blockIndex=inodes[inode_index].blocks[j];
    freeBlockList[blockIndex]=1; //allocates blocks to free block list to render them free for next use.
   }
}

int attrib(char * filename, char * attribute)
{
   int dir_index = findDirectoryIndex(filename);
   if(dir_index == -1)
   {
     printf("attrib error: File not found\n");
     return -1;
   }

   struct Directory_Entry* dirent = &dir[dir_index];
   int inode_index = dirent->inode; 
   struct Inode* inode = &(inodes[inode_index]);
   if(strcmp(attribute,"+h")==0)
   {
     if(inode->attributes==2)
     {
     inode->attributes=3;  
     }
     else
     {
       inode->attributes=1; 
     }
   }
   else if(strcmp(attribute,"-h")==0)
   {
    if(inode->attributes==3)
    {
       inode->attributes=2;
    }
    else
    {
      inode->attributes=0;
    }

   }
   else if(strcmp(attribute,"+r")==0)
   {
    if(inode->attributes==1)
     {
     inode->attributes=3;  
     }
     else
     {
       inode->attributes=2; 
     }
   
   }
   else if(strcmp(attribute,"-r")==0)
   {
     if(inode->attributes==3)
    {
       inode->attributes=1;
    }
    else
    {
      inode->attributes=0;
    }
   }
  return 0;
}

int main()
{
  dir=(struct Directory_Entry*)&blocks[0];
  freeInodeList=(uint8_t *)&blocks[7];
  freeBlockList=(uint8_t *)&blocks[8];
  inodes=(struct Inode *)&blocks[9]; // every inode requires 5k bytes.
   
  initializeDirectory();
  initializeBlockList();
  initializeInodeList();

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

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

   //if(token_count < 2)
   //{

     //continue;
   //}
   if(token[0]==NULL)
   {
    continue;
   }

   if(strcmp(token[0],"put")==0)
   {
     char *filename=token[1];

     put(filename);
   }
   else if(strcmp(token[0],"get")==0)
   {
        char *filename=token[1];
	char *newfilename = NULL;
	if(token_count > 2)
	{
		newfilename = token[2];
	}

     get(filename, newfilename);
   }

    else if(strcmp(token[0],"df")==0)
    {
      int d=df();
      printf("Remaining Space is %d\n",d);
    }
    else if(strcmp(token[0],"del")==0)
    {
        char *filename=token[1];
        int dir_index = findDirectoryIndex(filename);
   
			if(dir_index == -1)
			{
			  printf("File not found\n");
		          return -1;
			}

   			struct Directory_Entry* dirent = &dir[dir_index];
   			int inode_index = dirent -> inode; 
  		        struct Inode* inode = &(inodes[inode_index]);
                            
			if(inode -> attributes == 2 || inode -> attributes == 3)
			{
				printf("del: That file is marked read-only.\n");
			}
	
		    else{
       				del(filename);
			}

   
    }
    

    else if(strcmp(token[0],"list")==0)
    {
       char * t  = token[1];
      if(t==NULL)
      {
      list();
       }
       else if(strcmp(t,"-h")==0)
       {
         hiddenList();
       }
     else
      {
        printf("Wrong Command\n");
       }
    }
  
      
    
    else if(strcmp(token[0],"createfs")==0)
    {
      char* filename= token[1];
      if(filename==NULL)
      {
        printf("createfs: File not found\n");
      }
      else
     {
      createfs(filename);
     }
    }
    else if(strcmp(token[0],"open")==0)
    {
      char * filename=token[1];
      if(filename==NULL)
      {
        printf("open: File not found\n");
      }
      else
     {
      open(filename);
     }
    }
    else if(strcmp(token[0],"attrib")==0)
    {
      char *filename=token[2];
      char * attribute=token[1];
      attrib(filename,attribute);
    }
    else if(strcmp(token[0],"close")==0)
    {
     closefs(fsFile);
    }
    else if(strcmp(token[0],"quit")==0)
    {
      break;
    }
    else if(strcmp(token[0],"exit")==0)
    {
     break;
    }
    else
    {
      printf("Type Valid Command\n");
      continue;
    }
    
    free( working_root );
    

  }
  return 0;
}



