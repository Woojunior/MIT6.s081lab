#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXSIZE 16
#define MAXARG 32
int main(int argc, char *argv[])
{   
    //sleep(10);
    // 1.获取前一个指令的标准输出（参考教材，第13页）作为xargs的标准输入，文件描述符0代表标准输入
    char buf[MAXSIZE]; // 字符指针的数组，也可以理解为字符串数组
    read(0, buf, sizeof(buf));

    // 2.将xargs后面的参数进行储存
    char *xargs_argv[MAXARG]; // 用于储存 xargs后面的参数，xargs_argv[0]是命令符
    int xargs_argc = 0;
    for (int i = 1; i < argc; i++)
    {
        xargs_argv[xargs_argc] = argv[i];
        xargs_argc++;
    }

    // 将buf中前一个指令的输出传递给xargs_argv数组
    char *p = buf; // p指向buf数组
    for (int i = 0; i < MAXSIZE; i++)
    {
        if (buf[i] == '\n')
        {
            // 3.开始执行
            int fid = fork();
            if (fid == 0)
            {
                // 子进程
                buf[i] = 0;                 // 将换行符这里改为0即'\0'
                xargs_argv[xargs_argc] = p; // 此时在参数后面加上前面一个命令的标准输出的一行
                xargs_argc++;               // 移动到下一个位置
                xargs_argv[xargs_argc] = 0; // 下一个位置置空，表示字符串的结束

                exec(xargs_argv[0], xargs_argv); // 执行命令
                exit(0);
            }
            else
            {
                // 父进程
                p = &buf[i + 1]; // 指向下一个字符串，即下一行的开始位置
                wait(0);         // 等待子进程
            }
        }
    }

    exit(0); // 主进程退出
}