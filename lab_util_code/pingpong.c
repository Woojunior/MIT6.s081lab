#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int p0[2], p1[2];
    char buf = '0';

    int a1 = pipe(p0); // 父发送到子
    int a2 = pipe(p1); // 子发送到父
    if (a1 == -1 || a2 == -1)
    {
        return -1;
    }

    if (fork() == 0)
    { // 子进程
        int c = getpid();
        close(p1[0]);
        close(p0[1]);
        int r = read(p0[0], &buf, 1); // 2 // p0 读端，放到 buf 里一个字节
        if (r == -1)
        {
            exit(-1);
        }
        printf("%d: received ping\n", c);
        int w = write(p1[1], &buf, 1); // 3 // p1 写端，写给 p1 写端
        if (w == -1)
        {
            exit(-1);
        }
        close(p1[1]);
        close(p0[0]);
    }
    else
    {
        int f = getpid();
        close(p0[0]);
        close(p1[1]);
        int w = write(p0[1], &buf, 1); // 1
        if (w == -1)
        {
            exit(-1);
        }
        int r = read(p1[0], &buf, 1); // 4
        if (r == -1)
        {
            exit(-1);
        }
        printf("%d: received pong\n", f);

        close(p0[1]);
        close(p1[0]);
    }

    exit(0);
}