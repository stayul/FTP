#include "head.h"

void fun1(int signum)
{
	printf("我收到了这个信号\n");
}
int main(int argc,char **argv)
{
    if(argc!=3)
    {
            printf("IP PORT USERID\n");
            return -1;
    }
	signal(SIGSEGV,fun1);
	struct stat ddxc;
	DIR *dir;
	struct dirent *pdir;
	long size;
    int ret,sfd,i,j;
	long sizeoffile;
    sfd = socket(AF_INET,SOCK_STREAM,0);  //生成socket描述符
    if(sfd==-1)
    {
            perror("socket");
            return -1;
    }
    struct sockaddr_in khj;  //定义新的结构体包含指定的端口和地址
    memset(&khj,0,sizeof(khj));
    char buf[128]={0},*p;
    khj.sin_family= AF_INET;
    khj.sin_port = htons(atoi(argv[2]));
    khj.sin_addr.s_addr = inet_addr(argv[1]);
    ret = connect(sfd,(struct sockaddr*)&khj,sizeof(khj));  //请求连接服务器
	if(ret==-1)
	{
		perror("connect");
		return -1;
	}
	passwdfunc(sfd);//密码登录或注册
	while(1)
	{
		memset(buf,0,sizeof(buf));
		ret=read(0,buf,sizeof(buf));
		if(ret>0)
		{
			if(strncmp(buf,"cd ",3)==0&&strcmp(buf,"cd \n")!=0)//当输入为cd...
			{
				i=1;
				send(sfd,(char*)&i,sizeof(int),0);
				p=buf+3;
				send(sfd,p,strlen(p)-1,0);
				memset(buf,0,sizeof(buf));
				recv(sfd,buf,sizeof(buf),0);
				system("clear");
				printf("%s\n",buf);
			}
			else if(strcmp(buf,"ls\n")==0)//当输入为ls...
			{
				i=2;
				send(sfd,(char*)&i,sizeof(int),0);
				system("clear");
				while(recv(sfd,&j,sizeof(int),0),j>0)
				{
					memset(buf,0,sizeof(buf));
					recv(sfd,buf,j,0);
					recv(sfd,&sizeoffile,8,0);
					recv(sfd,&j,sizeof(int),0);
					if(j==0)
					{
						printf("\033[34m%-20s\033[0m\t%-ld\n",buf,sizeoffile);
					}
					else if(j==1)
					{
						 printf("%-20s\t%-ld\n",buf,sizeoffile);
					}
					else if(j==2)
					{
						printf("\033[32m%-20s\033[0m\t%-ld\n",buf,sizeoffile);
					}
				}
			}
			else if(strncmp(buf,"puts ",5)==0&&strcmp(buf,"puts \n")!=0)//当输入为puts...
			{
				system("clear");
				i=3;
				send(sfd,(char*)&i,sizeof(int),0);
				p=buf+5;
				i=strlen(p);
				p[i-1]='\0';
				send_file(sfd,p);
			}
			else if(strncmp(buf,"gets ",5)==0&&strcmp(buf,"gets \n")!=0)//当输入为gets...
			{
				system("clear");
				i=4;
				send(sfd,(char*)&i,sizeof(int),0);
				p=buf+5;
				i=strlen(p)-1;
				send(sfd,(char*)&i,sizeof(int),0);
				memset(&ddxc,0,sizeof(ddxc));
				dir=opendir("./");
				while((pdir=readdir(dir))!=NULL)
				{
					if(strcmp(p,pdir->d_name)==0)
					{
						break;
					}
				}
				send(sfd,p,strlen(p)-1,0);
				if(pdir==NULL)
				{
					size=0;
				}
				else
				{
					stat(pdir->d_name,&ddxc);
					size=ddxc.st_size;
				}
				send(sfd,(char*)&size,sizeof(size),0);
				recv_file(sfd,size);
			}
			else if(strncmp(buf,"remove ",7)==0&&strcmp(buf,"remove \n")!=0)//当输入为remove...
			{
				system("clear");
				i=5;
				send(sfd,(char*)&i,sizeof(int),0);
				p=buf+7;

				i=strlen(p);
				send(sfd,p,strlen(p)-1,0);
				recv(sfd,&i,sizeof(int),0);
				if(i==-1)
				{
					printf("服务器没有此文件,请重新输入\n");
				}
				else
				{
					printf("文件删除成功\n");
				}
			}
			else if(strcmp(buf,"pwd\n")==0)//当输入为pwd...
			{
				system("clear");
				i=6;
				send(sfd,(char*)&i,sizeof(int),0);
				memset(buf,0,sizeof(buf));
				recv(sfd,buf,sizeof(buf),0);
				printf("%s\n",buf);
			}
			else if(strcmp(buf,"exit\n")==0||strcmp(buf,"quit\n")==0)
			{
				return 0;//退出
			}
			else
			{
				system("clear");
				printf("输入错误指令，请重新输入\n");
			}
		}
	}
	close(sfd);
	return 0;
}
