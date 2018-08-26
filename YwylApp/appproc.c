#include "appproc.h"

extern int epfd;
extern HALL *hall;
//pthread_mutex_t mutex;

void* ClientThread(void *arg)
{
	void childproc(int);
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	childproc(i);
	close(i);
	return NULL;
}

void* HallFirstThread(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	sendSession(i);
	close(i);
	__sync_fetch_and_sub(&(hall->thread),1);
	return NULL;
}

void* HallClientThread(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	hallClientproc(i);
	close(i);
	__sync_fetch_and_sub(&(hall->thread),1);
	return NULL;
}

void* MasterThread(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	free(arg);
	
	masterproc(i);
	exit(0);
	//return NULL;
}

void* BJLThread(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	bjlDealer(i);
	exit(0);
	//return NULL;
}

void* WZQThread(void *arg)
{
	int i = (int)(*(int*)arg);
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	wzqDealer(i);
	exit(0);
	//return NULL;
}
/*
void* EpollThread(void *arg)
{
	void epollproc(struct epoll_event*);
	
	if(pthread_detach(pthread_self()) != 0){
		return(NULL);
	}
	epollproc((struct epoll_event *)arg);
}*/

int childMake(int i)
{
	pid_t pid;
	if(0!=socketpair(AF_UNIX,SOCK_STREAM,0,hall->fork[i].fd))
	{
		perror("socketpair failed\n");
		return -1;
	}
	if ((pid = fork()) != 0)
	{
		if (pid > 0)
		{
			/*father process*/
			hall->fork[i].child_pid = pid;
			hall->fork[i].status = 1;
			//close(hall->fork[i].fd[1]);
			pthread_t fhid;
			
			int *p = malloc(sizeof(int));
			*p = i;
			if (pthread_create(&fhid,NULL,(void *)MasterThread,(void *)p) != 0)
			{
				printf("create master thread error!\n");
				exit(-1);
			}
			return pid;
		}
		else
		{
			/*fork error*/
			return pid;
		}
	}
	int j;
	sem_init(&redis_lock,0,1);
	redis_flag = 0;

	for (j=0;j<MYSQL_MAX;j++)
	{
		sem_init(&mysql_lock[j],0,1);
		mysql_flag[j] = 0;
	}
	//close(hall->fork[i].fd[0]);
	/*child process*/
	int sd[128];
	pthread_t thid;
	
	//pthread_t poid;
	//epoll
	//struct epoll_event ev,events[EPOLL_SIZE_MAX];
	//epfd = epoll_create(1024);
	//
	/*
	if (pthread_create(&poid,NULL,(void *)EpollThread,(void *)&events) != 0)
	{
		printf("create thread error!\n");
		exit(0);
	}*/
	//初始化共享内存
	hall->fork[i].thread = 0;
	//pthread_mutex_init(&mutex,NULL);
	//
	int forkid = i;
	if (hall->fork[forkid].gameid == 0 || hall->fork[forkid].gameid == 4)
	{
		//百家乐
		pthread_t bjl;
		if (pthread_create(&bjl,NULL,(void *)BJLThread,&forkid) != 0)
		{
			printf("create bjl thread error!\n");
			exit(-1);
		}
	}
	else if (hall->fork[forkid].gameid == 5)
	{
		//五子棋
		pthread_t wzq;
		if (pthread_create(&wzq,NULL,(void *)WZQThread,&forkid) != 0)
		{
			printf("create bjl thread error!\n");
			exit(-1);
		}
	}
	int sd_i,one;
	sd_i = 0;
	//
	while(1)
	{
		sd[sd_i] = read_fd(hall->fork[i].fd[1]);
		//printf("fork %d read fd %d\n",i,sd[sd_i]);
		if (sd[sd_i] < 0) continue;
		//cflags = fcntl(sd[i],F_GETFL,0);
		//fcntl(sd[sd_i],F_SETFL,cflags|O_NONBLOCK);	
		/*
		if (setsockopt(sd[sd_i],IPPROTO_TCP,TCP_NODELAY,(char *)&one,sizeof(int)) < 0)
		{
			printf("setsockopt error %d %s\n",__LINE__,__FILE__);
			close(sd[sd_i]);
			continue;
		}*/
		/*	
		ev.data.fd = sd;
		ev.events=EPOLLIN;
		if(epoll_ctl(epfd,EPOLL_CTL_ADD,sd,&ev) < 0 || pthread_create(&thid,NULL,(void *)ClientThread,(void *)sd) != 0)
		{
			fprintf(stderr," 把 socket '%d' 加入epoll失败! %s\n",sd,strerror(errno));
			close(sd);
			continue;
		}*/
		one = 1;
		if (setsockopt(sd[sd_i],IPPROTO_TCP,TCP_NODELAY,(char *) &one, sizeof(int)) < 0) 
		{
			printf("Tcp: failed on setsockopt(): %s\n",strerror(errno));
			close(sd[sd_i]);
			continue;
		}
		
		if (pthread_create(&thid,NULL,(void *)ClientThread,&sd[sd_i]) != 0)
		{
			printf("create thread error!\n");
			close(sd[sd_i]);
			continue;
		}
		
		//__sync_fetch_and_add(&(hall->fork[i].thread),1);
		sd_i++;
		if (sd_i >= 128) sd_i = 0;
	}
	return 0;
}

