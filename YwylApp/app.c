#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <signal.h>
#include <strings.h>
#include <pthread.h>
#include <time.h>
#include "appproc.h"
//tcp 协议
//	'|'	+	length(int)	+	command(int)	+	session(int)	+	内容
extern HALL *hall;

static	void	sig_child(int signo)
{
	pid_t	pid;
	int		stat;
	while((pid=waitpid(-1,&stat,WNOHANG))>0);
	return;
}

/*初始化守护进程*/
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

void AcceptThread(int * s_sock)
{
	int sd[128],sd_i=0;
	struct sockaddr_in cli_addr;
	pthread_t thid;
	socklen_t cli_len;
	if(0 != pthread_detach(pthread_self())) return;
	//cli_len = sizeof(struct sockaddr_in);
	while(hall->flag)
	{
		sd[sd_i]=accept(*s_sock,(struct sockaddr*)&cli_addr,&cli_len);	
		if(sd[sd_i] < 0)
		{
			printf("Thid: %d accept client failed",(int)pthread_self());
			continue;
		}
		printf("Thid: %lu Accept: %d IP: %s thread=%d\n",pthread_self(),sd[sd_i],inet_ntoa(cli_addr.sin_addr),hall->thread);
		
		if (hall->thread < HALL_USER_MAX)
		{
			if (pthread_create(&thid,NULL,(void *)HallFirstThread,&sd[sd_i]) != 0)
			{
				printf("create thread error!\n");
				close(sd[sd_i]);
				continue;
			}
			__sync_fetch_and_add(&(hall->thread),1);
			//__sync_fetch_and_add(&(hall->thread_total),1);
		}
		else
		{
			//大厅爆满
			close(sd[sd_i]);
		}
		
		/*
		write_fd(hall->fork[0].fd[0],c_sock);
		close(c_sock);*/
		
		sd_i++;
		if (sd_i >= 128) sd_i = 0;
	}
}

void UpdatePool()
{
	int i;
	long long int pool_mix,pool_new;
	while (1)
	{
		pool_new = 0;
		pool_mix = hall->pool-hall->pool_init;
		i = update_game_pool(pool_mix,&pool_new);
		if (i == 1)
		{
			//ok
			if (pool_mix <= 0)
			{
				__sync_fetch_and_add(&(hall->pool_init),pool_mix);
				pool_mix = pool_new - hall->pool_init;
				if (pool_mix > 0)
				{
					__sync_fetch_and_add(&(hall->pool),pool_mix);
					__sync_fetch_and_add(&(hall->pool_init),pool_mix);
				}
			}
		}
		else if (i == -2)
		{
			//select no
			if (pool_mix < 0)
			{
				__sync_fetch_and_add(&(hall->pool_init),pool_mix);
			}
		}

		sleep(30);
	}
}

void CheckThread()
{
	char tmp[32];
	char *buf = (char *)malloc(BUF_MAX);
	//unsigned int times=0;
	int i;
	while (1)
	{
		for (i=0;i<USER_MAX;i++)
		{
			if (hall->user[i].status != 0 && hall->user[i].socket < 0 && hall->time_now - hall->user[i].time >= TIME_OUT*4/10)
			{
				if (hall->user[i].hds < 0)
				{
					//未入桌
					cleanUser(i);
					continue;
				}
				
				if (hall->fork[hall->user[i].hds/1000].gameid != 5)
				{
					//非五子棋
					cleanUser(i);
				}
			}
		}

		memset(buf,'\0',BUF_MAX);
		for (i=0;i<hall->normal_num;i++)
		{
			memset(tmp,'\0',32);
			sprintf(tmp,"%d:%d:%d:%d\n",hall->fork[i].gameid,hall->fork[i].grade,i,hall->fork[i].thread);
			strcat(buf,tmp);
		}
		writeToFile(WEB_ROOT,buf);
		/*
		memset(buf,'\0',BUF_MAX);
		for (i=0;i<USER_MAX;i++)
		{
			if (hall->user[i].forkid >= 0 && hall->fork[hall->user[i].forkid].gameid == 0)
			{
				//百家乐
				memset(tmp,'\0',32);
				sprintf(tmp,"%d:%s:%s:%lld\n",hall->user[i].uid,hall->user[i].name,hall->user[i].url,hall->user[i].money);
				strcat(buf,tmp);
			}
		}

		writeToFile(WEB_ROOT_BJL,buf);
		
		memset(buf,'\0',BUF_MAX);
		for (i=0;i<USER_MAX;i++)
		{
			if (hall->user[i].forkid >= 0 && hall->fork[hall->user[i].forkid].gameid == 4)
			{
				//推筒子
				memset(tmp,'\0',32);
				sprintf(tmp,"%d:%s:%s:%lld\n",hall->user[i].uid,hall->user[i].name,hall->user[i].url,hall->user[i].money);
				strcat(buf,tmp);
			}
		}
		writeToFile(WEB_ROOT_TTZ,buf);
		*/
		sleep(10);
		//times++;
	}
	
}

