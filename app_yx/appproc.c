#include "appproc.h"

extern int epfd;
extern HALL *hall;
//pthread_mutex_t mutex;

void* ClientThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	childproc(i);
	close(i);
	return NULL;
}

void* HallFirstThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	sendSession(i);
	close(i);
	__sync_fetch_and_sub(&(hall->thread),1);
	return NULL;
}

void* HallClientThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	hallClientproc(i);
	close(i);
	__sync_fetch_and_sub(&(hall->thread),1);
	return NULL;
}

void* MasterThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	free(arg);

	//printf("master thread i = %d\n",i);
	masterproc(i);
	
	exit(0);
	//return NULL;
}

void* BJLThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	if (hall->fork[i].gameid == 13)
	{
		HBDealer(i);
	}
	else
	{
		bjlDealer(i);
	}
	
	exit(0);
	//return NULL;
}

void* WZQThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	wzqDealer(i);
	exit(0);
	//return NULL;
}

void* SRCThread(void *arg)
{
	if(pthread_detach(pthread_self()) != 0){
		return NULL;
	}
	int i = (int)(*(int*)arg);
	
	srcDealer(i);
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
	int sd[1024];
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
	if (hall->fork[forkid].grade == 100)
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
	else if (hall->fork[forkid].grade == PRIVATE_HOUSE)
	{
		//私人场
		pthread_t src;
		if (pthread_create(&src,NULL,(void *)SRCThread,&forkid) != 0)
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
		if (sd_i >= 1024) sd_i = 0;
	}
	return 0;
}

void masterproc(int forkid)
{
	//printf("master proc forkid = %d\n",forkid);
	int sd[1024],sd_i=0,one;
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
		if (sd_i >= 1024) sd_i = 0;
	}
}

//	'|'	+	length(int)	+	command(int)	+	session(int)	+	内容
void sendSession(int sock)
{
	//int i,site[BJL_GAME_MAX];
	int session,forkid,user=-1,length,timeout=0;
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
			printf("socket is broken!\n");
			return;
		}
		else if (length == 0)
		{
			timeout++;
			//printf("time out = %d\n",timeout);
			if (timeout >= HALL_TIME_OUT)
			{
				printf("time out over\n");
				length = pack666(buf,0);
				App_Send(sock,buf,length);
				return;
			}

			if (user == 0)
			{
				sleep(1);
				continue;
			}
		}
		else
		{
			//printf("[%s]\n",buf+17);
			if (strcmp(buf+17,CONNECT_CHECK_TEST) == 0)
			{
				sprintf(buf,"Y");
				length = App_Send(sock,buf,strlen(buf));
				if (length <= 0)
				{
					printf("socket is broken!\n");
					return;
				}
				user = 0;
				timeout = 0;
				continue;
			}
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
		//sprintf(buf,"check ip[%s] error!",ip);
		//writeFile("/root/ywyl_error.log",buf);
		return;
	}
	
	session = createSession(sock,ip);
	user = getUserBySession(session);

	length = pack1000(buf,session);
	if (App_Send(sock,buf,length) < 0)
	{
		//sprintf(buf,"App_Send socket=%d error!",sock);
		//writeFile("/root/ywyl_error.log",buf);
		cleanUser(buf,user);
		return;
	}
	//printf("send session to sock %d session=%d user=%d gold=%lld money=%lld ...\n",sock,session,user,hall->user[user].gold,hall->user[user].money);

	forkid = selectForkid(user,BJL_DEFAULT,1,100,0);
	if (forkid < 0)
	{
		//printf("select forkid = %d\n",forkid);
		//全部服务器房间爆满
		//length = pack2002(buf,0,0,0,0);
		//App_Send(sock,buf,length);
		sprintf(buf,"selectForkid forkid=%d error!",forkid);
		writeFile("/root/ywyl_error.log",buf);
		cleanUser(buf,user);
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
			getUserBySocket(-1,sock,&user);
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
				getUserBySocket(-1,sock,&user);
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
				getUserBySocket(-1,sock,&user);
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
							forkid = selectForkid(user,BJL_DEFAULT,1,100,0);
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
	cleanUser(buf,user);
	//
}