void masterproc(int forkid)
{
	//printf("master proc forkid = %d\n",forkid);
	int sd[128],sd_i=0,one;
	pthread_t thid;
	
	while(1)
	{
		sd[sd_i] = read_fd(hall->fork[forkid].fd[0]);
		if (sd[sd_i] < 0) continue;
		one = 1;
		if (setsockopt(sd[sd_i],IPPROTO_TCP,TCP_NODELAY,(char *) &one, sizeof(int)) < 0) 
		{
			printf("Tcp: failed on setsockopt(): %s\n",strerror(errno));
			close(sd[sd_i]);
			continue;
		}
		//printf("fork%d write fd to fork1\n",forkid);
		//write_fd(hall->fork[1].fd[0],sd[sd_i]);
		if (pthread_create(&thid,NULL,(void *)HallClientThread,&sd[sd_i]) != 0)
		{
			printf("create thread error!\n");
			close(sd[sd_i]);
			continue;
		}
		
		__sync_fetch_and_add(&(hall->thread),1);
		sd_i++;
		if (sd_i >= 128) sd_i = 0;
	}
}

//	'|'	+	length(int)	+	command(int)	+	session(int)	+	内容
void sendSession(int sock)
{
	int session,forkid,user,length,timeout=0;
	char buf[RECV_BUF_MAX];
	
	length = pack555(buf);
	if (App_Send(sock,buf,length) < 0)
	{
		return;
	}
	
	while (1)
	{
		length = App_Recv(sock,buf,RECV_BUF_MAX);
		//printf("send session length = %d\n",length);
		if (length < 0)
		{
			return;
		}
		else if (length == 0)
		{
			timeout++;
			if (timeout >= HALL_TIME_OUT)
			{
				length = pack666(buf,0);
				App_Send(sock,buf,length);
				return;
			}
		}
		else
		{
			printf("[%s]\n",buf+17);
			if (strcmp(buf+17,CONNECT_CHECK) != 0)
			{
				//tcp连接校验失败
				//printf("connect check no!\n");
				length = pack666(buf,0);
				App_Send(sock,buf,length);
				return;
			}
			else 
			{
				//校验成功
				break;
			}
		}
	}
	
	length = pack666(buf,1);
	if (App_Send(sock,buf,length) < 0)
	{
		return;
	}
	//
	//检查ip连接数
	struct sockaddr_in sa;
	int len = sizeof(sa);
	char ip[IP_MAX];
	memset(ip,'\0',IP_MAX);
	if(!getpeername(sock,(struct sockaddr *)&sa,(socklen_t *)&len))
	{
		sprintf(ip,inet_ntoa(sa.sin_addr));
	}

	if (strlen(ip) <= 0)
	{
		//获取IP失败
		return;
	}

	if (checkIP(ip) < 0)
	{
		sprintf(buf,"check ip[%s] error!",ip);
		writeFile("/root/ywyl_error.log",buf);
		return;
	}

	session = createSession(sock,ip);
	user = getUserBySession(session);

	length = pack1000(buf,session);
	if (App_Send(sock,buf,length) < 0)
	{
		sprintf(buf,"App_Send socket=%d error!",sock);
		writeFile("/root/ywyl_error.log",buf);
		cleanUser(user);
		return;
	}
	//printf("send session to sock %d session=%d user=%d gold=%lld money=%lld ...\n",sock,session,user,hall->user[user].gold,hall->user[user].money);

	forkid = selectForkid(user,0,1,100,0);
	if (forkid < 0)
	{
		//printf("select forkid = %d\n",forkid);
		//全部服务器房间爆满
		//length = pack2002(buf,0,0,0,0);
		//App_Send(sock,buf,length);
		sprintf(buf,"selectForkid forkid=%d error!",forkid);
		writeFile("/root/ywyl_error.log",buf);
		cleanUser(user);
		return;
	}

	length = pack777(buf);
	App_Send(sock,buf,length);
	hall->user[user].status = 2;
	hall->user[user].forkid = forkid;
	hall->user[user].socket = -2;
	__sync_fetch_and_add(&(hall->fork[forkid].thread),1);
	write_fd(hall->fork[forkid].fd[0],sock);
	return;
}

