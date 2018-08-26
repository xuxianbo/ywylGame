#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <setjmp.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#define HOST "47.52.202.22"
#define PORT 16888
#define BOT_MAX 1

int daemon_init()
{
    struct sigaction act;
    int i, maxfd;
    if(fork()!=0)
        exit(0);
    if(setsid()<0)
        return -1;
    act.sa_handler=SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGHUP, &act, 0);
    //signal(SIGHUP,SIG_IGN);
    if(fork()!=0)
        exit(0);
    chdir("/");
    umask(0);
    maxfd=sysconf(_SC_OPEN_MAX);
    for(i=0;i<maxfd;i++)
        close(i);
    open("/dev/null",O_RDWR);
    dup(0);
    dup(1);
    dup(2);
    return 0;
}

int writeFile(char *path,char *buf)
{
	if (*buf == '\0') return -1;
        FILE *file;
        file = fopen(path,"a");
        if (file == NULL)
        {
                return -1;
        }

        fwrite(buf,strlen(buf),1,file);
        fwrite("\n",1,1,file);

        if (file)
        fclose(file);
        return 0;
}

void send_long64_2buf(char *buf,long long int n)
{
	memcpy(buf,(char *)&n,sizeof(long long int));
}

void send_int32_2buf(char *buf, int n)
{
    //bcopy((char *)&n, buf, sizeof(int));
	memcpy(buf,(char *)&n,sizeof(int));
}

void send_int16_2buf(char *buf, short int n)
{
	/*
	union MYINT16{
        	char sTmp[2];
        	short int nTmp;
	} tmp;
	tmp.nTmp = n;
	int i = 0;
	for(i = 0;i<2;i++) buf[i] = tmp.sTmp[i];*/
	memcpy(buf,(char *)&n,sizeof(short int));
}

void send_string_2buf(char *buf, char*str)
{
    //bcopy(str, buf, strlen(str));
	memcpy(buf,str,strlen(str));
}

void recv_long64_from(char *buf,long long int *n)
{
	memcpy(n,buf,sizeof(long long int));
}

void recv_int32_from(char *buf, int *n)
{
    //bcopy(buf, (void*)n, sizeof(int));
	memcpy(n,buf,sizeof(int));
}

void recv_int16_from(char *buf, short int *n)
{
    //bcopy(buf, (void*)n, sizeof(short int));
	memcpy(n,buf,sizeof(short int));
}

int openSocket(char *host, int port,int forkid) 
{
	long ipAddress;
	struct hostent* hostInfo;
	struct sockaddr_in sockInfo;
	struct sockaddr_in addr;
	int sock;
	//char tmp[128]="";
	//sprintf(tmp,"host = [%s] port = %d forkid = %d ip = [%s]",host,port,forkid,myshm->config[myshm->ssl[forkid].eppid].ip);
	//writeFile("/root/epp.log",tmp);
	memset(&sockInfo, 0, sizeof(sockInfo));
	sockInfo.sin_family = AF_INET;
	sockInfo.sin_port = htons(port);
	ipAddress = inet_addr(host);
	if ((int)ipAddress < 0) 
	{
		hostInfo = gethostbyname(host);
		ipAddress = *(long *)*hostInfo->h_addr_list;
	}
	sockInfo.sin_addr.s_addr = ipAddress;

	// Open the socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	// And connect
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	//addr.sin_port=htons(my_port);
	addr.sin_port=INADDR_ANY;
	addr.sin_addr.s_addr = INADDR_ANY;
	//addr.sin_addr.s_addr = inet_addr(myshm->config[myshm->ssl[forkid].eppid].ip);
	if (bind(sock,(struct sockaddr*)&addr,sizeof(addr)) == -1)
	{
		printf("bind err\n");
		return -1;
	}
	if (connect (sock, (struct sockaddr*)&sockInfo, sizeof(sockInfo)) == -1)
	{
		printf("fork%d connect err\n",forkid);
		return -1;
	}
	
	//sprintf(tmp,"fork%d connect ok",forkid);
	//writeFile("/root/epp.log",tmp);
	//int one=1;
	/*
	if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char *)&one,sizeof(int)) < 0)
	{
		printf("setsockopt error %d %s\n",__LINE__,__FILE__);
		close(sock);
		return -1;
	}*/
	return sock;
}

int App_Send(int socket_fd,char *buff,int b_size)
{
	ssize_t	nleft;
	ssize_t nwrite;
	ssize_t nsendlength = 0;
	char *ptr;
	nleft = b_size;
	ptr = buff;

	if(socket_fd <= 0 || socket_fd >= 1022032){
		printf("socket_fd error = %d\n",socket_fd);
 		return -2;
	}
	ptr = buff;

	int n=0;
	while(nleft>0){
		if((nwrite=write(socket_fd,ptr,nleft))<0){
			if(errno == EINTR){
				printf("Send errno == EINTR\n");
				nwrite=0;
				continue;
			}
			else{
				return -1;
			}
		}
		nleft -= nwrite;
		nsendlength += nwrite;
		if(nleft<=0)
			break;
		ptr += nwrite;

	}

	return nwrite;
}
//tcp 协议
//	'|'	+	length(int)	+	command(int)	+	session(int)	+	内容	
int App_Recv(int socket,char *buff,int size)
{
	int flag,nread,length,timeout,result;
	char *ptr;
	memset(buff,'\0',size);
	*buff = '0';
	ptr = buff;
	timeout = 0;
	while (*ptr != '|')
	{
		nread = read(socket,ptr,1);
		if (nread <= 0)
		{
			return -1;
		}
	}
	ptr++;
	length = 4;
	while (length > 0)
	{
		nread = read(socket,ptr,length);
		if (nread <= 0)
		{
			return -1;
		}
		length -= nread;
		ptr += nread;
	}

	recv_int32_from(buff+1,&length);
	//printf("TCP recv length = %d\n",length);
	length -= 5;
	result = length;
	if (result > size)
	{
		//长度超限
		return -1;
	}
	while (length > 0)
	{
		nread = read(socket,ptr,length);
		if (nread <= 0)
		{
			return -1;
		}
		length -= nread;
		ptr += nread;
	}
	
	*ptr = '\0';
	return result+5;
}