int userLogin(char *token,int *user,int *session)
{
	if (hall->flag != 1 || *user < 0 || *user >= USER_MAX)
	{
		return -1;
	}
	//char logbuf[1024];
	char passwd[PASSWD_MAX];
	char name[USER_NAME_MAX];
	char url[USER_HEAD_URL];
	char tmp_ip[IP_MAX];
	int i,uid,tmp_user=-1,agent[AGENT_MAX],group[GROUP_MAX];
	short int sex,group_num,agent_num;
	long long int gold,money,stock;

	//get uid to do
	//redis
	uid = getUserByToken(token,&gold,&money,&stock,name,url,&sex,passwd,&group_num,group,&agent_num,agent);
	if (uid <= 0)
	{
		//登录失败
		printf("redis get user error!\n");
		return -1;
	}

	tmp_user = getUser(uid);
	printf("user login uid=%d,user=%d,tmp_user=%d\n",uid,*user,tmp_user);
	if (tmp_user < 0)
	{
		//sprintf(logbuf,"user=%d,tmp_user=%d,uid=%d",user,tmp_user,uid);
		//writeFile("/root/login.log",logbuf);
	}
	else 
	{
		//sprintf(logbuf,"user=%d,tmp_user=%d,uid=%d,tmp_user.uid=%d,tmp_user.status=%d",user,tmp_user,uid,hall->user[tmp_user].uid,hall->user[tmp_user].status);
		//writeFile("/root/login.log",logbuf);
		if (hall->user[tmp_user].uid == uid)
		{
			//printf("1111user=%d,tmp_user=%d,user.sock=%d,tmp_user.sock=%d\n",(*user),tmp_user,hall->user[(*user)].socket,hall->user[tmp_user].socket);
			sprintf(tmp_ip,hall->user[tmp_user].ip);
			
			hall->user[tmp_user].socket = hall->user[(*user)].socket;
			sprintf(hall->user[tmp_user].ip,hall->user[(*user)].ip);
			sprintf(hall->user[tmp_user].passwd,passwd);
			hall->user[tmp_user].status = 10;
			(*session) = ((int)(hall->user[tmp_user].time%100))*100000 + tmp_user;
			(*session) ^= SESSION_KEY;
			//printf("2222user=%d,tmp_user=%d,user.sock=%d,tmp_user.sock=%d\n",(*user),tmp_user,hall->user[(*user)].socket,hall->user[tmp_user].socket);
			cleanIP(tmp_ip);
			init_user((*user));
			
			(*user) = tmp_user;
			return 0;
		}
	}
	
	//未登录
	hall->user[(*user)].uid = uid;
	hall->user[(*user)].status = 10;
	hall->user[(*user)].login = 1;
	hall->user[(*user)].sex = sex;
	hall->user[(*user)].gold_init = gold;
	hall->user[(*user)].gold = gold;
	hall->user[(*user)].money_init = money;
	hall->user[(*user)].money = money;
	hall->user[(*user)].stock = stock;
	sprintf(hall->user[(*user)].name,name);
	sprintf(hall->user[(*user)].url,url);
	hall->user[(*user)].group_num = group_num;
	hall->user[(*user)].agent_num = agent_num;

	sprintf(hall->user[(*user)].passwd,passwd);

	for (i=0;i<group_num;i++)
	{
		hall->user[(*user)].group[i] = group[i];
	}

	for (i=0;i<agent_num;i++)
	{
		hall->user[(*user)].agent[i] = agent[i];
	}

	if (hall->site_msg-1 < 0)
	{
		hall->user[(*user)].msg_id = hall->aaa_msg[MSG_LIST_MAX-1].id;
	}
	else
	{
		hall->user[(*user)].msg_id = hall->aaa_msg[hall->site_msg-1].id;
	}
	
	sem_wait(&(hall->user[(*user)].lock_user_msg));
	hall->user[(*user)].user_msg_num = 0;
	for (i=0;i<MSG_LIST_MAX;i++)
	{
		hall->user[(*user)].user_msg[i].length = 0;
		memset(hall->user[(*user)].user_msg[i].msg,'\0',MSG_BUF_MAX);
	}
	sem_post(&(hall->user[(*user)].lock_user_msg));
	return 0;
}

