#include "mytcp.h"
#include "myshm.h"
#include "mybase.h"
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <stdio.h>
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

//sigjmp_buf jmp_env;

//static sock_type = 0;	//0.tcp	1.unix
extern int epfd;
extern HALL *hall;

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
/*
static void connect_alarm(int signal)
 {
//	siglongjmp(jmp_env, 1);
	printf("connect time out\n");
	exit(-1);
 }*/

void XORStr(char *buff,int b_size)
{
	int i;
	for(i = 0;i < b_size;i++)
		buff[i] ^= 0xFF; 
}

int	init_sock(char *host,int port_id)
{
	int socket_fd;
	struct sockaddr_in  serv_addr;
	struct hostent *h = gethostbyname(host);

	if(!h){
		printf("Host name lookup failure for %s\n",host);
		return -1;
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&(serv_addr.sin_addr.s_addr), h->h_addr, h->h_length);
	serv_addr.sin_port              = htons(port_id);

  if ( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
      printf("TcpClient: can't open stream socket");
      return -1;
    }
	if (connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("TcpClient: can't connect to server : %s",strerror(errno ));
		return -1;
	}
	return socket_fd;

};
//tcp 协议
//	'|'	+	length(int)	+	command(int)	+	session(int)	+	内容
int App_Send(int socket_fd,char *buff,int b_size)
{
	
	ssize_t	nleft;
	ssize_t nwrite;
	ssize_t nsendlength;
	//test
	/*char mybuf[1024];
	recv_int32_from(buff+1,(int *)&nsendlength);
	recv_int32_from(buff+5,(int *)&nleft);
	sprintf(mybuf,"TCP socket%d App_Send size=%d command = %d length=%d time=%d",socket_fd,b_size,(int)nleft,(int)nsendlength,(int)hall->time_now);
	writeFile("/root/tcp.log",mybuf);*/
	//
	char *ptr;
	nleft = b_size;
	ptr = buff;
	nwrite = -1;
	nsendlength = 0;
	
	if(socket_fd <= 0 || socket_fd >= 1022032){
		printf("socket_fd error = %d\n",socket_fd);
 		return -2;
	}

	ptr = buff;

	int n=0;
	while(nleft>0){
		n = checkWriteSocket(socket_fd,MAXTIMEWAIT_WRITE);
		if(n < 0 ){	
			printf("write n=%d %s \n",n,strerror(errno));
			return -1;
		}
		if(n == 0){
			printf("write  n=%d %s \n",n,strerror(errno));
                        return -1;
		}
		
		if((nwrite=write(socket_fd,ptr,nleft))<0){
			if(errno == EINTR){
				printf("Send errno == EINTR\n");
				nwrite=0;
				continue;
			}
			else{
                //printf("Send err:len=%d  %d--%s sock = %d",b_size-nleft,getpid(),strerror(errno),socket_fd);
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
	//memset(buff,'\0',size);
	*buff = '0';
	ptr = buff;
	timeout = 0;
	while (*ptr != '|')
	{
		flag = checkSocket(socket,TIMEOUT);
		if (flag > 0)
		{
			nread = read(socket,ptr,1);
			if (nread <= 0)
			{
				return -1;
			}
		}
		else if (flag == 0)
		{
			//timeout
			return 0;
		}
		else
		{
			//error
			return -1;
		}
	}
	ptr++;
	length = 4;
	while (length > 0)
	{
		flag = checkSocket(socket,TIMEOUT);
		if (flag > 0)
		{
			nread = read(socket,ptr,length);
			if (nread <= 0)
			{
				return -1;
			}
			length -= nread;
			ptr += nread;
		}
		else if (flag == 0)
		{
			//timeout
			timeout++;
			if (timeout > 50) return -1;
			continue;
		}
		else
		{
			return -1;
		}
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
		flag = checkSocket(socket,TIMEOUT);
		if (flag > 0)
		{
			nread = read(socket,ptr,length);
			if (nread <= 0)
			{
				return -1;
			}
			length -= nread;
			ptr += nread;
		}
		else if (flag == 0)
		{
			//timeout
			timeout++;
			if (timeout > 50) return -1;
			continue;
		}
		else
		{
			return -1;
		}
	}
	
	*ptr = '\0';
	return result+5;
}

int checkSocket( int sock, int timeout )
{
	int rtn;
	fd_set fi;
	fd_set fo;
	fd_set fe;
	struct timeval tp;
	FD_ZERO(&fi);
	FD_ZERO(&fo);
	FD_ZERO(&fe);
	FD_SET(sock, &fi);
	FD_SET(sock, &fe);
	tp.tv_sec = timeout;
	tp.tv_usec = 200000; //0.2s

	rtn = select( sock + 1, &fi, &fo, &fe, &tp);
	if( rtn < 0 )
	{
		return -1;
	}
	if( rtn == 0 )
	{
		return 0;
	}
	if( FD_ISSET(sock, &fi) )
	{
		return +1;
	}
	if( FD_ISSET(sock, &fe) )
	{
		return -1;
	}
	return 0;
}

int read_fd(int fd)
{
	char tmpbuf[CONTROLLEN];
     struct cmsghdr *cmptr = (struct cmsghdr *) tmpbuf;
     struct iovec iov[1];
     struct msghdr msg;
     int recvfd;
     char buf[1];
     iov[0].iov_base = buf;
     iov[0].iov_len = sizeof (buf);
     msg.msg_iov = iov;
     msg.msg_iovlen = 1;
     msg.msg_name = NULL;
     msg.msg_namelen = 0;
     msg.msg_control = cmptr;
     msg.msg_controllen = CONTROLLEN;
     if (recvmsg(fd, &msg, 0) <= 0) {
        perror("recvmsg failed");
        return(-2);
     }
     if((cmptr = CMSG_FIRSTHDR(&msg)) != NULL && cmptr->cmsg_len == CMSG_LEN(sizeof(int))){
        if(cmptr->cmsg_level != SOL_SOCKET){
          printf("read_fd error cmptr->cmsg_level != SOL_SOCKET \n");
          return -1;
        }
        if(cmptr->cmsg_type != SCM_RIGHTS){
          printf("read_fd error cmptr->cmsg_type != SCM_RIGHTS \n");
          return -1;
	}
	recvfd = *(int *) CMSG_DATA (cmptr);
     }else
	recvfd = -1;
	
    //printf("recvfd = %d\n", recvfd);
	return recvfd;/*传递句柄*/
}
/* end read_fd */

int write_fd(int fd, int sendfd)
{
    char tmpbuf[CONTROLLEN];
    struct cmsghdr *cmptr = (struct cmsghdr *) tmpbuf;
    struct iovec iov[1];
    struct msghdr msg;
    char buf[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_control = cmptr;
    msg.msg_controllen = CONTROLLEN;
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    cmptr->cmsg_len = CONTROLLEN;
    *(int *)CMSG_DATA (cmptr) = sendfd;
 
    if (sendmsg(fd, &msg, 0) < 0) {
       perror("sendmsg failed");
       return -1;
    }
	return 1;

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

int checkWriteSocket( int sock, int timeout )
{
	int rtn;
	fd_set fi;
	fd_set fo;
	fd_set fe;
	struct timeval tp;
	FD_ZERO(&fi);
	FD_ZERO(&fo);
	FD_ZERO(&fe);
	FD_SET(sock, &fo);
	FD_SET(sock, &fe);
	tp.tv_sec = timeout;
	tp.tv_usec = 0;//100000 * MAXTIMEWAIT; 
	rtn = select( sock + 1, &fi, &fo, &fe, &tp);
	if( rtn < 0 )
	{
		return -1;
	}
	if( rtn == 0 )
	{
		return 0;
	}
	if( FD_ISSET(sock, &fo) )
	{
		return +1;
	}
	if( FD_ISSET(sock, &fe) )
	{
		return -1;
	}
	return 0;
}
/* end write_fd */


//#define DEBUG_CLI 1

#ifdef	DEBUG_CLI
#endif
