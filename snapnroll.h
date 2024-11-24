#include "user.h"
#include "types.h"
#include "fcntl.h"
#include "fs.h"
#include "stat.h"

#define SNAP         "snapshots"
#define WORKING      "working"
#define LOG          "log.txt"
#define LOGPATH      "snapshots/log.txt"
#define SAVE         "snapshots/working"
#define MAX_VERSIONS  3
       

//Snapshot and RollBack Function
int sshot( );
int rback(int );

//Logging Functions
int read_log(char *,char ** );
void write_log(char *,char *);
int create_log( );
int get_log( );

//Helper Functions
int isAvailable(char * ,char* );
char* strcat(char * ,char * ); 
void printHelp();

//Functions for removing whole Directory
int remove_recursive(char * );
int tryremove(int ,char * argv[] );


//Functions to copy one Directory  to another
void copy(char * , char * );
void copy_files_with_extension(char * , int , char * );
void copy_recursive(char * , char * );
int trycopy(int ,char *argv[] );