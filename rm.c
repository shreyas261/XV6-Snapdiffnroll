#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int remove(char * arg);

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf(2, "Usage: rm dest\n");
    exit();
  }
  if(remove(argv[1]) < 0){
    printf(1,"Tried but failed\n");
  }

  exit();
}

int
remove(char * arg)
{
    if(unlink(arg) < 0){
      return -1;
    }
    return 0;
}