#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

//参数不足，第一个实际参数不是数字，则输出错误
  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

//将第一个参数转换为整数，作为参数传给trace，如果trace的返回值小于零就报错。
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

//用nargv字符串数组保存数字之后的参数 就是要追踪的系统调用的名字
  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i-2] = argv[i];
  }

//int execv(const char *path, char *const argv[]);
//这里是执行程序narv[0]，即根据narv[0]代表的命令名执行程序
//nargv是新启动程序的命令行参数，相当于main参数里的argv
  exec(nargv[0], nargv);
  exit(0);
}
