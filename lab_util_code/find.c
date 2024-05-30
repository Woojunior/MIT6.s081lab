#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    //从路径中提取出文件名
    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p),0, DIRSIZ - strlen(p));//这里和ls不一样，文件名的大小不会用' '填充。
    return buf;
}

void find(char *path, char *filename)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    //按路径打开目录。

    // cann't open
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    //  #define T_DIR 1    // Directory
    //  #define T_FILE 2    // File
    //  #define T_DEVICE 3 // Device

    // state of file
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    //如果输入的路径直接到所需查找的文件名，就直接打印路径
    case T_FILE:
        if (strcmp(filename, fmtname(path)) == 0)
        {
            printf("%s\n", path);
        }
        break;

    //如果路径是到的是文件夹，就在当前文件目录下查找
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);//将当前路径拷贝到buf
        p = buf + strlen(buf);//p是指向buf末尾即'\0'的，字符指针
        *p++ = '/';//在当前字符串，即路径后面加上字符/

        // 遍历当前目录
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        { // 无效的目录，或者是当前目录，或者是当前目录的父目录 就继续读取下一个文件
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            memmove(p, de.name, DIRSIZ);//将文件名拷贝到p中
            p[DIRSIZ] = 0; // 在目录项后添加空字符'\0'
            if (stat(buf, &st) < 0)
            {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            // 如果此时buf代表的路径仍然是目录文件，则递归寻找，直到找到非目录文件，就执行case T_FILE所对应的，打印目标文件对应的目录
            find(buf, filename); 
        }
        break;
    }
    close(fd);//查找完毕，关闭文件描述符
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Error!expect: find path filename1 filename2 and so on");
        exit(1);
    }

    // 这里循环，是一个路径下可能需要查找多个文件
    for (int i = 2; i < argc; i++)
    {
        find(argv[1], argv[i]);
    }
    exit(0);
}