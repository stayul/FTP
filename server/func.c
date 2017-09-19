#include "head.h"

void zhucefunc(int new_fd,char *username,int sizeof_un)//注册函数服务器端
{
	int len,ret;
	train t;
	char salt[20];
	char sp[100];
lable:
	recv(new_fd,&len,sizeof(len),0);
	if(len==1)
	{
		memset(username,0,sizeof_un);
		memset(salt,0,sizeof(salt));
		memset(sp,0,sizeof(sp));
		recv(new_fd,&len,sizeof(len),0);
		recv(new_fd,username,len,0);
		sql_salt(username,salt); //根据客户端输入的用户名匹配盐值和密文
		sql_sp(username,sp);
		memset(&t,0,sizeof(t));
		t.len=strlen(salt);
		strcpy(t.buf,salt);
		send(new_fd,&t,4+t.len,0);
		memset(&t,0,sizeof(t));
		t.len=strlen(sp);
		strcpy(t.buf,sp);
		send(new_fd,&t,4+t.len,0);
		recv(new_fd,&ret,sizeof(ret),0);
		if(ret!=0)
		{
			goto lable;
		}
	}
	else if(len==2)
	{
		memset(username,0,sizeof_un);
		memset(salt,0,sizeof(salt));
		memset(sp,0,sizeof(sp));
		recv(new_fd,&len,sizeof(len),0);
		recv(new_fd,username,len,0);
		recv(new_fd,&len,sizeof(len),0);
		recv(new_fd,salt,len,0);
		recv(new_fd,&len,sizeof(len),0);
		recv(new_fd,sp,len,0);
		ret=sql_insert(username,salt,sp);
		send(new_fd,&ret,sizeof(ret),0);
		goto lable;
	}
}
void sql_salt(char *username,char *salt)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char query[100]={0};
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,"localhost","root","0809","ftp",0,NULL,0))
	{
		printf("error connecting to ftp:%s\n",mysql_error(conn));
	}
	sprintf(query,"%s%s%s","select SALT from userinfor where USERNAME='",username,"'");
	t=mysql_query(conn,query);
	if(t)
	{
		printf("error making query:%s\n",mysql_error(conn));
	}
	res=mysql_use_result(conn);
	row=mysql_fetch_row(res);
	strcpy(salt,row[0]);
}
void sql_sp(char *username,char *sp)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char query[100]={0};
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,"localhost","root","0809","ftp",0,NULL,0))
	{
		printf("error connecting to ftp:%s\n",mysql_error(conn));
	}
	sprintf(query,"%s%s%s","select SP from userinfor where USERNAME='",username,"'");
	t=mysql_query(conn,query);
	if(t)
	{
		printf("error making query:%s\n",mysql_error(conn));
	}
	res=mysql_use_result(conn);
	row=mysql_fetch_row(res);
	strcpy(sp,row[0]);
}
int sql_insert(char *username,char *salt,char *sp)
{
	MYSQL *conn;
	char *server="localhost";
	char *user="root";
	char *password="0809";
	char *database="ftp";
	char query[200]={0};
	int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("error connecting to database:%s\n",mysql_error(conn));
	}
	sprintf(query,"%s%s%s%s%s%s%s","insert into userinfor(USERNAME,SALT,SP) values('",username,"','",salt,"','",sp,"')");
	t=mysql_query(conn,query);
	if(t)
	{
		printf("error making query:%s\n",mysql_error(conn));
		return -1;
	}
	mysql_close(conn);
	return 0;
}
void sig123(int signum)
{
	printf("我收到了这个信号\n");
}
void childfunc(int fd)  //子进程的任务
{
	signal(SIGINT,SIG_IGN);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGSEGV,sig123);
	int new_fd,ret,i,ret1,logfd;
	long daxiao;
	mkdir("log",0777);
	time_t t;
	char buf[128];
	char username[20];
	char rizhiname[128];
	char logtime[128];
	char caozuo[128];
	while(1)
	{
		recv_fd(fd,&new_fd);//获取客户端文件操作符
		time(&t);
		zhucefunc(new_fd,username,sizeof(username));//进行密码验证或者注册
		memset(rizhiname,0,sizeof(rizhiname));
		getcwd(rizhiname,sizeof(rizhiname));
		strcat(rizhiname,"/log/");
		strcat(rizhiname,username);
		logfd=open(rizhiname,O_WRONLY|O_CREAT|O_APPEND,0666);//创建并打开日志文件，如果文件已经存在则直接打开
		memset(logtime,0,sizeof(caozuo));
		strcpy(logtime,"登录时间:");
		strcat(logtime,"\t");
		strcat(logtime,ctime(&t));
		write(logfd,logtime,strlen(logtime));
		while(1)
		{
			memset(caozuo,0,sizeof(caozuo));
			memset(buf,0,sizeof(buf));
			ret=recv(new_fd,&i,sizeof(int),0);
			time(&t);
			strcpy(caozuo,"骚操作:");
			strcat(caozuo,"\t");
			if(ret>0)
			{
				if(i==1)//cd...
				{
					recv(new_fd,buf,sizeof(buf),0);
					ret1=chdir(buf);
					strcat(caozuo,"cd ");
					strcat(caozuo,buf);
					if(ret1==-1)
					{
						send(new_fd,"输入路径目录错误,请重新输入.",40,0);
					}
					else
					{
						send(new_fd,"更改目录成功",18,0);
					}
				}
				else if(i==2)//ls...
				{
					strcat(caozuo,"ls");
					train t;
					DIR *dirfd;
					struct stat file_stat;
					dirfd = opendir("./");
					struct dirent *p;
					while((p=readdir(dirfd))!=NULL)
					{
						if(!(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0))
						{
							memset(&t,0,sizeof(t));
							t.len=strlen(p->d_name);
							memcpy(t.buf,p->d_name,t.len);
							send(new_fd,(char*)&t,4+t.len,0);
							memset(&t,0,sizeof(t));
							stat(p->d_name,&file_stat);
							memcpy(t.buf,&file_stat.st_size,8);
							send(new_fd,t.buf,8,0);
							if(p->d_type==4)
							{
								i=0;
								send(new_fd,(char*)&i,4,0);
							}
							else if(p->d_type==8)
							{
								i=1;
								send(new_fd,(char*)&i,4,0);
							}
							else if(p->d_type==12)
							{
								i=2;
								send(new_fd,(char*)&i,4,0);
							}
						}
					}
					t.len=0;
					send(new_fd,(char*)&t,4,0);	
				}
				else if(i==3)//上传文件
				{
					strcat(caozuo,"puts ");
					recv_file(new_fd,caozuo);
				}
				else if(i==4)//下载文件
				{
					memset(buf,0,sizeof(buf));
					recv(new_fd,&i,sizeof(int),0);
					recv(new_fd,buf,i,0);
					recv(new_fd,&daxiao,sizeof(daxiao),0);
					send_file(new_fd,buf,daxiao,&i);
					strcat(caozuo,"gets ");
					strcat(caozuo,buf);
					if(i==-1)
					{
						break;
					}
				}
				else if(i==5)//删除文件
				{
					memset(buf,0,sizeof(buf));
					recv(new_fd,buf,sizeof(buf),0);
					strcat(caozuo,"remove ");
					strcat(caozuo,buf);
					i=remove(buf);
					send(new_fd,(char*)&i,sizeof(int),0);
				}
				else if(i==6)//显示当前路径
				{
					strcat(caozuo,"pwd");
					memset(buf,0,sizeof(buf));
					getcwd(buf,sizeof(buf));
					send(new_fd,buf,sizeof(buf),0);
				}
			}
			else if(ret==0)
			{
				strcpy(caozuo,"退出登录!");
				strcat(caozuo,"\t");
				strcat(caozuo,ctime(&t));
				write(logfd,caozuo,strlen(caozuo));
				break;
			}
			strcat(caozuo,"\t");
			strcat(caozuo,ctime(&t));
			write(logfd,caozuo,strlen(caozuo));
		}
		write(fd,"over",4);
		close(new_fd);
		close(logfd);
	}
}
void makechild(pchild p,int num)//创建子进程
{
	int i,fds[2];
	pid_t pid;
	int ret;
	for(i=0;i<num;i++)
	{
		socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
		pid=fork();
		if(!pid)  //子进程
		{
			close(fds[1]);
			childfunc(fds[0]);
		}
		close(fds[0]);
		p[i].pid=pid;
		p[i].busy=0;
		p[i].tsfd=fds[1];
	}
}
void send_file(int new_fd,char *filename,long daxiao,int *ret1)  //发送文件数据
{
	int fd=open(filename,O_RDONLY);
	if(fd==-1)
	{
		send(new_fd,(char*)&fd,sizeof(int),0);
		return;
	}
	else
	{
		send(new_fd,"null",4,0);
	}
	lseek(fd,daxiao,SEEK_SET);
	train t;
	int total;
	memset(&t,0,sizeof(t));
	int ret;
	char *p;
	p=(char*)&t;
	strcpy(t.buf,filename);
	t.len=strlen(t.buf);
	send(new_fd,&t,4+t.len,0);
	struct stat buf;
	memset(&buf,0,sizeof(buf));
	memset(&t,0,sizeof(t));
	fstat(fd,&buf);
	t.len=sizeof(buf.st_size);
	memcpy(t.buf,(char*)&buf.st_size,sizeof(buf.st_size));
	send(new_fd,&t,4+t.len,0);
	if(buf.st_size>104857600)
	{
		char *p1=(char*)mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
		if(p1==(char*)-1)
		{
			perror("mmap");
			return;
		}
		char *p2=p1;
		p2+=daxiao;
		long leftdata=buf.st_size-daxiao;
		while(leftdata>1000)
		{
			total=0;
			t.len=1000;
			memset(t.buf,0,sizeof(t.buf));
			memcpy(t.buf,p2,t.len);
			p2+=1000;
			while(total<t.len)
			{
				*ret1=send(new_fd,p+total,4+t.len-total,0);
				if(*ret1==-1)
				{
					return;
				}
				total+=*ret1;
			}
			leftdata-=1000;
		}
		total=0;
		t.len=(int)leftdata;
		memset(t.buf,0,sizeof(t.buf));
		memcpy(t.buf,p2,t.len);
		while(total<t.len)
		{
			ret=send(new_fd,p+total,4+t.len-total,0);
			total+=ret;
		}
		munmap(p1,buf.st_size);
	}
	else
	{
		while(memset(&t,0,sizeof(t)),(t.len=read(fd,t.buf,sizeof(t.buf)))>0)
		{
			total=0;
			while(total<t.len)
			{
				*ret1=send(new_fd,p+total,4+t.len-total,0);
				if(*ret1==-1)//当客户端断开时，让子进程进入最开始的阻塞
				{
					return;
				}
				total+=*ret1;
			}
		}
	}
	t.len=0;
	send(new_fd,&t,4+t.len,0);
	close(fd);
}
void recv_file(int sfd,char *caozuo)//接收文件数据
{
	int len,total,ret;
	char buf[1000]={0};
	recv(sfd,&len,sizeof(len),0);
	if(len==-1)
	{
		return;
	}
	recv(sfd,&len,sizeof(len),0);
	recv(sfd,buf,len,0);
	strcat(caozuo,buf);
	int fd=open(buf,O_WRONLY|O_CREAT,0666);
	long i,sum=0;
	recv(sfd,&len,sizeof(len),0);
	recv(sfd,&i,len,0);
	time_t t,t1;
	time(&t);
	t1=t+1;
	while(1)
	{
		total=0;
		recv(sfd,&len,sizeof(len),0);
		if(len>0)
		{
			while(total<len)
			{
				memset(buf,0,sizeof(buf));
				ret=recv(sfd,buf,len-total,0);
				write(fd,buf,ret);
				total+=ret;
				sum+=ret;
			}
		}
		else if(len==0)
		{
			break;
		}
	}
}
void send_fd(int fdw,int fd)
{	
	struct cmsghdr* cmsg;
	struct msghdr msg;
	memset(&msg,0,sizeof(msg));
	//memset(cmsg,0,sizeof(cmsg));
    int len = CMSG_LEN(sizeof(int));
	cmsg=(struct cmsghdr*)calloc(1,len);
	cmsg->cmsg_len=len;
 	cmsg->cmsg_level=SOL_SOCKET;
	cmsg->cmsg_type=SCM_RIGHTS;
	*(int *)CMSG_DATA(cmsg)=fd;
	msg.msg_control=cmsg;
	msg.msg_controllen=len;
	struct iovec iov[1];
	char buf[0];
	iov[0].iov_base=buf;
	iov[0].iov_len=1;
//	memset(iov,0,sizeof(struct iovec));
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	sendmsg(fdw,&msg,0);
}
void recv_fd(int fdr,int* fd)
{
	struct cmsghdr* cmsg;
	struct msghdr msg;
	memset(&msg,0,sizeof(msg));
	//memset(cmsg,0,sizeof(cmsg));
    int len = CMSG_LEN(sizeof(int));
	cmsg=(struct cmsghdr*)calloc(1,len);
	cmsg->cmsg_len=len;
 	cmsg->cmsg_level=SOL_SOCKET;
	cmsg->cmsg_type=SCM_RIGHTS;
//	*(int *)CMSG_DATA(cmsg)=fd;
	msg.msg_control=cmsg;
	msg.msg_controllen=len;
	struct iovec iov[1];
	char buf[0];
	iov[0].iov_base=buf;
	iov[0].iov_len=1;
//	memset(iov,0,sizeof(struct iovec));
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	recvmsg(fdr,&msg,0);
	*fd=*(int *)CMSG_DATA(cmsg);
}