void hallClientproc(int sock)
{
	int forkid,flag,user,uid,timeout,flag_switch=0,length;
	int command,session,house;
	short int gid,type,grade;
	char buf[RECV_BUF_MAX];
	//char token[32]="";
	timeout = 0;
	user = -1;

	//进入大厅成功
	//printf("!!!!!!hall client %d enter ...\n",sock);
	
	//recv
	while (1)
	{
		flag = App_Recv(sock,buf,RECV_BUF_MAX);
		if (flag < 0)
		{
			//printf("app scok = %d broken\n",sock);
			//sprintf(buf,"hallproc client %d broken %s",sock,formatTime(logbuf));
			//writeFile(LOG_PATH,buf);
			break;
		}
		else if (flag == 0)
		{
			//time out
			timeout++;
			if (timeout%20 == 0)
			{
				printf("hall client %d timeout ...\n",sock);
			}
			if (timeout >= HALL_TIME_OUT)
			{
				//超时断开
				//printf("time out ... kill sock %d\n",sock);
				//sprintf(buf,"hallproc client %d time out %d... %s",sock,timeout,formatTime(logbuf));
				//writeFile(LOG_PATH,buf);
				break;
			}
		}
		else
		{
			//ok
			//printf("app recv flag = %d\n",flag);
			recv_int32_from(buf+5,&command);
			recv_int32_from(buf+9,&session);
			recv_int32_from(buf+13,&uid);
			user = checkSession(-1,session,uid,sock);
			//printf("HALL user=%d,command=%d,session=%d,uid=%d\n",user,command,session,uid);
			if (user < 0)
			{
				//session 错误
				break;
			}
			
			switch (command)
			{
				case 1002:
					
					if (hall->user[user].hds >= 0)
					{
						//游戏中 强制跳回
						forkid = hall->user[user].hds/1000;
					}
					else
					{
						//选择进入房间
						recv_int16_from(buf+17,&gid);
						recv_int16_from(buf+19,&type);
						recv_int16_from(buf+21,&grade);
						recv_int32_from(buf+23,&house);
						//printf("1002 user=%d,gid=%d,type=%d,grade=%d,house=%d\n",user,gid,type,grade,house);
						forkid = selectForkid(user,gid,type,grade,house);
						//printf("1002 forkid = %d\n",forkid);
						if (forkid < 0)
						{
							//指定服务器爆满
							length = pack2002(buf,forkid,gid,type,grade);
							if (UserSend(user,buf,length) < 0)
							{
								flag_switch = 1;
								break;
							}

							//sprintf(buf,"uid=%d gid=%d type=%d house=%d 进入房间失败! %d\n",hall->user[user].uid,gid,type,house,forkid);
							//writeFile(LOG_PATH,buf);
							forkid = selectForkid(user,0,1,100,0);
							if (forkid < 0)
							{
								//全部服务器房间爆满
								//length = pack2002(buf,0,0,0,0);
								//UserSend(user,buf,length);
								flag_switch = 1;
								break;
							}
							if (hall->user[user].status != 10)
							{
								flag_switch = 1;
								break;
							}
							hall->user[user].status = 2;
							hall->user[user].forkid = forkid;
							hall->user[user].socket = -2;
							hall->user[user].time = hall->time_now;
							__sync_fetch_and_add(&(hall->fork[forkid].thread),1);
							write_fd(hall->fork[forkid].fd[0],sock);
							return;
						}
					}

					hall->user[user].status = 2;
					hall->user[user].forkid = forkid;
					hall->user[user].socket = -2;
					hall->user[user].time = hall->time_now;
					__sync_fetch_and_add(&(hall->fork[forkid].thread),1);
					write_fd(hall->fork[forkid].fd[0],sock);
					return;
				case 999:
					//心跳
					length = pack999(buf);
					if (UserSend(user,buf,length) < 0)
					{
						flag_switch = 1;
					}
					break;
				default:
					//flag_switch = 1;
					break;
			}
			if (flag_switch)
			{
				break;
			}
		}
	}//end while
	//
	if (user < 0)
	{
		getUserBySocket(-1,sock,&user);
	}

	cleanUser(user);
	//
}