int main(int argc, char *argv[])
{
	int one=0,err=0,i,j,k;
	init_myshm(0);
	
	/*
	short int cards[108];
	short int val[108];
	short int card_max;
	initCards(cards,108);
	card_max = 108;
	for (i=0;i<card_max;i++)
	{
		printf("%d,",cards[i]);
	}
	printf("\n");
	getMJCards(16,cards,&card_max,val);
	for (i=0;i<card_max;i++)
	{
		printf("%d,",cards[i]);
	}
	printf("\n");
	for (i=0;i<16;i++)
	{
		printf("%d,",val[i]);
	}
	printf("\n");
	return 0;*/
	/*
	printf("bjl flag=%d,start_time=%d,now=%d\n",hall->bjl[0].flag,(int)hall->bjl[0].start_time,(int)hall->time_now);
	//printf("bjl cards=%d,%d,%d\n",hall->bjl[0].card[0][0],hall->bjl[0].card[0][1],hall->bjl[0].card[0][2]);
	print_card(hall->bjl[0].banker_card);
	for (i=0;i<4;i++)
	{
		print_card(hall->bjl[0].card[i]);
	}*/
	printf("time_now=%d\n",(int)hall->time_now);
	printf("----------------desk info---------------\n");
	for (i=0;i<FORK_MAX;i++)
	{
		for (j=0;j<DESK_MAX;j++)
		{
			if (hall->fork[i].zjh[j].people != 0 || hall->fork[i].zjh[j].desk_status != 0)
			{
				printf("fork%d->desk%d->seat_max=%d->people=%d->player=%d->flag=%d->desk_status=%d->time=%d->card_num=%d->turn=%d\n",i,j,hall->fork[i].zjh[j].seat_max,hall->fork[i].zjh[j].people,hall->fork[i].zjh[j].player,hall->fork[i].zjh[j].flag,hall->fork[i].zjh[j].desk_status,(int)(hall->time_now-hall->fork[i].zjh[j].start_time),hall->fork[i].zjh[j].card_num,hall->fork[i].zjh[j].turn);
				for (k=0;k<SEAT_MAX;k++)
				{
					if (hall->fork[i].zjh[j].seat[k].user >= 0)
					{
						if (hall->fork[i].gameid == 2)
						{
							err = check_type_ox(hall->fork[i].zjh[j].seat[k].card,hall->fork[i].zjh[j].model,hall->fork[i].zjh[j].kan);
						}
						printf("fork%d->desk%d->seat%d:user=%d,user.status=%d,uid=%d,status=%d,login=%d,hds=%d,timeout=%d,in=%d,out=%d,begin_time=%d,flag_win=%d,flag_peng=%d,flag_gang=%d,lack=%d,ranking=%d,num_gang=%d,num_peng=%d\n",i,j,k,hall->fork[i].zjh[j].seat[k].user,hall->user[hall->fork[i].zjh[j].seat[k].user].status,hall->user[hall->fork[i].zjh[j].seat[k].user].uid,hall->fork[i].zjh[j].seat[k].status,hall->user[hall->fork[i].zjh[j].seat[k].user].login,hall->user[hall->fork[i].zjh[j].seat[k].user].hds,err,hall->fork[i].zjh[j].seat[k].card_in,hall->fork[i].zjh[j].seat[k].card_out,(int)hall->fork[i].zjh[j].seat[k].begin_time,hall->fork[i].zjh[j].seat[k].flag_win,hall->fork[i].zjh[j].seat[k].flag_peng,hall->fork[i].zjh[j].seat[k].flag_gang,hall->fork[i].zjh[j].seat[k].lack,hall->fork[i].zjh[j].seat[k].ranking,hall->fork[i].zjh[j].seat[k].num_gang,hall->fork[i].zjh[j].seat[k].num_peng);
						//print_card(hall->fork[i].zjh[j].seat[i].card);
						printf("card_num=%d\n",hall->fork[i].zjh[j].seat[k].card_num);
						for (err=0;err<hall->fork[i].zjh[j].seat[k].card_num;err++)
						{
							printf("%d,",hall->fork[i].zjh[j].seat[k].card[err]);
						}
						printf("\n");
						/*printf("card_out_num=%d\n",hall->fork[i].zjh[j].seat[k].card_out_num);
						for (err=0;err<hall->fork[i].zjh[j].seat[k].card_out_num;err++)
						{
							printf("%d,",hall->fork[i].zjh[j].seat[k].card_out_list[err]);
						}
						printf("\n");
						for (err=0;err<hall->fork[i].zjh[j].seat[k].num_gang;err++)
						{
							for (m=0;m<3;m++)
							{
								printf("lose seat->%d:gold->%lld\n",hall->fork[i].zjh[j].seat[k].gang[err].seat[m],hall->fork[i].zjh[j].seat[k].gang[err].gold[m]);
							}
						}
						printf("\n");*/
					}
				}
				printf("\n");
			}
			
		}
	}

	printf("----------------user info---------------\n");
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].status != 0)
		{
			printf("[%s]user:%d fork=%d socket=%d status=%d uid=%d hds=%d createHDS=%d login=%d money=%lld house=%lld,timeout=%d\n",hall->user[i].ip,i,hall->user[i].forkid,hall->user[i].socket,hall->user[i].status,hall->user[i].uid,hall->user[i].hds,createHDS(hall->user[i].hds),hall->user[i].login,hall->user[i].money,hall->user[i].stock,(int)(hall->time_now - hall->user[i].time));
		}
		
	}

	printf("----------------bjl info---------------\n");
	for (i=0;i<2;i++)
	{
		printf("bjl->%d flag=%d time=%d banker=%d msg_list_flag=%d\n",i,hall->bjl[i].flag,(int)(hall->time_now - hall->bjl[i].start_time),hall->bjl[i].banker,hall->bjl[i].msg_list_flag);

		if (hall->bjl[0].banker >= 0)
		{
			printf("----------------bjl->%d banker=%d uid = %d banker_flag=%d---------------\n",i,hall->bjl[i].banker,hall->user[hall->bjl[i].banker].uid,hall->user[hall->bjl[i].banker].banker_flag[i]);
		}

	}
	
	for (i=0;i<8;i++)
	{
		printf("bjl->0 seat%d -> user%d uid=%d user.status=%d hds=%d\n",i,hall->bjl[0].seat[i].user,hall->user[hall->bjl[0].seat[i].user].uid,hall->user[hall->bjl[0].seat[i].user].status,hall->user[hall->bjl[0].seat[i].user].hds);
	}
	printf("\n");
	for (i=0;i<8;i++)
	{
		printf("bjl->1 seat%d -> user%d uid=%d user.status=%d hds=%d\n",i,hall->bjl[1].seat[i].user,hall->user[hall->bjl[1].seat[i].user].uid,hall->user[hall->bjl[1].seat[i].user].status,hall->user[hall->bjl[1].seat[i].user].hds);
	}

	for (i=0;i<FORK_MAX;i++)
	{
		one += hall->fork[i].thread;
		if (hall->fork[i].thread > 0)
		{
			printf("fork %d => thread=%d\n",i,hall->fork[i].thread);
		}
	}
		
	for (i=0;i<hall->ip_num;i++)
	{
		printf("ip[%s]->%d\n",hall->myip[i].ip,hall->myip[i].num);
	}
	printf("myip_num = %d-%d%d%d%d\n",hall->ip_num,hall->bjl[0].win[0],hall->bjl[0].win[1],hall->bjl[0].win[2],hall->bjl[0].win[3]);
	printf("online total people = %d\n",one);
	//42 68 94
	/*i = 94;
	j = 30;
	hall->fork[i].zjh[j].bottom = 100;
	hall->fork[i].zjh[j].seat_max = 5;
	hall->fork[i].zjh[j].people = 5;
	hall->fork[i].zjh[j].desk_status = 1;
	hall->fork[i].zjh[j].round_max = 3;
	hall->fork[i].zjh[j].model = 1;
	hall->fork[i].zjh[j].kan = 1;
	*//*
	//test ssc
	short int cards[52],max;
	short int card[3];
	for (i=0;i<100;i++)
	{
		initCards(cards,52);
		max = 52;
		getThreeCardsByType(6,cards,&max,card);
		print_card(card);
	}*/
	/*
	//test begin
	printf("test start\n");
	init_myshm(0);
	printf("myshm init ok\n");
	int session = createSession(3);
	printf("session = %d\n",session);
	i = userLogin("cd6b8341a4078c08d9efedb82e09aac9",getUserBySession(session));
	printf("login i = %d\n",i);
	User_SSC(0,getUserBySession(session),"cd6b8341a4078c08d9efedb82e09aac9",11061029,6,30000);
	return 0;
	//test end
	*/
	if (argc > 1)
	{
		if (strcmp(argv[1],"start") == 0)
		{
			printf("app system start ...\n");
			srand(time(NULL));
			sem_init(&redis_lock,0,1);
			redis_flag = 0;

			for (i=0;i<MYSQL_MAX;i++)
			{
				sem_init(&mysql_lock[i],0,1);
				mysql_flag[i] = 0;
			}

			init_myshm(1);
			printf("init myshm ok!\n");
			hall->flag = 1;
			//后台数据统一初始化
			if (init_house_config() < 0)
			{
				printf("init house config error!\n");
				return 0;
			}
			
			if (init_game_pool() < 0)
			{
				printf("init game pool error!\n");
				return 0;
			}
		}
		else if (strcmp(argv[1],"stop") == 0)
		{
			init_myshm(0);
			//关闭倒计时
			hall->flag = 2;
			hall->time_over = hall->time_now + 600;
			printf("600秒后关闭服务器设置完成\n");
			return 0;
		}
		else
		{
			printf("./app start or stop? ...\n");
			return 0;
		}
	}
	else 
	{
		printf("./app start or stop? ...\n");
		return 0;
	}
	
	
	char tmp[32];
	int listen_sd,sone;

	pthread_t thid,ahid,bhid;
	
	struct sockaddr_in sa_serv;
	daemon_init();
	signal(SIGCHLD,sig_child);
	signal(SIGPIPE,SIG_IGN);
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);   
//	CHK_ERR(listen_sd, "socket");
	sone = 1;
	if (setsockopt(listen_sd,SOL_SOCKET,SO_REUSEADDR,(char *) &sone, sizeof(int)) < 0) return 0;
	//one = 1;
	//if (setsockopt(listen_sd,IPPROTO_TCP,TCP_NODELAY,(char *) &one, sizeof(int)) < 0) return 0;
	//printf("TcpServer: failed on setsockopt(): %s\n",strerror(errno));
	memset (&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (MYPORT);          
	err = bind(listen_sd, (struct sockaddr*) &sa_serv,sizeof (sa_serv));   
	//CHK_ERR(err, "bind");
	err = listen (listen_sd, 20);
	if (err < 0) return -1;
    //CHK_ERR(err, "listen");
	sprintf(tmp,"port X8888 listen OK!");
	printf("%s\n",tmp);
	writeFile(LOG_PATH,tmp);	
	if (0 != (pthread_create(&thid,NULL,(void *)AcceptThread,&listen_sd)))
	{
		printf("create accept thread listen failed!\n");
		exit(0);
	}

	if (0 != (pthread_create(&ahid,NULL,(void *)CheckThread,NULL)))
	{
		printf("create Check thread failed!\n");
		exit(0);
	}

	if (0 != (pthread_create(&bhid,NULL,(void *)UpdatePool,NULL)))
	{
		printf("create Check thread failed!\n");
		exit(0);
	}

	while(hall->flag)
	{
		if (hall->flag == 2)
		{
			if (hall->time_now > hall->time_over)
			{
				break;
			}
		}
		else
		{
			for (i = 0;i < FORK_MAX;i++)
			{
				if (hall->fork[i].status == 0 || (hall->fork[i].child_pid > 0 && getpgid(hall->fork[i].child_pid) <= 0 ))
				{
					if (hall->fork[i].status != 0)
					{
						sprintf(tmp,"fork%d restart!",i);
						writeFile(LOG_PATH,tmp);	
						close(hall->fork[i].fd[0]);
						close(hall->fork[i].fd[1]);
						kill(hall->fork[i].child_pid,9);
					}
					//printf("fork%d start\n",i);
					childMake(i);
				}
			}
		}
		//check fork
		//
		sleep(1);
		hall->time_now = time(NULL);
	}
	//最后同步一次用户金币
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].status == 10)
		{
			updateUserGold(i);
		}
	}

	//最后同步彩池
	long long int pool_new;
	i = update_game_pool(hall->pool-hall->pool_init,&pool_new);
	if (i!=1)
	{
		sprintf(tmp,"pool=%lld,pool_init=%lld,i=%d",hall->pool,hall->pool_init,i);
		writeFile(LOG_PATH,tmp);
	}
	sleep(10);
	//系统关闭
	for (i = 0;i < FORK_MAX;i++)
	{
		if (hall->fork[i].status != 0)
		{
			sprintf(tmp,"fork%d stop!",i);
			writeFile(LOG_PATH,tmp);	
			close(hall->fork[i].fd[0]);
			close(hall->fork[i].fd[1]);
			kill(hall->fork[i].child_pid,9);
		}
	}
	//
	return 0;
}