void childproc(int sock)
{
	int group[GROUP_MAX],agent[AGENT_MAX];
	short int group_num,agent_num;
	int i,j,k,forkid,user,flag,timeout,ret,flag_switch=0;
	int command,session,uid,length;
	char buf[RECV_BUF_MAX];
	//char mybuf[RECV_BUF_MAX];
	//char logbuf[64];
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
			printf("fork%d client socket %d broken flag=%d user=%d...\n",forkid,sock,flag,user);
			getUserBySocket(forkid,sock,&user);
			break;
		}
		else if (flag == 0)
		{
			//无消息
			timeout++;
			/*if (timeout%20 == 0)
			{
				printf("fork%d client %d timeout = %d ...\n",forkid,sock,timeout);
			}*/
			if (timeout >= (TIME_OUT-200))
			{
				//超时 断开
				printf("time out ... kill socket %d\n",sock);
				getUserBySocket(forkid,sock,&user);
				break;
			}
			
			getUserBySocket(forkid,sock,&user);
			if (user < 0)
			{
				//未验证用户
				continue;
			}
			//
			for (i=0;i<BJL_GAME_MAX;i++)
			{
				if (hall->user[user].banker_flag[i] != 0)
				{
					//
					if (timeout >= 50)
					{
						break;
					}
				}
			}

			if (i != BJL_GAME_MAX)
			{
				break;
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

			//聊天消息
			if (hall->user[user].user_msg_num > 0)
			{
				sem_wait(&(hall->user[user].lock_user_msg));
				j = hall->user[user].user_msg_num;
				for (i=0;i<j;i++)
				{
					if (hall->user[user].user_msg[i].length > 0)
					{
						//printf("user=%d user_msg_length = %d\n",user,hall->user[user].user_msg[i].length);
						recv_int16_from(hall->user[user].user_msg[i].msg+13,&group_num);
						if (group_num == 10004)
						{
							//通知进群成功 刷新群信息
							if (getUserByUid(hall->user[user].uid,&group_num,group,&agent_num,agent) == 0)
							{
									hall->user[user].group_num = group_num;
									for (k=0;k<group_num;k++)
									{
										//printf("uid=%d,group=%d,k=%d\n",hall->user[user].uid,group[k],k);
										hall->user[user].group[k] = group[k];
									}

									hall->user[user].agent_num = agent_num;
									for (k=0;k<agent_num;k++)
									{
										hall->user[user].agent[k] = agent[k];
									}
							}
						}
						if (UserSend(user,hall->user[user].user_msg[i].msg,hall->user[user].user_msg[i].length) < 0)
						{
							//printf("user msg send error!\n");
							break;
						}
						hall->user[user].user_msg[i].length = 0;
						hall->user[user].user_msg_num--;
					}
				}
				sem_post(&(hall->user[user].lock_user_msg));
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
			
			User_order_check(buf,user);

			//金币mysql同步更新
			/*if (hall->time_now%30 == user%30)
			{
				updateUserGold(user,0,0,0,0);
			}*/
			
			//金币redis同步更新
			if (hall->user[user].msg_gold_update)
			{
				getUserGoldFromRedis(buf,user,0);
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
				case 10:
					//推筒子
					BJLTimeOut(forkid,2,user,buf);
					break;
				case 11:
					BJLTimeOut(forkid,3,user,buf);
					break;
				case 12:
					BJLTimeOut(forkid,4,user,buf);
					break;
				case 13:
					HBTimeOut(forkid,hall->fork[forkid].type,user,buf);
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
				getUserBySocket(forkid,sock,&user);
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
				
				ret = userLogin(buf+17,&user,&session);
				//printf("user login ret = %d\n",ret);
				if (ret < 0)
				{
					//登录失败
					cleanUser(buf,user);
					length = pack2001(buf,-1,0,0,0,0,-1,session);
					App_Send(sock,buf,length);
					__sync_fetch_and_sub(&(hall->fork[forkid].thread),1);
					//sprintf(buf,"childproc exit->fork=%d,socket=%d,user=%d,uid=%d --> login failed!!! %s",forkid,sock,user,uid,formatTime(logbuf));
					//writeFile(LOG_PATH,buf);
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

				printf("fork%d uid:%d login OK! hds=%d,ret=%d\n",forkid,hall->user[user].uid,hall->user[user].hds,ret);
				//
				length = pack2001(buf,1,(short)ret,hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,createHDS(hall->user[user].hds/10*10),session);
				if (UserSend(user,buf,length) < 0)
				{
					printf("User Send error!\n");
					break;
				}
				getUserGoldFromRedis(buf,user,0);

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
				case 10:
					if (BJLCommand(2,user,command,forkid,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 11:
					if (BJLCommand(3,user,command,forkid,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 12:
					if (BJLCommand(4,user,command,forkid,buf) < 0)
					{
						flag_switch = 1;
					}
					break;
				case 13:
					if (HBCommand(hall->fork[forkid].type,user,command,forkid,buf) < 0)
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
	
	if (user >= 0 && user < USER_MAX)
	{
		hall->user[user].time = hall->time_now;
	}
	cleanUser(buf,user);
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
				return -1;
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
					if (createZJHDesk(buf,user,gid,type,grade,flag,gold,ip,seat_max,round) <= 0)
					{
						return 0;
					}
					break;
				case 2:
					//牛牛
					
					recv_int16_from(buf+37,&model);
					recv_int16_from(buf+39,&kan);
					if (createOXDesk(buf,user,gid,type,grade,flag,gold,ip,seat_max,model,kan) <= 0)
					{
						return 0;
					}
					break;
				case 5:
					//五子棋
					if (createWZQDesk(buf,user,gid,type,grade) <= 0)
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
					if (createMJDesk(buf,user,gid,type,grade,flag,gold,ip,seat_max,mj_change,mj_zm,mj_times,mj_dg,mj_yjjd,mj_mqzz,mj_td) <= 0)
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
			if (hall->user[user].hds >= 0 || hall->thread >= HALL_USER_MAX)
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
			recv_int16_from(buf+17,&type);
			return getUserGoldFromRedis(buf,user,(int)type);
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
			//充值订单
			recv_long64_from(buf+17,&gold);
			return User_order(buf,user,gold);
		case 1060:
			//仓库操作
			recv_int16_from(buf+17,&type);
			recv_long64_from(buf+19,&gold);
			return User_stock(buf,user,type,gold);
		case 1083:
			//兑换
			recv_int16_from(buf+17,&type);
			recv_long64_from(buf+19,&gold);
			return User_exchange(buf,user,type,gold,buf+27);
		case 1090:
			//发起对某uid的聊天 
			recv_int32_from(buf+17,&hds);
			recv_int16_from(buf+21,&gid);
			recv_int16_from(buf+23,&type);
			recv_int16_from(buf+25,&grade);
			return User_chat(buf,user,hds,gid,type,grade,buf+27);
		case 1093:
			recv_int32_from(buf+17,&hds);
			recv_long64_from(buf+21,&gold);
			recv_int16_from(buf+29,&type);
			if (type < 0 || type >= 128)
			{
				return -1;
			}
			return User_trade(buf,user,hds,gold,buf+31,buf+31+type);
		case 999:
			//心跳
			length = pack999(buf);
			return UserSend(user,buf,length);
		default:
			break;
	}
	return 2000;
}

void srcDealer(int forkid)
{
	int i;
	char buf[RECV_BUF_MAX];
	while (1)
	{
		for (i=0;i<USER_MAX;i++)
		{
			if (hall->user[i].status != 0 && hall->user[i].status != 10 && hall->time_now - hall->user[i].time >= TIME_OUT*4/10)
			{
				if (hall->user[i].hds >= 0 && hall->user[i].hds/1000 == forkid)
				{
					//已入桌
					memset(buf,'\0',RECV_BUF_MAX);
					cleanUser(buf,i);
				}
			}
		}
		sleep(10);
	}
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
