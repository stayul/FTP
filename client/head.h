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
#include<shadow.h>
#include<errno.h>
#include<mysql/mysql.h>
#define _XOPEN_SOURCE

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
void recv_fd(int fd,int* new_fd);
void send_file(int fd,char *filename);
void recv_file(int fd,long size);
void get_salt(char*,char*);
void error_quit(char*);
char*crypt(char*,char*);
int sql_insert(char*,char*,char*);
void passwdfunc(int);
