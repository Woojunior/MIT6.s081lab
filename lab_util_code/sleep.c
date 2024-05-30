#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{ 
  //输入 例如sleep 10的形势才能被执行
  if(argc==2){
    const int time=atoi(argv[1]);
    sleep(time);
   
    exit(0);
  }
  else{
    printf("error! expect sleep <time>\n");
    exit(1);
  }
}