void robot(int tid)
{
	int i,sock,length,flag,time_start,time_begin,time_end;
	int command,session,uid=10041;
	int qh;
	long long int gold;
	time_t timeNow;
	short int login,status;
	char buf[1024];
	char token[32]="";
	char nDate[16];
	char *p;
	struct tm *t;
	/*p = token;
	for (i=0;i<32;i++)
	{
		sprintf(p,"%d",tid);
		p++;
	}*/
	time_start = time(NULL);
	sprintf(token,"002df028f1a268a7e9bf1821e8b4a562");
	printf("thread uid %d token=[%s] start\n",uid,token);
	sock = openSocket(HOST,PORT,tid);
	if (sock < 0)
	{
		printf("socket open error\n");
		return;
	}
	//printf("thread %d ->sokcet %d ok!\n",tid,sock);
	//获取session
	flag = App_Recv(sock,buf,1024);
	if (flag < 0)
	{
		printf("thread %d recv socket %d broken!\n",tid,sock);
	}
	recv_int32_from(buf+5,&command);
	recv_int32_from(buf+9,&session);
	//printf("command = %d session = %d\n",command,session);

	//进入大厅成功
	flag = App_Recv(sock,buf,1024);
	if (flag < 0)
	{
		printf("thread %d recv socket %d broken!\n",tid,sock);
	}
	recv_int32_from(buf+5,&command);
	//printf("command = %d\n",command);
	
	//请求登录
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,49);
	p+=4;
	send_int32_2buf(p,1001);
	p+=4;
	send_int32_2buf(p,session);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_string_2buf(p,token);
	App_Send(sock,buf,49);
	//登录是否成功
	flag = App_Recv(sock,buf,1024);
	if (flag < 0)
	{
		printf("thread %d recv socket %d broken!\n",tid,sock);
	}
	recv_int32_from(buf+5,&command);
	recv_int16_from(buf+9,&login);
	recv_int16_from(buf+11,&status);
	printf("command = %d login=%d status=%d\n",command,login,status);
	if (login < 0)
	{
		printf("uid %d login failed\n",uid);
		return;
	}
	return;
	time_begin = time(NULL);
	while (1)
	{
		//999心跳
		time_end = time(NULL);
		if (time_end - time_begin >= 14)
		{
			p = buf;
			*p = '|';
			p++;
			send_int32_2buf(p,17);
			p+=4;
			send_int32_2buf(p,999);
			p+=4;
			send_int32_2buf(p,session);
			p+=4;
			send_int32_2buf(p,uid);
			flag = App_Send(sock,buf,17);
			if (flag < 0)
			{
				printf("thread %d send socket %d broken!\n",tid,sock);
				close(sock);
				break;
			}
			printf("send 999 ok!...\n");
			sprintf(buf,"online time = %d seconds",time_end-time_start);
			writeFile("/root/robot.log",buf);
			time_begin = time_end;
		}
		
		flag = App_Recv(sock,buf,1024);
		if (flag < 0)
		{
			printf("thread %d recv socket %d broken!\n",tid,sock);
			break;
		}
		else if (flag > 0)
		{
			recv_int32_from(buf+5,&command);
			printf("command = %d\n",command);
		}
		sleep(1);
		sprintf(buf,"while time=%d",time_end);
		writeFile("/root/robot.log",buf);
	}
	time_end = time(NULL);
	sprintf(buf,"\nonline time_total = %d seconds",time_end-time_start);
	writeFile("/root/robot.log",buf);
	return;
	//时时彩下注
	timeNow = time(NULL);
	t = localtime(&(timeNow)); 
	sprintf(nDate,"%d%02d%02d%04d",t->tm_year-117,1+t->tm_mon,t->tm_mday,t->tm_hour*60+t->tm_min+1);
	qh = atoi(nDate);
	printf("qh=%d\n",qh);

	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,63);
	p+=4;
	send_int32_2buf(p,1015);
	p+=4;
	send_int32_2buf(p,session);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_int32_2buf(p,qh);
	p+=4;
	send_int32_2buf(p,1);
	p+=2;
	send_long64_2buf(p,20000);
	p+=8;
	send_string_2buf(p,token);
	App_Send(sock,buf,63);

	flag = App_Recv(sock,buf,1024);
	if (flag < 0)
	{
		printf("thread %d recv socket %d broken!\n",tid,sock);
	}
	recv_int32_from(buf+5,&command);
	recv_int16_from(buf+9,&login);
	recv_int16_from(buf+11,&status);
	recv_long64_from(buf+13,&gold);
	printf("command = %d flag=%d type=%d gold=%lld\n",command,login,status,gold);
	//是否进入房间

	//入桌

	//座位信息

		
	while (1)
	{
		sleep(10);
	}
}


void* RoBot(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	free(arg);
	
	robot(i);
	return NULL;
}


int main(int argc, char *argv[])
{
	int i;
	pthread_t thid[BOT_MAX];
//	daemon_init();
	for (i=0;i<BOT_MAX;i++)
	{
		int *p = malloc(sizeof(int));
		*p = i;
		if (0 != (pthread_create(&(thid[i]),NULL,(void *)RoBot,(void *)p)))
		{
			printf("create accept thread failed!\n");
			exit(0);
		}
	}
	while (1)
	{
		sleep(10);
	}
	return 0;
}
