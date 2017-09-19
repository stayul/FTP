#include<stdio.h>
#include<string.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/wait.h>
#include<dirent.h>
#include<time.h>
#include<sys/mman.h>
#include<mysql/mysql.h>

typedef struct
{
        pid_t pid;
        int tsfd;
        short busy;
}child,*pchild;
typedef struct
{
        int len;
        char buf[1000];
}train,*ptrain;
void ls_func(char *dirname);
void makechild(pchild,int);
void recv_fd(int,int*);
void send_fd(int,int);
void recv_file(int fd,char *caozuo);
void send_file(int,char*,long,int*);
int sql_insert(char*,char*,char*);
void sql_salt(char*,char*);
void sql_sp(char*,char*);
