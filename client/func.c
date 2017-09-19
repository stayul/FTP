#include "head.h"

void get_rand_str(char *salt,int num)
{
	strcat(salt,"$6$");
	char *str="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
	int str_len=strlen(str);
	srand(time(NULL));
	int i;
	for(i=3;i<num;i++)
	{
		salt[i]=str[rand()%str_len];
	}
}
void passwdfunc(int sfd)
{
	int xuhao,i=0;
	char username[20];
	char salt[20];
	char sp[100];
	train t;
lable:
	xuhao=0;
	system("clear");
	printf("输入序号1进行登录,输入序号2进行注册,请输入:\n");
	scanf("%d",&xuhao);
	memset(username,0,sizeof(username));
	memset(salt,0,sizeof(salt));
	memset(sp,0,sizeof(sp));
	if(xuhao==1)//登录
	{
		i=1;
		send(sfd,&i,sizeof(i),0);//告诉服务器要进行登录
		printf("请输入登录用户名:");
		fflush(stdout);
		memset(&t,0,sizeof(t));
		scanf("%s",username);
		char *passwd=getpass("请输入密码:");
		t.len=strlen(username);
		strcpy(t.buf,username);
		send(sfd,&t,4+t.len,0);
		recv(sfd,&i,sizeof(i),0);
		recv(sfd,salt,i,0);
		recv(sfd,&i,sizeof(i),0);
		recv(sfd,sp,i,0);
		if(strcmp(sp,crypt(passwd,salt))==0)
		{
			printf("登录成功\n");
			i=0;
			send(sfd,&i,sizeof(int),0);
		}
		else
		{
			getpass("用户名不存在或密码错误，按任意键返回上一级:");
			i=1;
			send(sfd,&i,sizeof(int),0);
			goto lable;
		}
	}
	else if(xuhao==2)//注册
	{
		i=2;
		send(sfd,&i,sizeof(i),0);//告诉服务器要进行注册
		printf("请输入用户名:");
		fflush(stdout);
		scanf("%s",username);
		get_rand_str(salt,5);
		char *passwd=getpass("请输入密码:");
		strcpy(sp,crypt(passwd,salt));
		memset(&t,0,sizeof(t));
		t.len=strlen(username);
		strcpy(t.buf,username);
		send(sfd,&t,4+t.len,0);
		memset(&t,0,sizeof(t));
		t.len=strlen(salt);
		strcpy(t.buf,salt);
		send(sfd,&t,4+t.len,0);
		memset(&t,0,sizeof(t));
		t.len=strlen(sp);
		strcpy(t.buf,sp);
		send(sfd,&t,4+t.len,0);
		recv(sfd,&i,sizeof(i),0);
		if(i!=-1)
		{
			getpass("注册成功，按任意键返回上一级:");
			goto lable;
		}
		else
		{
			getpass("注册失败，按任意键返回上一级:");
			goto lable;
		}
	}
	else
	{
		printf("输入错误,请重新输入:\n");
		goto lable;
	}
}
//int sql_insert(char *username,char *salt,char *sp)
//{
//	MYSQL *conn;
//	char *server="localhost";
//	char *user="root";
//	char *password="0809";
//	char *database="ftp";
//	char query[200]={0};
//	int t;
//	conn=mysql_init(NULL);
//	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
//	{
//		printf("error connecting to database:%s\n",mysql_error(conn));
//	}
//	sprintf(query,"%s%s%s%s%s%s%s","insert into userinfor(USERNAME,SALT,SP) values('",username,"','",salt,"','",sp,"')");
//	t=mysql_query(conn,query);
//	if(t)
//	{
//		printf("error making query:%s\n",mysql_error(conn));
//		return -1;
//	}
//	mysql_close(conn);
//	return 0;
//}
void error_quit(char*msg)//perror
{
	perror(msg);
	exit(-1);
}
void get_salt(char *salt,char *passwd)//得到salt
{
	int i,j;
	for(i=0,j=0;passwd[i]&&j!=3;i++)
	{
		if(passwd[i]=='$')
		{
			j++;
		}
	}
	strncpy(salt,passwd,i-1);
}
void send_file(int new_fd,char *filename)  //发送文件数据
{
	int fd=open(filename,O_RDONLY);
	if(fd==-1)
	{
		printf("当前位置没有此文件,请重新输入\n");
		send(new_fd,(char*)&fd,sizeof(int),0);
		return;
	}
	else
	{
		send(new_fd,"null",4,0);
	}
	train t;
	int total;
	memset(&t,0,sizeof(t));
	int ret;
	char *p;
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
	time_t t1,t2;
	time(&t1);
	t2=t1+1;
	long sum=0;
	while(memset(&t,0,sizeof(t)),(t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		p=(char*)&t;
		total=0;
		while(total<t.len)
		{
			ret=send(new_fd,p+total,4+t.len-total,0);
			total+=ret;
			sum+=ret;
			time(&t1);
			if(t1>t2)
			{
				printf("\r已经上传%5.2f%s的数据",(double)sum*100/buf.st_size,"%");
				fflush(stdout);
				t2=t1+1;
			}
		}
	}
	printf("\r已经上传100%s的数据  \n","%");
	printf("上传成功\n");
	t.len=0;
	send(new_fd,&t,4+t.len,0);
}
void recv_file(int sfd,long size)//接收文件数据
{
	int len,total,ret;
	char buf[1000]={0};
	recv(sfd,&len,sizeof(len),0);
	if(len==-1)
	{
		printf("服务器没有此文件,请重新输入\n");
		return;
	}
	recv(sfd,&len,sizeof(len),0);
	recv(sfd,buf,len,0);
	int fd=open(buf,O_WRONLY|O_CREAT,0666);
	lseek(fd,size,SEEK_SET);
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
				time(&t);
				if(t>t1)
				{
					printf("\r已经下载%5.2f%s的数据",(double)(sum+size)*100/i,"%");
					fflush(stdout);
					t1=t+1;
				}
			}
		}
		else
		{
			printf("\r已经下载100%s的数据  \n","%");
			printf("下载成功\n");
			break;
		}
	}
	close(fd);
}

