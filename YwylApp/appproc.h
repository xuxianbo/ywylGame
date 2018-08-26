#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sys/syscall.h"
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "cards.h"
#include "zjh.h"
#include "bjl.h"
#include "ox.h"
#include "wzq.h"
#include "mj.h"
#include "tcppack.h"

#define  MYPORT  28888
#define  HALL_USER_MAX 1500
//
//#define  EPOLL_SIZE_MAX 5120

int childMake(int i);

void masterproc(int forkid);

void hallClientproc(int sock);

void sendSession(int sock);

void* HallClientThread(void *arg);

void* HallFirstThread(void *arg);

int userLogin(char *token,int user);

void* BJLThread(void *arg);

void* WZQThread(void *arg);

int BaseCommand(int forkid,int user,int sock,int command,char *buf);