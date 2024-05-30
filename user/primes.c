#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define RD 0
#define WR 1
const int INT_L = sizeof(int);

void get_prime(int *p1);

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        printf("Error!");
        return -1;
    }

    // 创建一个管道接收1-35
    int p1[2];
    pipe(p1);
    for (int i = 2; i <= 35; i++)
    {
        // 将1-35写入到管道中
        write(p1[WR], &i, INT_L);
    }

    if (fork() == 0)
    {
        // 创建子进程，筛分质数
        get_prime(p1);
    }

    close(p1[WR]);
    close(p1[RD]);
    wait(0); // 等待子进程结束

    exit(0);
}

void get_prime(int *p1)
{
    // 不需要向p1写入了，只需读取
    close(p1[WR]);

    // 创建子进程的管道
    int p2[2];
    pipe(p2);

    // 读取左邻近，写入右邻近

    // 如果有数字
    int prime_num = 0;
    if (read(p1[RD], &prime_num, INT_L) > 0)
    {
       //载入管道中第一个读取的数
        int tmp; // 过滤掉非prime数,(管道中的所有数都是按照从小到大的)
        while (read(p1[RD], &tmp, INT_L) > 0)
        {
            if (tmp % prime_num != 0)
            {
                // prime数，写入子管道
                write(p2[WR], &tmp, INT_L);
            }
        }
        printf("prime %d\n", prime_num);

        if (fork() == 0)
        {
            // 子进程继续调用函数,(递归调用)
            get_prime(p2);
        }

        // 执行完毕，关闭读写
        close(p1[RD]);
        close(p2[WR]);
        close(p2[RD]);
        wait(0); // 等待子进程
        exit(0);
    }
    else
    {
        // 没有数字可以读取了,对应那个图，就是35的下一个进程
        close(p2[RD]);
        close(p2[WR]);
        exit(0);
    }
}