void send_fd(int fdw,int fd)//主进程将文件描述符发送给子进程
{
        int ret;
        struct msghdr msg;
        char buf1[10]="hello";
        char buf2[10]="world";
        struct iovec iov[2];
        iov[0].iov_base=buf1;  //对ivoec结构体进行赋值
        iov[0].iov_len=5;
        iov[1].iov_base=buf2;
        iov[1].iov_len=5;
        msg.msg_iov = iov;
        msg.msg_iovlen=2;
        struct cmsghdr *cmsg;
        int len = CMSG_LEN(sizeof(int));
        cmsg=(struct cmsghdr*)calloc(1,len);
        cmsg->cmsg_len=len;
        cmsg->cmsg_level=SOL_SOCKET;
        cmsg->cmsg_type=SCM_RIGHTS;
        *(int*)CMSG_DATA(cmsg)=fd;
        msg.msg_control=cmsg;
        msg.msg_controllen=len;
        ret = sendmsg(fdw,&msg,0);
        if(ret == -1)
        {
                perror("sendmsg");
                return;
        }
}
void recv_fd(int fdr,int* fd)//子进程接受主进程发送过来的文件描述符
{
        int ret;
        struct msghdr msg;
        char buf1[10]="hello";
        char buf2[10]="world";
        struct iovec iov[2];
        iov[0].iov_base=buf1;  //对ivoec结构体进行赋值
        iov[0].iov_len=5;
        iov[1].iov_base=buf2;
        iov[1].iov_len=5;
        msg.msg_iov = iov;
        msg.msg_iovlen=2;
        struct cmsghdr *cmsg;
        int len = CMSG_LEN(sizeof(int));
        cmsg=(struct cmsghdr*)calloc(1,len);
        cmsg->cmsg_len=len;
        cmsg->cmsg_level=SOL_SOCKET;
        cmsg->cmsg_type=SCM_RIGHTS;
        msg.msg_control=cmsg;
        msg.msg_controllen=len;
        ret = recvmsg(fdr,&msg,0);
        if(ret == -1)
        {
                perror("recvmsg");
                return;
        }
        *fd=*(int*)CMSG_DATA(cmsg);
}

