#include "head.h"
int exitpipe[2];
/*信号处理函数，关闭服务器端的socket端口，发送信号让子进程退出*/
void sig(int signum)
{
	write(exitpipe[1],"exit",4);
}
int main(int argc,char **argv)
{
	if(argc!=4)
	{
		printf("IP PORT CHILDNUM\n");
		return -1;
	}
	int num=atoi(argv[3]);
	pchild p;
	p=(pchild)calloc(num,sizeof(child));
	makechild(p,num);  //创建形参中输入个数的子进程，并且通过全双工管道连接
	pipe(exitpipe);
	signal(SIGINT,sig);
	char ipdizhi[128]={0};
	int sfd,new_fd,ret,i,j,logfd;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd==-1)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ser.sin_port=htons(atoi(argv[2]));
	ret = bind(sfd,(struct sockaddr*)&ser,sizeof(ser));
	if(ret==-1)
	{
		perror("bind");
		return -1;
	}
	int epfd = epoll_create(1);
	struct epoll_event event,*eve;
	eve=(struct epoll_event*)calloc(num+2,sizeof(struct epoll_event));
	memset(&event,0,sizeof(event));
	event.events = EPOLLIN;
	event.data.fd=sfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	event.events = EPOLLIN;
	event.data.fd=exitpipe[0];
	epoll_ctl(epfd,EPOLL_CTL_ADD,exitpipe[0],&event);
	for(j=0;j<num;j++)
	{
		memset(&event,0,sizeof(event));
		event.events = EPOLLIN;
		event.data.fd=p[j].tsfd;
		epoll_ctl(epfd,EPOLL_CTL_ADD,p[j].tsfd,&event);
	}
	listen(sfd,num);
	int ret1;
	char buf[128]={0};
	struct sockaddr_in khj;
	memset(&khj,0,sizeof(khj));
	int l= sizeof(khj);
	while(1)
	{
		ret1=epoll_wait(epfd,eve,num+2,-1);
		for(i=0;i<ret1;i++)
		{
			if(eve[i].data.fd==sfd)
			{
				new_fd = accept(sfd,(struct sockaddr*)&khj,&l);  //接受客户端的连接请求
				printf("终于有一个客户端连接了，IP地址为%s,端口为%d\n",inet_ntoa(khj.sin_addr),ntohs(khj.sin_port));
				for(j=0;j<num;j++)
				{
					if(p[j].busy==0)
					{
						send_fd(p[j].tsfd,new_fd);  //将与客户端进行通信的操作符给空闲的子进程
						printf("开始会话！\n");
						p[j].busy=1;
						break;
					}
					//暂时先不考虑客户端超过进程总数目的情况
				}
				close(new_fd);
			}
			for(j=0;j<num;j++)
			{
				if(eve[i].data.fd==p[j].tsfd)
				{
					ret=read(p[j].tsfd,buf,sizeof(buf));
					if(ret>0)
					{
						printf("结束会话！\n");
						p[j].busy=0;
						break;
					}
				}
			}
			if(eve[i].data.fd==exitpipe[0])
			{
				read(exitpipe[0],buf,sizeof(buf));
				printf("收到退出信号.\n");
				event.events = EPOLLIN;
				event.data.fd=sfd;
				epoll_ctl(epfd,EPOLL_CTL_DEL,sfd,&event);
				int left=0;
				for(j=0;j<num;j++)
				{
					if(!p[j].busy)//获取空闲的子进程数目
					{
						left++;
					}
				}
				while(1)
				{
					if(left==num)break;
					ret=0;
					ret=epoll_wait(epfd,eve,num+2,-1);
					for(j=0;j<ret;j++)
					{
						read(eve[j].data.fd,buf,sizeof(buf));
					}
					left+=ret;
				}
				for(j=0;j<num;j++)
				{
					kill(p[j].pid,SIGKILL);
				}
				for(j=0;j<num;j++)
				{
					wait(NULL);
				}
				printf("退出完毕.\n");
				return 0;
			}
		}
	}
}