int userLogin(char *token,int user)
{
	if (hall->flag != 1 || user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	//char logbuf[1024];
	char name[USER_NAME_MAX];
	char url[USER_HEAD_URL];
	int i,m,uid,tmp_user=-1,forkid,desk,seat;
	short int sex;
	long long int gold,money,stock;

	//get uid to do
	//redis
	uid = getUserByToken(token,user,&tmp_user,&gold,&money,&stock,name,url,&sex);
	if (uid <= 0)
	{
		//登录失败
		printf("redis get user error!\n");
		return -1;
	}
	//printf("user login uid=%d,user=%d,tmp_user=%d\n",uid,user,tmp_user);
	if (tmp_user < 0)
	{
		//sprintf(logbuf,"user=%d,tmp_user=%d,uid=%d",user,tmp_user,uid);
		//writeFile("/root/login.log",logbuf);
	}
	else 
	{
		//sprintf(logbuf,"user=%d,tmp_user=%d,uid=%d,tmp_user.uid=%d,tmp_user.status=%d",user,tmp_user,uid,hall->user[tmp_user].uid,hall->user[tmp_user].status);
		//writeFile("/root/login.log",logbuf);

		sem_wait(&(hall->user[tmp_user].lock_user));
		if (hall->user[tmp_user].uid == uid && (hall->user[tmp_user].status == 10 || hall->user[tmp_user].status == 4 || hall->user[tmp_user].status == 2))
		{
			hall->user[user].uid =			uid;
			hall->user[user].status =		10;
			hall->user[user].login =		1;
			hall->user[user].sex =			hall->user[tmp_user].sex;
			hall->user[user].hds =			hall->user[tmp_user].hds;
			hall->user[user].gold_init =	hall->user[tmp_user].gold_init;
			hall->user[user].gold =			hall->user[tmp_user].gold;
			hall->user[user].money_init =	hall->user[tmp_user].money_init;
			hall->user[user].money =		hall->user[tmp_user].money;
			hall->user[user].stock =		hall->user[tmp_user].stock;
			hall->user[user].msg_gold_update = hall->user[tmp_user].msg_gold_update;		
			hall->user[user].msg_id =		hall->user[tmp_user].msg_id;			
			sprintf(hall->user[user].name,hall->user[tmp_user].name);
			sprintf(hall->user[user].url,hall->user[tmp_user].url);

			sem_post(&(hall->user[tmp_user].lock_user));
			//百家乐
			//
			for (m=0;m<2;m++)
			{
				for (i=0;i<BJL_BET_MAX;i++)
				{
					hall->user[user].bet[m][i] =	hall->user[tmp_user].bet[m][i];
				}
				for (i=0;i<BJL_BET_MAX;i++)
				{
					hall->user[user].gold_win[m][i] = hall->user[tmp_user].gold_win[m][i];
				}
				hall->user[user].banker_flag[m] =	hall->user[tmp_user].banker_flag[m];
				hall->user[user].bjl_flag[m] =		hall->user[tmp_user].bjl_flag[m];
				hall->user[user].bjl_balance[m] =	hall->user[tmp_user].bjl_balance[m];
				//百家乐 庄家
				if (hall->user[user].banker_flag[m] != 0)
				{
					sem_wait(&(hall->bjl[m].lock_banker));
					if (hall->bjl[m].banker == tmp_user)
					{
						//正在坐庄 或请求下庄中
						hall->bjl[m].banker = user;
					}
					else if (hall->user[user].banker_flag[m] == 1)
					{
						//上庄列表中
						for (i=0;i<BANKER_MAX;i++)
						{
							if (hall->bjl[m].banker_list[i] == tmp_user)
							{
								hall->bjl[m].banker_list[i] = user;
								break;
							}
						}
					}
					sem_post(&(hall->bjl[m].lock_banker));
				}
			}
			//订单
			sem_wait(&(hall->user[tmp_user].lock_order));
			for (m=0;m<ORDER_MAX;m++)
			{
				hall->user[user].order[m].order = hall->user[tmp_user].order[m].order;
				hall->user[user].order[m].create_time = hall->user[tmp_user].order[m].create_time;
			}
			hall->user[user].order_num = hall->user[tmp_user].order_num;
			sem_post(&(hall->user[tmp_user].lock_order));

			//
			if (hall->user[user].hds > 0)
			{
				//游戏中
				forkid = (hall->user[user].hds)/1000;
				desk = ((hall->user[user].hds)%1000)/10;
				seat = (hall->user[user].hds)%10;
				
				if (hall->fork[forkid].gameid == 0)
				{
					//百家乐 可视座位
					if (seat <= BJL_SEAT_MAX && hall->bjl[0].seat[seat].user == tmp_user)
					{
						hall->bjl[0].seat[seat].user = user;
					}
				}
				else if (hall->fork[forkid].gameid == 4 && hall->bjl[1].seat[seat].user == tmp_user)
				{
					if (seat <= BJL_SEAT_MAX)
					{
						hall->bjl[1].seat[seat].user = user;
					}
				}
				else
				{
					//其它
					sem_wait(&(hall->fork[forkid].zjh[desk].lock));
					if (hall->fork[hall->user[user].hds/1000].grade == PRIVATE_HOUSE && hall->fork[forkid].zjh[desk].desk_status == 0)
					{
						//私人场桌已解散
						hall->user[user].hds = -1;
					}
					else if (hall->fork[forkid].zjh[desk].seat[seat].user == tmp_user)
					{
						hall->fork[forkid].zjh[desk].seat[seat].user = user;
					}
					else
					{
						hall->user[user].hds = -1;
					}
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
				}
				
			}
			
			hall->user[tmp_user].login = 0;
			cleanUser(tmp_user);
			
			return 0;
		}
		sem_post(&(hall->user[tmp_user].lock_user));
	}
	
	//未登录
	hall->user[user].uid = uid;
	hall->user[user].status = 10;
	hall->user[user].login = 1;
	hall->user[user].sex = sex;
	hall->user[user].gold_init = gold;
	hall->user[user].gold = gold;
	hall->user[user].money_init = money;
	hall->user[user].money = money;
	hall->user[user].stock = stock;
	sprintf(hall->user[user].name,name);
	sprintf(hall->user[user].url,url);
	hall->user[user].msg_gold_update = 0;		
	hall->user[user].hds = -1;
	if (hall->site_msg-1 < 0)
	{
		hall->user[user].msg_id = hall->aaa_msg[MSG_LIST_MAX-1].id;
	}
	else
	{
		hall->user[user].msg_id = hall->aaa_msg[hall->site_msg-1].id;
	}
	//百家乐
	for (m=0;m<2;m++)
	{
		for (i=0;i<BJL_BET_MAX;i++)
		{
			hall->user[user].bet[m][i] = 0;
		}
		for (i=0;i<BJL_BET_MAX;i++)
		{
			hall->user[user].gold_win[m][i] = 0;
		}
		hall->user[user].banker_flag[m] = 0;
		hall->user[user].bjl_flag[m] = 0;
		hall->user[user].bjl_balance[m] = 0;
	}

	//订单
	for (m=0;m<ORDER_MAX;m++)
	{
		hall->user[user].order[m].order = 0;
		hall->user[user].order[m].create_time = 0;
	}
	hall->user[user].order_num = 0;
	
	return 0;
}

void childproc(int sock)
{
	int i,j,forkid,user,flag,timeout,ret,flag_switch=0;
	int command,session,uid,length;
	char buf[RECV_BUF_MAX];
	//char mybuf[RECV_BUF_MAX];
	char logbuf[64];
	//struct tm *t;

	forkid = getForkid();
	//if (forkid < 0 || forkid >= FORK_MAX) return;
	timeout = 0;
	user = -1;

	//printf("child %d enter fork%d ...\n",sock,forkid);
	
	length = pack2002(buf,1,hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade);
	App_Send(sock,buf,length);

	length = pack998(buf);
	App_Send(sock,buf,length);

	while (1)
	{
		flag = App_Recv(sock,buf,RECV_BUF_MAX);
		if (flag < 0)
		{
			//断线
			printf("fork%d client socket %d broken flag=%d ...\n",forkid,sock,flag);
			break;
		}
		else if (flag == 0)
		{
			//无消息
			timeout++;
			if (timeout%20 == 0)
			{
				//printf("fork%d client %d timeout = %d ...\n",forkid,sock,timeout);
			}
			if (timeout >= TIME_OUT)
			{
				//超时30秒断开
				printf("time out ... kill socket %d\n",sock);
				break;
			}
			
			getUserBySocket(forkid,sock,&user);
			if (user < 0)
			{
				//未验证用户
				continue;
			}

			//关服信息
			if (hall->flag == 2)
			{
				ret = hall->time_over - hall->time_now;
				length = pack888(buf,ret);
				if (length > 0)
				{
					UserSend(user,buf,length);
					sleep(1);
				}
			}

			if (hall->time_now%60 == sock%60)
			{
				length = pack998(buf);
				UserSend(user,buf,length);
			}
			
			if (hall->user[user].login == 0)
			{
				//未登录
				continue;
			}
			
			//大奖信息
			j = hall->site_msg;
			for (i=0;i<MSG_LIST_MAX;i++)
			{
				//printf("j=%d\n",j);
				if ((hall->user[user].msg_id < hall->aaa_msg[j].id || hall->aaa_msg[j].id == 0) && hall->aaa_msg[j].length > 0)
				{
					//
					if (UserSend(user,hall->aaa_msg[j].msg,hall->aaa_msg[j].length) < 0)
					{
						break;
					}
					hall->user[user].msg_id = hall->aaa_msg[j].id;
					//
				}
				j++;
				if (j >= MSG_LIST_MAX)
				{
					j = 0;
				}
			}
			
			User_order_check(user);

			//金币mysql同步更新
			/*if (hall->time_now%30 == user%30)
			{
				updateUserGold(user);
			}*/
			
			//金币redis同步更新
			if (hall->user[user].msg_gold_update)
			{
				/*
				if (hall->user[user].hds >= 0 && hall->fork[forkid].gameid == 0)
				{
					//不通知
				}
				else
				{
					if (getUserGoldFromRedis(user,0) < 0)
					{
						flag_switch = 1;
						break;
					}
				}*/
				if (getUserGoldFromRedis(user,0) < 0)
				{
					flag_switch = 1;
					break;
				}
				hall->user[user].msg_gold_update = 0;
			}
			
			//
			if (hall->user[user].hds < 0)
			{
				//未入桌
				continue;
			}
			//按gameid区分游戏
			switch (hall->fork[forkid].gameid)
			{
				case 0:
					//百家乐
					BJLTimeOut(forkid,0,user,buf);
					break;
				case 1:
					//炸金花
					ZJHNormalTimeOut(forkid,user,buf);
					break;
				case 2:
					//牛牛
					OXTimeOut(forkid,user,buf);
					break;
				case 4:
					//推筒子
					BJLTimeOut(forkid,1,user,buf);
					break;
				case 5:
					WZQTimeOut(forkid,user,buf);
					break;
				case 6:
					MJTimeOut(forkid,user,buf);
					break;
				default:
					break;
			}
			//
		}
		else
		{
			//正常请求
			recv_int32_from(buf+5,&command);
			recv_int32_from(buf+9,&session);
			recv_int32_from(buf+13,&uid);
			user = checkSession(forkid,session,uid,sock);
			
			printf("fork%d socket %d recv command=%d session=%d uid=%d user=%d length=%d time=%d\n",forkid,sock,command,session,uid,user,flag,(int)hall->time_now);
			
			//printf("[%s]\n",mybuf);
			if (user < 0)
			{
				//session错误
				//sprintf(mybuf,"session error fork%d socket %d recv command=%d session=%d uid=%d user=%d length=%d time=%d\n",forkid,sock,command,session,uid,user,flag,(int)hall->time_now);
				//writeFile("/root/tcp.log",mybuf);
				break;
			}
			
			if (hall->user[user].login != 1)
			{
				//未登录
				if (command != 1001)
				{
					continue;
				}
				//printf("token=[%s]\n",buf+17);
				//sprintf(mybuf,"recv->ok fork=%d,socket=%d,command=%d,session=%d,uid=%d,user=%d,length=%d,login......time=%d !!!!!!",forkid,sock,command,session,uid,user,flag,(int)hall->time_now);
				//writeFile(LOGIN_PATH,mybuf);
				
				ret = userLogin(buf+17,user);
				if (ret < 0)
				{
					//登录失败
					cleanUser(user);
					length = pack2001(buf,-1,0,0,0,0,-1);
					App_Send(sock,buf,length);
					__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);
					sprintf(buf,"childproc exit->fork=%d,socket=%d,user=%d,uid=%d --> login failed!!! %s",forkid,sock,user,uid,formatTime(logbuf));
					writeFile(LOG_PATH,buf);
					return;
				}
				
				if (hall->user[user].hds < 0)
				{
					//正常
					ret = 0;
				}
				else 
				{
					//正在游戏中
					ret = 1;
				}

				//printf("fork%d uid:%d login OK! hds=%d,ret=%d,gid=%d,type=%d\n",forkid,hall->user[user].uid,hall->user[user].hds,ret,gid,type);
				//
				length = pack2001(buf,1,(short)ret,hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,createHDS(hall->user[user].hds/10*10));
				if (UserSend(user,buf,length) < 0)
				{
					flag_switch = 1;
					break;
				}
				getUserGoldFromRedis(user,0);

				//sprintf(mybuf,"send->ok fork=%d,socket=%d,command=%d,session=%d,uid=%d,user=%d,length=%d,login OK...time=%d",forkid,sock,command,session,uid,user,flag,(int)hall->time_now);
				//writeFile(LOGIN_PATH,mybuf);
				if (ret == 1)
				{
					//正在游戏中 回到大厅
					//printf("#####write to hall...\n");
					hall->user[user].status = 2;
					hall->user[user].forkid = -1;
					hall->user[user].socket = -2;
					hall->user[user].time = hall->time_now;
					write_fd(hall->fork[forkid].fd[1],sock);
					__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);
					return;
				}
				continue;
				//
			}
			timeout = 0;
			//基本命令集
			ret = BaseCommand(forkid,user,sock,command,buf);
			if (ret == 1000)
			{
				return;
			}
			else if (ret < 0)
			{
				flag_switch = 1;
				break;
			}
			else if (ret != 2000)
			{
				continue;
			}
			//
			//各游戏命令
			flag_switch = 0;
			//printf("gameid = %d\n",hall->fork[forkid].gameid);
			switch (hall->fork[forkid].gameid)
			{
				case 0:
					//百家乐
					if (BJLCommand(0,user,command,forkid,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 1:
					//炸金花
					if (ZJHNormalCommand(forkid,user,command,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 2:
					//牛牛
					if (OXCommand(forkid,user,command,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 4:
					if (BJLCommand(1,user,command,forkid,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 5:
					if (WZQCommand(forkid,user,command,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 6:
					if (MJCommand(forkid,user,command,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				default:
					flag_switch = 1;
					break;
			}

			if (flag_switch)
			{
				break;
			}
			//
		}//end flag - else
		//
		//
	}
	__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);

	if (user < 0)
	{
		getUserBySocket(forkid,sock,&user);
	}

	cleanUser(user);
	//
}

int BaseCommand(int forkid,int user,int sock,int command,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	long long int gold;
	int length;
	int hds,hds_tmp,wforkid,wdesk;
	short int gid,type;
	short int flag,grade,seat_max,ip;
	short int round;
	short int model,kan;
	short int mj_change,mj_zm,mj_times,mj_dg,mj_yjjd,mj_mqzz,mj_td;

	switch (command)
	{
		case 1006:
			//创建私人房
			if (hall->user[user].hds >= 0)
			{
				//已入桌
				return 0;
			}
			
			

			recv_int16_from(buf+17,&gid);
			recv_int16_from(buf+19,&type);
			recv_int16_from(buf+21,&grade);
			recv_int16_from(buf+23,&flag);
			recv_long64_from(buf+25,&gold);
			recv_int16_from(buf+33,&ip);
			recv_int16_from(buf+35,&seat_max);

			switch (gid)
			{
				case 1:
					//炸金花
					
					recv_int16_from(buf+37,&round);
					if (createZJHDesk(user,gid,type,grade,flag,gold,ip,seat_max,round) <= 0)
					{
						return 0;
					}
					break;
				case 2:
					//牛牛
					
					recv_int16_from(buf+37,&model);
					recv_int16_from(buf+39,&kan);
					if (createOXDesk(user,gid,type,grade,flag,gold,ip,seat_max,model,kan) <= 0)
					{
						return 0;
					}
					break;
				case 5:
					//五子棋
					if (createWZQDesk(user,gid,type,grade) <= 0)
					{
						return 0;
					}
					break;
				case 6:
					
					recv_int16_from(buf+37,&mj_change);
					recv_int16_from(buf+39,&mj_zm);
					recv_int16_from(buf+41,&mj_times);
					recv_int16_from(buf+43,&mj_dg);
					recv_int16_from(buf+45,&mj_yjjd);
					recv_int16_from(buf+47,&mj_mqzz);
					recv_int16_from(buf+49,&mj_td);
					if (createMJDesk(user,gid,type,grade,flag,gold,ip,seat_max,mj_change,mj_zm,mj_times,mj_dg,mj_yjjd,mj_mqzz,mj_td) <= 0)
					{
						return 0;
					}
					break;
				default:
					return 0;
			}
			

			if (hall->user[user].status != 10)
			{
				return -1;
			}
			hall->user[user].status = 2;
			hall->user[user].forkid = -1;
			hall->user[user].socket = -2;
			hall->user[user].time = hall->time_now;
			write_fd(hall->fork[forkid].fd[1],sock);
			__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);
			return 1000;
		case 1011:
			//退出房间至大厅
			if (hall->user[user].hds >= 0)
			{
				return 0;
			}

			//printf("user = %d user.status=%d\n",user,hall->user[user].status);
			if (hall->user[user].status != 10)
			{
				return -1;
			}
			//
			hall->user[user].status = 2;
			hall->user[user].forkid = -1;
			hall->user[user].socket = -2;
			hall->user[user].time = hall->time_now;
			write_fd(hall->fork[forkid].fd[1],sock);
			__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);
			return 1000;
		case 1016:
			//金币下载
			return getUserGoldFromRedis(user,1);
		case 1018:
			//房间列表
			recv_int16_from(buf+17,&gid);
			recv_int16_from(buf+19,&type);
			//printf("1018 gid=%d,type=%d\n",gid,type);
			length = pack2018(buf,gid,type);
			//printf("2018 length = %d\n",length);
			return UserSend(user,buf,length);
		case 1019:
			recv_int32_from(buf+17,&hds);
			hds_tmp = hds;
			hds = getHDS(hds);
			wforkid = hds/1000;
			wdesk = (hds%1000)/10;
			//printf("hds = %d,hds_tmp = %d\n",hds,hds_tmp);
			if (wforkid < 0 || wforkid >= hall->normal_num || wdesk < 0 || wdesk >= DESK_MAX || hall->fork[wforkid].zjh[wdesk].desk_status == 0)
			{
				//不存在的桌子
				length = pack2019(buf,0,0,0,0,hds_tmp);
				return UserSend(user,buf,length);
			}
			
			length = pack2019(buf,1,hall->fork[wforkid].gameid,hall->fork[wforkid].type,hall->fork[wforkid].grade,hds_tmp);
			return UserSend(user,buf,length);
		case 1030:
			recv_long64_from(buf+17,&gold);
			return User_order(user,gold);
		case 1060:
			recv_int16_from(buf+17,&type);
			recv_long64_from(buf+19,&gold);
			return User_stock(user,type,gold);
		case 999:
			//心跳
			length = pack999(buf);
			return UserSend(user,buf,length);
		default:
			break;
	}
	return 2000;
}
/*
void epollproc(struct epoll_event *events)
{
	//to do ...
	int i,err,nfds = -1,sock=0;
	char send_buf[SEND_BUF_MAX]="";
	char recv_buf[RECV_BUF_MAX]="";
	struct epoll_event ev;
	while(1)
	{
	    nfds = epoll_wait(epfd,events,EPOLL_SIZE_MAX,3000);
	   // printf("nfds = %d time=%d\n",nfds,time(NULL));
	    if(nfds < 0)
	    {
		if(errno == EINTR) continue;
		printf("nfds=%d errno=%d err=%s\n",nfds,errno,strerror(errno));
		exit(0);
	    }
	    
	    for(i=0;i<nfds;i++)
	    {
		if(events[i].events&EPOLLIN)
		{
			if((sock = events[i].data.fd) < 0) continue;
		}

		bzero(send_buf,SEND_BUF_MAX);
		err = recv(sock,recv_buf,64,0);
		if (err <= 0)
		{
			//printf("epoll %d sock err = %d\n",i,err);
			events[i].data.fd = -1;
			ev.data.fd=sock;
			ev.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_DEL,sock,&ev);
			continue;
		}
		else if(err < 4)
		{
			//非法
			printf("非法 recv_buf=[%s]\n",recv_buf);
			events[i].data.fd = -1;
			ev.data.fd=sock;
			ev.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_DEL,sock,&ev);
			continue;
		}
		else if(err == 64)
		{
			events[i].data.fd = -1;
			ev.data.fd=sock;
			ev.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_DEL,sock,&ev);
			close(sock);
			continue;
		}
		else
		{
			if(recv_buf[err-2] == '\r')
			{
				//window
				recv_buf[err-2] = '\0';
				printf("epoll window recv_buf = [%s] pid = %d\n",recv_buf,getpid());	
				sprintf(send_buf,"hello window! epoll\r\n");
				send(sock,send_buf,strlen(send_buf),0);
			}
			else if(recv_buf[err-1] == '\n')
			{
				//linux
				recv_buf[err-1] = '\0';
				printf("epoll linux recv_buf = [%s] pid = %d\n",recv_buf,getpid());	
				sprintf(send_buf,"hello linux! epoll\n");
				send(sock,send_buf,strlen(send_buf),0);
			}
			else
			{
				continue;
			}
			events[i].data.fd = -1;
			ev.data.fd=sock;
			ev.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_DEL,sock,&ev);
			close(sock);
		}
		
	    }
	}
}*/
