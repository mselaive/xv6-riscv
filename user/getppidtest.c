#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  int pid  = getpid();
  int ppid = getppid();
  printf("PID: %d\n", pid);
  printf("Parent PID: %d\n", ppid);
  exit(0);
}
