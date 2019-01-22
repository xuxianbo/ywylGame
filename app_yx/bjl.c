#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "bjl.h"
#include "cards.h"
#include "tcppack.h"

extern HALL *hall;

int BJLTimeOut(int forkid,int index,int user,char *buf)
{
	int length;
	if (hall->user[user].bjl_balance[index] && hall->bjl[index].flag == 4)
	{
		hall->user[user].bjl_balance[index] = 0;
		//
		//游戏结算通知
		length = pack4021(forkid,index,buf,user,BJL_BET_MAX,BJL_SEAT_MAX,GOLD_BANKER);
		return UserSend(user,buf,length);
		//
	}
	return 0;
}

int BJLCommand(int index,int user,int command,int forkid,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	long long int gold=-1;
	short int seat=-1,type=-1;

	if (command != 1003)
	{
		if (hall->user[user].hds < 0 || hall->user[user].hds/1000 != forkid)
		{
			return 0;
		}
	}

	switch (command)
	{
		case 1003:
			//进入百家乐
			return bjlInDesk(buf,user,forkid);
		case 1015:
			//获取游戏初始化信息
			return bjlGameInfo(buf,forkid,index,user);
		case 1004:
			//离开桌子
			return bjlOutDesk(buf,index,user);
		case 1020:
			//请求入桌到指定座位
			recv_int16_from(buf+17,&seat);
			return bjlSeatDown(buf,index,user,seat-1);
		case 1021:
			//百家乐下注
			recv_int16_from(buf+17,&type);
			recv_long64_from(buf+19,&gold);
			return bjlOrder(buf,index,user,type,gold);
		case 1022:
			//请求上庄
			recv_long64_from(buf+17,&gold);
			return bjlWantBanker(buf,index,user);
		case 1023:
			//请求下庄
			return bjlGiveUpBanker(buf,index,user);
		case 1025:
			//请求上庄列表信息
			return bjlBankerList(buf,index,user);
		case 1026:
			return bjlUserMsg(buf,index,user);
		default:
			//未定义命令
			break;
	}
	return 0;
}
//百家乐荷官任务
void bjlDealer(int forkid)
{
	//return;
	int i=0,j,length,index;
	//long long int robot_want,robot_tmp,gold_max,bet_total;
	char buf[RECV_BUF_MAX];
	char myTime[64];
	char path[64];

	switch (hall->fork[forkid].gameid)
	{
		case 0:
			index = 0;
			break;
		case 4:
			index = 1;
			break;
		case 10:
			index = 2;
			break;
		case 11:
			index = 3;
			break;
		case 12:
			index = 4;
			break;
		default:
			return;
	}
	//printf("bjlDealer start...%d\n",index);
	sprintf(path,"/root/bjl_%d.log",index);

	while (1)
	{
		switch (hall->bjl[index].flag)
		{
			case 0:
				//游戏等待
				if (hall->time_now >= hall->bjl[index].start_time)
				{
					//printf("fork %d -> %d -> %d\n",forkid,forkid%hall->bjl[index].house_num,hall->bjl[index].house_num-1);
					if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
					{
						//刷新内存
						hall->bjl[index].banker_flag = 0;
						hall->bjl[index].gold_total = 0;
						//输赢，押注
						for (i=0;i<BJL_BET_MAX;i++)
						{
							hall->bjl[index].win[i] = 0;
							hall->bjl[index].gold[i] = 0;
							hall->bjl[index].gold_win[i] = 0;
							hall->bjl[index].robot_gold[i] = 0;
							//闲家手牌
							for (j=0;j<BJL_HAND_MAX;j++)
							{
								hall->bjl[index].card[i][j] = 0;
							}
						}
						//庄家手牌
						for (i=0;i<BJL_HAND_MAX;i++)
						{
							hall->bjl[index].banker_card[i] = 0;
						}
						//可视座位
						for (i=0;i<BJL_SEAT_MAX;i++)
						{
							if (hall->bjl[index].seat[i].user >= 0)
							{
								for (j=0;j<BJL_BET_MAX;j++)
								{
									hall->user[hall->bjl[index].seat[i].user].gold_win[index][j] = 0;
								}
							}
						}
						
						//大赢家列表
						for (i=0;i<WINNER_MAX;i++)
						{
							hall->bjl[index].win_max[i] = 0;
							hall->bjl[index].win_uid[i] = -1;
							memset(hall->bjl[index].win_name[i],'\0',USER_NAME_MAX);
							memset(hall->bjl[index].win_url[i],'\0',USER_HEAD_URL);
						}
						//robot
						//robot_want = myRandom()%(ROBOT_MAX-ROBOT_MIN) + ROBOT_MIN;
						//开奖
						//printf("是否开奖 index=%d\n",index);
						//sprintf(buf,"bjl set banker begin time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						//writeToFile(path,buf);
						
						bjlSetBanker(buf,index);

						//sprintf(buf,"bjl bjlDraw begin time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						//writeToFile(path,buf);
						
						while (bjlDraw(index) < 0)
						{
							//sprintf(buf,"bjl bjlDraw over time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
							//writeToFile(path,buf);
							//hall->bjl[index].start_time = hall->time_now + 999;
							printf("bjlDraw index = %d last < 0\n",index);
							sleep(1);
						}

						//sprintf(buf,"bjl bjlDraw end time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						//writeToFile(path,buf);
						//游戏开始
						hall->bjl[index].start_time = hall->time_now + TIME_BET;
						hall->bjl[index].flag = 1;
						//
						//sprintf(buf,"bjl game start and order begin...%d",(int)hall->time_now);
						//writeFile("/root/bjl.log",buf);
					}
					//
					//printf("bjl game start ...\n");
				}
				else
				{
					//printf("...%d\n",i++);
					//sleep(1);
				}
				break;
			case 1:
				//游戏开始可下注
				if (hall->time_now >= hall->bjl[index].start_time)
				{
					//游戏封盘
					if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
					{
						hall->bjl[index].start_time = hall->time_now + 999;
						hall->bjl[index].flag = 2;
						//sprintf(buf,"bjl game order end...%d",(int)hall->time_now);
						//writeFile("/root/bjl.log",buf);
					}
					//printf("bjl game order end...\n");
					//
				}
				else
				{
					
					if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
					{
						//printf("机器人每1秒下注一次...\n");
						//bjlCheckBankerList(index);
						robotOrder(index);
					}
					//sleep(1);
				}
				break;
			case 2:
				//开始开奖不可下注
				//
				length = pack6020(forkid,index,buf,BJL_BET_MAX);
				AllBjlSend(forkid,buf,length);
				//开奖
				if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
				{
					while (hall->bjl[index].banker>=0 && checkUid(hall->user[hall->bjl[index].banker].uid) >= 0 && hall->bjl[index].gold_total >= 10000 && bjlDraw(index) < 0)
					{
						sprintf(buf,"bjl bjlDraw over time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						writeToFile(path,buf);
						//hall->bjl[index].start_time = hall->time_now + 999;
						sleep(1);
					}
					//sprintf(buf,"bjl drawRecord begin time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					//writeToFile(path,buf);

					while (drawRecord(index) < 0)
					{
						sprintf(buf,"bjl drawRecord over time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						writeToFile(path,buf);
						sleep(1);
					}

					//sprintf(buf,"bjl drawRecord end time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					//writeToFile(path,buf);
					//保存开奖记录
					bjlRecord(index);

					//sprintf(buf,"bjl bjlRecord end time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					//writeToFile(path,buf);
					//开始结算
					//sleep(1);
					if (index == 2 || index == 3)
					{
						hall->bjl[index].start_time = hall->time_now + TIME_OVER - 3;
					}
					else if (index == 4)
					{
						hall->bjl[index].start_time = hall->time_now + TIME_OVER + 9;
					}
					else
					{
						hall->bjl[index].start_time = hall->time_now + TIME_OVER;
					}
					
					hall->bjl[index].flag = 3;
					//sprintf(buf,"bjl game balance end and show time...%d",(int)hall->time_now);
					//writeFile("/root/bjl.log",buf);
				}
				//
				break;
			case 3:
				//开始结算
				if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
				{
					while (bjlBalance(buf,forkid,index) != 1)
					{
						sleep(1);
					}

					//sprintf(buf,"bjl update_game_bjl_reward begin time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					//writeToFile(path,buf);
					//全部结算完成
					while (update_game_bjl_reward(index,hall->bjl[index].qh,hall->bjl[index].win_uid,hall->bjl[index].win_max,hall->bjl[index].win_name) < 0)
					{
						//sprintf(buf,"bjl update_game_bjl_reward over time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
						//writeToFile(path,buf);
						sleep(1);
					}

					//sprintf(buf,"bjl update_game_bjl_reward end time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					//writeToFile(path,buf);

					bjlCheckBankerList(index);
					hall->bjl[index].flag = 4;
					//设置座位广播
					for (i=0;i<hall->bjl[index].house_num;i++)
					{
						hall->bjl[index].flag_seat[i] = 1;
						hall->bjl[index].flag_over[i] = 1;
					}
				}
				break;
			case 4:
				//游戏结束结果展示期
				if (hall->time_now >= hall->bjl[index].start_time)
				{
					//展示完成->游戏等待
					if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
					{
						if (hall->flag != 1 && hall->time_now > hall->time_over - 300)
						{
							hall->bjl[index].start_time = hall->time_now + 999;
							hall->bjl[index].flag = 0;
							bjlSetBanker(buf,index);
						}
						else
						{
							hall->bjl[index].start_time = hall->time_now + TIME_WAIT;
							hall->bjl[index].flag = 0;
						}
						
						//sprintf(buf,"bjl show time end and game waiting...%d",(int)hall->time_now);
						//writeFile("/root/bjl.log",buf);
					}
					//printf("bjl game wait...\n");
					
				}
				else
				{
					//printf("...%d\n",i++);
					//sleep(3);
				}
				break;
			default:
				//未定义
				if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
				{
					sprintf(buf,"bjl case default time_now=[%d] start_time=[%d] %s",(int)hall->time_now,(int)hall->bjl[index].start_time,formatTime(myTime));
					writeToFile(path,buf);
				}
				/*if (forkid%hall->bjl[index].house_num == hall->bjl[index].house_num-1)
				{
					//hall->bjl[index].start_time = hall->time_now + TIME_WAIT;
					//hall->bjl[index].flag = 0;
					//sprintf(buf,"bjl default and game waiting...%d",(int)hall->time_now);
					//writeFile("/root/bjl.log",buf);
				}*/
				break;
		}
		if (hall->bjl[index].flag_game[forkid%hall->bjl[index].house_num] != hall->bjl[index].flag)
		{
			//printf("send 4020 to all...\n");
			hall->bjl[index].flag_game[forkid%hall->bjl[index].house_num] = hall->bjl[index].flag;
			//游戏状态广播
			length = pack4020(forkid,index,buf,BJL_BET_MAX);
			AllBjlSend(forkid,buf,length);
		}

		if (hall->bjl[index].flag_over[forkid%hall->bjl[index].house_num])
		{
			//printf("send 4021 to all...\n");
			hall->bjl[index].flag_over[forkid%hall->bjl[index].house_num] = 0;
			//游戏结束广播
			length = pack4021(forkid,index,buf,-1,BJL_BET_MAX,BJL_SEAT_MAX,GOLD_BANKER);
			AllBjlSend(forkid,buf,length);
			//更新列表记录信息
			length = pack5020(forkid,index,buf,BJL_BET_MAX);
			AllBjlSend(forkid,buf,length);
		}

		if (hall->bjl[index].flag_seat[forkid%hall->bjl[index].house_num])
		{
			//座位广播
			//printf("send 3020 to all...\n");
			hall->bjl[index].flag_seat[forkid%hall->bjl[index].house_num] = 0;
			//to do
			length = pack3020(forkid,index,buf,BJL_BET_MAX,BJL_SEAT_MAX,GOLD_BANKER);
			AllBjlSend(forkid,buf,length);
		}
		
		if (hall->bjl[index].flag == 1)
		{
			length = pack6020(forkid,index,buf,BJL_BET_MAX);
			AllBjlSend(forkid,buf,length);
		}

		if (hall->bjl[index].flag_msg[forkid%hall->bjl[index].house_num] && hall->bjl[index].flag != 2 && hall->bjl[index].flag != 3)
		{
			//有新消息
			if (hall->bjl[index].msg_list_flag == 0)
			{
				sem_wait(&(hall->bjl[index].lock_msg));
				if (hall->bjl[index].msg_list_flag == 0)
				{
					hall->bjl[index].msg_list_flag = 1;
				}
				sem_post(&(hall->bjl[index].lock_msg));
			}

			//开始广播
			for (i=0;i<MSG_LIST_MAX;i++)
			{
				if (hall->bjl[index].bjl_msg[i].length > 0)
				{
					AllBjlSend(forkid,hall->bjl[index].bjl_msg[i].msg,hall->bjl[index].bjl_msg[i].length);
				}
			}

			sem_wait(&(hall->bjl[index].lock_msg));
			hall->bjl[index].flag_msg[forkid%hall->bjl[index].house_num] = 0;
			//
			for (i=0;i<hall->bjl[index].house_num;i++)
			{
				if (hall->bjl[index].flag_msg[i])
				{
					break;
				}
			}

			if (i >= hall->bjl[index].house_num)
			{
				if (hall->bjl[index].msg_list_flag == 1)
				{
					for (i=0;i<MSG_LIST_MAX;i++)
					{
						hall->bjl[index].bjl_msg[i].length = 0;
						memset(hall->bjl[index].bjl_msg[i].msg,'\0',RECV_BUF_MAX);
					}
					hall->bjl[index].msg_list_flag = -1;
				}
			}
			sem_post(&(hall->bjl[index].lock_msg));
		}
		sleep(1);
	}
}

//进场入桌
int bjlInDesk(char *buf,int user,int forkid)
{
	short int myseat;
	int length,myforkid,mydesk;

	if (hall->user[user].hds < 0)
	{
		hall->user[user].hds = forkid*1000+BJL_SEAT_MAX;
	}

	myforkid = hall->user[user].hds/1000;
	mydesk = (hall->user[user].hds%1000)/10;
	myseat = hall->user[user].hds%10;
	
	//printf("forkid=%d,myforkid=%d,myseat=%d,hds=%d\n",forkid,myforkid,myseat,hall->user[user].hds);
	if (forkid != myforkid || myseat > BJL_SEAT_MAX)
	{
		return -1;
	}

	length = pack2003(buf,1,myforkid,mydesk,myseat);
	return UserSend(user,buf,length);
}

int bjlGameInfo(char *buf,int forkid,int index,int user)
{
	int length;
	//座位信息
	length = pack3020(forkid,index,buf,BJL_BET_MAX,BJL_SEAT_MAX,GOLD_BANKER);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	} 

	//游戏信息
	length = pack4020(forkid,index,buf,BJL_BET_MAX);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	} 
	//下注总额
	length = pack6020(forkid,index,buf,BJL_BET_MAX);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	} 
	//历史记录
	
	length = pack5020(forkid,index,buf,BJL_BET_MAX);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	}
	//自己的下注总额
	length = pack7020(forkid,index,buf,user,BJL_BET_MAX);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	} 

	return 0;
}

//离桌离场
int bjlOutDesk(char *buf,int index,int user)
{
	//printf("user %d bjl out desk hds=%d...\n",user,hall->user[user].hds);
	if (hall->user[user].hds < 0)
	{
		//未入桌
		return 0;
	}
	int length,seat;
	seat = hall->user[user].hds%10;
	bjlStandUp(index,seat);

	if (hall->user[user].banker_flag[index] != 0 && hall->user[user].banker_flag[index] != 2)
	{
		//上庄等待中
		bjlGiveUpBanker(buf,index,user);
	}

	if (hall->user[user].banker_flag[index] != 0 || hall->user[user].bjl_flag[index] == 1)
	{
		//不允许离场
		length = packReturnFlag(buf,2004,0);
		UserSend(user,buf,length);
		return 1;
	}

	hall->user[user].hds = -1;
	length = packPublicSeat(buf,3004,BJL_SEAT_MAX);
	UserSend(user,buf,length);
	return 0;
}

void bjlCleanUser(int index,int user)
{
	if (hall->user[user].hds < 0)
	{
		//未入桌
		return;
	}
	int i,j;
	bjlStandUp(index,hall->user[user].hds%10);

	if (hall->user[user].banker_flag[index] == 1)
	{
		//上庄等待
		sem_wait(&(hall->bjl[index].lock_banker));
		for (i=0;i<hall->bjl[index].list_num;i++)
		{
			if (hall->bjl[index].banker_list[i] == user)
			{
				hall->bjl[index].banker_list[i] = -1;
				hall->user[user].banker_flag[index] = 0;
				for (j=i;j<hall->bjl[index].list_num-1;j++)
				{
					hall->bjl[index].banker_list[j] = hall->bjl[index].banker_list[j+1];
					hall->bjl[index].banker_list[j+1] = -1;
				}
				break;
			}
		}

		if (i < hall->bjl[index].list_num)
		{
			hall->bjl[index].list_num--;
		}
		
		sem_post(&(hall->bjl[index].lock_banker));
		//设置座位广播
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}

	}
	else if (hall->user[user].banker_flag[index] == 10)
	{
		//上庄中->我要下庄
		hall->user[user].banker_flag[index] = 2;
		//设置座位广播
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}
	}

	if (hall->user[user].banker_flag[index] != 0 || hall->user[user].bjl_flag[index] == 1)
	{
		//不允许离场
		return;
	}

	hall->user[user].hds = -1;
	cleanSession(user);
}

//坐下 seat 0-7
int bjlSeatDown(char *buf,int index,int user,short int seat)
{
	int length;
	
	if (seat < 0 || seat >= BJL_SEAT_MAX)
	{
		//座号不对
		length = packPublicSeat(buf,2020,-1);
		return UserSend(user,buf,length);
	}

	if (hall->user[user].hds < 0)
	{
		length = packPublicSeat(buf,2020,-2);
		return UserSend(user,buf,length);
	}

	if (hall->user[user].hds%10 < BJL_SEAT_MAX)
	{
		//已入座
		length = packPublicSeat(buf,2020,-3);
		return UserSend(user,buf,length);
	}

	if (hall->bjl[index].seat[seat].user >= 0)
	{
		//座上已有人
		length = packPublicSeat(buf,2020,-4);
		return UserSend(user,buf,length);
	}
	
	sem_wait(&(hall->bjl[index].seat[seat].lock));
	if (hall->bjl[index].seat[seat].user < 0)
	{
		hall->bjl[index].seat[seat].user  = user;
		hall->user[user].hds += (seat - BJL_SEAT_MAX);
	}
	else
	{
		//座位已有人
		sem_post(&(hall->bjl[index].seat[seat].lock));
		length = packPublicSeat(buf,2020,-4);
		return UserSend(user,buf,length);
	}
	sem_post(&(hall->bjl[index].seat[seat].lock));
	//通知成功
	length = packPublicSeat(buf,2020,seat);
	UserSend(user,buf,length);
	//设置座位广播
	int i;
	for (i=0;i<hall->bjl[index].house_num;i++)
	{
		hall->bjl[index].flag_seat[i] = 1;
	}
	return 0;
}

//站起
void bjlStandUp(int index,int seat)
{
	//printf("bjl index=%d seat=%d\n",index,seat);
	if (seat < 0 || seat >= BJL_SEAT_MAX)
	{
		//已经站起
		return;
	}
	if (hall->bjl[index].seat[seat].user < 0 || hall->bjl[index].seat[seat].user >= USER_MAX)
	{
		return;
	}
	hall->user[hall->bjl[index].seat[seat].user].hds += (BJL_SEAT_MAX-seat);
	hall->bjl[index].seat[seat].user = -1;
	//设置座位广播
	int i;
	for (i=0;i<hall->bjl[index].house_num;i++)
	{
		hall->bjl[index].flag_seat[i] = 1;
	}
}

//我要上庄
int bjlWantBanker(char *buf,int index,int user)
{
	//int j,tmp;
	int uid;
	
	uid = getUidByToken(buf+25);
	//printf("token=[%s] uid=%d,user=%d,user.money=%lld list_num=%d banker_flag=%d\n",token,uid,user,hall->user[user].money,hall->bjl[index].list_num,hall->user[user].banker_flag[0]);
	if (uid < 0)
	{
		return -1;
	}
	int length;
	if (uid != hall->user[user].uid || hall->flag != 1 || hall->bjl[index].list_num == BANKER_MAX)
	{
		length = packReturnFlagGold(buf,2022,0,hall->user[user].money);
		return UserSend(user,buf,length);
	}
	
	if (hall->user[user].banker_flag[index] != 0)
	{
		//非初始化状态
		length = packReturnFlagGold(buf,2022,-1,hall->user[user].money);
		return UserSend(user,buf,length);
	}

	if (hall->user[user].money < UP_BANKER)
	{
		//金币不足
		length = packReturnFlagGold(buf,2022,-2,hall->user[user].money);
		return UserSend(user,buf,length);
	}

	sem_wait(&(hall->bjl[index].lock_banker));

	if (hall->user[user].banker_flag[index] != 0)
	{
		//非初始化状态
		sem_post(&(hall->bjl[index].lock_banker));
		length = packReturnFlagGold(buf,2022,-1,hall->user[user].money);
		return UserSend(user,buf,length);
	}
	
	if (hall->bjl[index].list_num >= BANKER_MAX)
	{
		sem_post(&(hall->bjl[index].lock_banker));
		length = packReturnFlagGold(buf,2022,-3,hall->user[user].money);
		return UserSend(user,buf,length);
	}
	/*
	for (i=0;i<BANKER_MAX;i++)
	{
		if (hall->bjl[index].banker_list[i] < 0)
		{
			//进入上庄等待队列
			hall->bjl[index].list_num++;
			hall->bjl[index].banker_list[i] = user;
			hall->user[user].banker_flag[index] = 1;
			break;
		}
	}*/
	hall->bjl[index].banker_list[hall->bjl[index].list_num] = user;
	hall->user[user].banker_flag[index] = 1;
	hall->bjl[index].list_num++;
	//排序
	/*for (i=0;i<hall->bjl[index].list_num-1;i++)
	{
		for (j=0;j<hall->bjl[index].list_num-1-i;j++)
		{
			if (hall->bjl[index].banker_list[j] >= 0 && hall->bjl[index].banker_list[j+1] >= 0)
			{
				if (hall->user[hall->bjl[index].banker_list[j]].money < hall->user[hall->bjl[index].banker_list[j+1]].money)
				{
					tmp = hall->bjl[index].banker_list[j];
					hall->bjl[index].banker_list[j] = hall->bjl[index].banker_list[j+1];
					hall->bjl[index].banker_list[j+1] = tmp;
				}
			}
			
		}
	}*/
	sem_post(&(hall->bjl[index].lock_banker));
	
	//设置座位广播
	int i;
	for (i=0;i<hall->bjl[index].house_num;i++)
	{
		hall->bjl[index].flag_seat[i] = 1;
	}
	length = packReturnFlagGold(buf,2022,1,hall->user[user].money);
	return UserSend(user,buf,length);
}

//我要下庄
int bjlGiveUpBanker(char *buf,int index,int user)
{
	int i,j,length;
	
	if (hall->user[user].banker_flag[index] == 0 || hall->user[user].banker_flag[index] == 2)
	{
		//未上庄 或 下庄等待中
		length = packReturnFlagGold(buf,2023,-1,hall->user[user].money);
		return UserSend(user,buf,length);
	}
	else if (hall->user[user].banker_flag[index] == 1)
	{
		//上庄等待
		sem_wait(&(hall->bjl[index].lock_banker));
		for (i=0;i<hall->bjl[index].list_num;i++)
		{
			if (hall->bjl[index].banker_list[i] == user)
			{
				//hall->bjl[index].list_num--;
				hall->bjl[index].banker_list[i] = -1;
				hall->user[user].banker_flag[index] = 0;
				for (j=i;j<hall->bjl[index].list_num-1;j++)
				{
					hall->bjl[index].banker_list[j] = hall->bjl[index].banker_list[j+1];
					hall->bjl[index].banker_list[j+1] = -1;
				}
				break;
			}
		}

		if (i < hall->bjl[index].list_num)
		{
			hall->bjl[index].list_num--;
		}
		
		sem_post(&(hall->bjl[index].lock_banker));
		//设置座位广播
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}

	}
	else
	{
		//上庄中->我要下庄
		hall->user[user].banker_flag[index] = 2;
		//设置座位广播
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}
	}
	length = packReturnFlagGold(buf,2023,1,hall->user[user].money);
	return UserSend(user,buf,length);
}

int bjlBankerList(char *buf,int index,int user)
{
	int length;
	//sem_wait(&(hall->bjl[index].lock_banker));
	length = pack3025(index,buf);
	//sem_post(&(hall->bjl[index].lock_banker));
	return UserSend(user,buf,length);
}

void bjlSetBanker(char *buf,int index)
{
	//flag 庄家是否改变，0没有，1有
	int i,flag=0,user = hall->bjl[index].banker;	
	//庄家设置
	if (user >= 0)
	{
		//printf("banker=%d flag=%d\n",hall->bjl[index].banker,hall->user[hall->bjl[index].banker].banker_flag);
		//有玩家在庄
		if (hall->bjl[index].banker_times >= 10 || hall->user[user].banker_flag[index] == 2 || hall->user[user].money < DOWN_BANKER || hall->user[user].status == 4 || hall->flag == 2)
		{
			//10局强制下庄 主动下庄 或 低于5千万，强制下庄 或关服
			hall->bjl[index].banker_times = 0;
			hall->bjl[index].banker = -1;
			hall->user[user].banker_flag[index] = 0;
			//更新数据
			//同步金币
			if (hall->user[user].status == 4 || (hall->user[user].status == 2 && hall->time_now - hall->user[user].time > 10))
			{
				//断线清理
				bjlCleanUser(index,user);
			}
			//
		}
		else
		{
			//继续连庄
			hall->bjl[index].banker_times++;
			for (i=0;i<hall->bjl[index].house_num;i++)
			{
				hall->bjl[index].flag_seat[i] = 1;
			}
			return;
		}
	}
	sem_wait(&(hall->bjl[index].lock_banker));
	if (hall->bjl[index].banker_list[0] >= 0 && hall->flag == 1)
	{
		//有玩家愿当庄
		hall->bjl[index].banker_times = 1;
		hall->bjl[index].banker = hall->bjl[index].banker_list[0];
		hall->user[hall->bjl[index].banker].banker_flag[index] = 10;
		//printf("bjl%d.banker = %d banker_flag=%d\n",index,hall->bjl[index].banker,hall->user[hall->bjl[index].banker].banker_flag[index]);
		hall->bjl[index].banker_list[0] = -1;
		for (i=0;i<hall->bjl[index].list_num-1;i++)
		{
			hall->bjl[index].banker_list[i] = hall->bjl[index].banker_list[i+1];
			hall->bjl[index].banker_list[i+1] = -1;
		}
		hall->bjl[index].list_num--;
		flag = 1;
	}
	else if (hall->bjl[index].banker == -1)
	{
		//切换系统庄
		flag = 1;
		hall->bjl[index].banker = myRandom()%3 - 102;
	}
	else
	{
		//系统连庄
	}
	//printf("bjl banker set %d...\n",hall->bjl[index].banker);
	sem_post(&(hall->bjl[index].lock_banker));

	//庄家改变
	if (flag)
	{
		//设置座位广播
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}
	}
	//
}

int bjlOrder(char *buf,int index,int user,short int type,long long int gold)
{
	//token = buf+27
	long long int gold_max;
	int i,length,times;
	//printf("user=%d,type=%d,gold=%lld,flag=%d,money=%lld\n",user,type,gold,hall->bjl[index].flag,hall->user[user].money);
	if (type < 0 || type > BJL_BET_MAX || gold <= 0)
	{
		//参数不合法
		length = pack2021(index,buf,user,type,0,BJL_BET_MAX);
		return UserSend(user,buf,length);
	}
	//下注
	if (hall->bjl[index].flag != 1)
	{
		//非下注期间
		length = pack2021(index,buf,user,type,-1,BJL_BET_MAX);
		return UserSend(user,buf,length);
	}
	
	if (hall->user[user].money < gold+gold*5/100 || hall->user[user].banker_flag[index] > 1)
	{
		//金币不足 或 庄家自己不能下注
		length = pack2021(index,buf,user,type,-2,BJL_BET_MAX);
		return UserSend(user,buf,length);
	}
	if (index < 2)
	{
		sem_wait(&(hall->bjl[index].lock_bet));
		/*
		for (i=0;i<BJL_BET_MAX;i++)
		{
			user_bet += hall->user[user].bet[index][i];
		}
		
		//printf("user_bet=%lld\n",user_bet);
		if (user_bet+gold > USER_BET_MAX)
		{
			sem_post(&(hall->bjl[index].lock_bet));
			length = pack2021(index,buf,user,type,-3,BJL_BET_MAX);
			return UserSend(user,buf,length);
		}
		*/
		if (hall->bjl[index].banker >= 0)
		{
			gold_max = hall->user[hall->bjl[index].banker].money/100*95;
		}
		else
		{
			gold_max = GOLD_BANKER/100*95;
		}

		if (hall->bjl[index].gold_total+gold > gold_max)
		{
			//下注总金额大于等于庄家金币
			sem_post(&(hall->bjl[index].lock_bet));
			length = pack2021(index,buf,user,type,-4,BJL_BET_MAX);
			return UserSend(user,buf,length);
		}	
		//__sync_fetch_and_add(&(hall->bjl[index].gold_total),gold);
		//__sync_fetch_and_add(&(hall->bjl[index].gold[type]),gold);
		hall->bjl[index].gold_total += gold;
		hall->bjl[index].gold[type] += gold;
		sem_post(&(hall->bjl[index].lock_bet));
		
		
		hall->user[user].money -= gold;
		hall->user[user].money -= gold*5/100;			//water
		hall->user[user].cake += gold*5/100;
		hall->user[user].bet[index][type] += gold;
		if (hall->user[user].bjl_flag[index] != 1)
		{
			hall->user[user].bjl_flag[index] = 1;
		}
		//printf("gold_total = %lld,gold_end = %lld\n",hall->bjl[index].gold_total,gold_end);
		//if (hall->bjl[index].gold_total >= gold_max)
		if (hall->bjl[index].gold_total > gold_max-100)
		{
			hall->bjl[index].start_time -= (TIME_BET - 3);
		}
	}
	else if (index == 3)
	{
		//龙虎
		sem_wait(&(hall->bjl[index].lock_bet));
		/*
		for (i=0;i<BJL_BET_MAX;i++)
		{
			user_bet += hall->user[user].bet[index][i];
		}
		
		//printf("user_bet=%lld\n",user_bet);
		if (user_bet+gold > USER_BET_MAX)
		{
			sem_post(&(hall->bjl[index].lock_bet));
			length = pack2021(index,buf,user,type,-3,BJL_BET_MAX);
			return UserSend(user,buf,length);
		}
		*/
		if (hall->bjl[index].banker >= 0)
		{
			gold_max = hall->user[hall->bjl[index].banker].money/100*95;
		}
		else
		{
			gold_max = GOLD_BANKER/100*95;
		}

		if (type == 0)
		{
			//押龙
			if (abs(hall->bjl[index].gold[0] - hall->bjl[index].gold[1] + gold) > gold_max)
			{
				sem_post(&(hall->bjl[index].lock_bet));
				length = pack2021(index,buf,user,type,-4,BJL_BET_MAX);
				return UserSend(user,buf,length);
			}
		}
		else if (type == 1)
		{
			//押虎
			if (abs(hall->bjl[index].gold[1] - hall->bjl[index].gold[0] + gold) > gold_max)
			{
				sem_post(&(hall->bjl[index].lock_bet));
				length = pack2021(index,buf,user,type,-4,BJL_BET_MAX);
				return UserSend(user,buf,length);
			}
		}
		else
		{
			//押和
			if ((hall->bjl[index].gold[2] + gold)*TIMES_HE > gold_max)
			{
				sem_post(&(hall->bjl[index].lock_bet));
				length = pack2021(index,buf,user,type,-4,BJL_BET_MAX);
				return UserSend(user,buf,length);
			}
		}
	
		//__sync_fetch_and_add(&(hall->bjl[index].gold[type]),gold);
		//__sync_fetch_and_add(&(hall->bjl[index].gold_total),gold);
		hall->bjl[index].gold[type] += gold;
		hall->bjl[index].gold_total += gold;
		sem_post(&(hall->bjl[index].lock_bet));

		hall->user[user].money -= gold;
		hall->user[user].money -= gold*5/100;			//water
		hall->user[user].cake += gold*5/100;
		hall->user[user].bet[index][type] += gold;
		if (hall->user[user].bjl_flag[index] != 1)
		{
			hall->user[user].bjl_flag[index] = 1;
		}
	}
	else
	{
		sem_wait(&(hall->bjl[index].lock_bet));

		if (hall->bjl[index].banker >= 0)
		{
			gold_max = hall->user[hall->bjl[index].banker].money/100*95;
		}
		else
		{
			gold_max = GOLD_BANKER/100*95;
		}

		for (i=0;i<BJL_BET_MAX;i++)
		{
			if (type == i)
			{
				times = getBJLTimes(index,i) - 1;
				gold_max -= times*(hall->bjl[index].gold[i]+gold);
			}
			else
			{
				gold_max += hall->bjl[index].gold[i];
			}
			//printf("%d gold_max = %lld\n",i,gold_max);
		}
		
		//printf("last gold_max=%lld\n",gold_max);
		if (gold_max < 0)
		{
			sem_post(&(hall->bjl[index].lock_bet));
			length = pack2021(index,buf,user,type,-4,BJL_BET_MAX);
			return UserSend(user,buf,length);
		}
		
		hall->bjl[index].gold[type] += gold;
		hall->bjl[index].gold_total += gold;
		sem_post(&(hall->bjl[index].lock_bet));

		hall->user[user].money -= gold;
		hall->user[user].money -= gold*5/100;			//water
		hall->user[user].cake += gold*5/100;
		hall->user[user].bet[index][type] += gold;
		if (hall->user[user].bjl_flag[index] != 1)
		{
			hall->user[user].bjl_flag[index] = 1;
		}
		//
	}
	//下注成功通知
	//printf("user = %d order gold=%lld ok!\n",user,gold);
	length = pack2021(index,buf,user,type,1,BJL_BET_MAX);
	if (UserSend(user,buf,length) < 0)
	{
		return -1;
	}
	//
	if (hall->user[user].hds%10 < BJL_SEAT_MAX)
	{
		//广播下注
		for (i=0;i<hall->bjl[index].house_num;i++)
		{
			hall->bjl[index].flag_seat[i] = 1;
		}
	}
	//
	return 0;
}

void createMyCards(int index,int type)
{
	int i,j,k,a,b;
	short int cards[BJL_CARD_MAX];
	short int max;
	short int hands[BJL_BET_MAX+1][3];		//+1 是庄家区域
	short int tmp[3];
	
	switch (index)
	{
		case 0:
			initCards(cards,BJL_CARD_MAX);
			max = BJL_CARD_MAX;
			for (i=0;i<BJL_BET_MAX+1;i++)
			{
				getMoreCards(0,3,cards,&max,hands[i]);
			}
			break;
		case 1:
			initCards(cards,TTZ_CARD_MAX);
			max = TTZ_CARD_MAX;
			for (i=0;i<BJL_BET_MAX+1;i++)
			{
				getMoreCards(2,2,cards,&max,hands[i]);
			}
			break;
		case 2:
			initCards(cards,BJL_CARD_MAX);
			max = BJL_CARD_MAX;
			for (i=0;i<BJL_BET_MAX+1;i++)
			{
				getMoreCards(1,1,cards,&max,hands[i]);
			}
			break;
		case 3:
			initCards(cards,18);
			max = 18;
			getMoreCards(1,3,cards,&max,hands[0]);
			/*max = rand()%6 + 1;
			hands[0][0] = max;
			hands[0][1] = max;
			hands[0][2] = max;*/
			break;
		case 4:
			hands[0][0] = getType(index);
			//printf("hands = %d\n",hands[0][0]);
			break;
		default:
			//拼三张
			initCards(cards,BJL_CARD_MAX);
			max = BJL_CARD_MAX;
			for (i=0;i<BJL_BET_MAX+1;i++)
			{
				getMoreCards(0,3,cards,&max,hands[i]);
			}
			break;
	}
	
	hall->bjl[index].banker_card[0] = hands[0][0];
	hall->bjl[index].banker_card[1] = hands[0][1];
	hall->bjl[index].banker_card[2] = hands[0][2];

	for (i=0;i<BJL_BET_MAX;i++)
	{
		hall->bjl[index].card[i][0] = hands[i+1][0];
		hall->bjl[index].card[i][1] = hands[i+1][1];
		hall->bjl[index].card[i][2] = hands[i+1][2];
	}

	if (hall->bjl[index].banker < 0 || checkUid(hall->user[hall->bjl[index].banker].uid) < 0)
	{
		//系统庄
		return;
	}

	if (index < 2)
	{
		//0,1
		if (hall->bjl[index].gold_total < 30000) return;
	}
	else if (index < 4)
	{
		//2,3
		a = get_point(hall->bjl[index].card[0][0],1);
		b = get_point(hall->bjl[index].card[1][0],1);
		if (a == b)
		{
			//和
			if (hall->bjl[index].gold[2]*TIMES_HE < 10000)
			{
				return;
			}
		}
		else
		{
			if (abs(hall->bjl[index].gold[0] -  hall->bjl[index].gold[1]) < 10000)
			{
				return;
			}
		}

	}
	else if (index == 4)
	{
		//4
		if (abs(hall->bjl[index].gold[0] -  hall->bjl[index].gold[1]) < 10000)
		{
			return;
		}
	}
	else
	{
		return;
	}
	//printf("do it!\n");
	//排序
	for (i=0;i<BJL_BET_MAX;i++)
	{
		for (j=0;j<BJL_BET_MAX-i;j++)
		{
			if (cards_pk(hands[j],hands[j+1]) == 2)
			{
				tmp[0] = hands[j][0];
				tmp[1] = hands[j][1];
				tmp[2] = hands[j][2];

				hands[j][0] = hands[j+1][0];
				hands[j][1] = hands[j+1][1];
				hands[j][2] = hands[j+1][2];

				hands[j+1][0] = tmp[0];
				hands[j+1][1] = tmp[1];
				hands[j+1][2] = tmp[2];
			}
		}
	}

	if (index == 0)
	{
		//百人三张
		short int site[4];
		type = myRandom()%3 + 2;
		hall->bjl[index].banker_card[0] = hands[4-type][0];
		hall->bjl[index].banker_card[1] = hands[4-type][1];
		hall->bjl[index].banker_card[2] = hands[4-type][2];

		j = 0;
		for (i=0;i<4+1;i++)
		{
			if (i!=4-type)
			{
				site[j] = i;
				j++;
			}
		}
			//0,1
		//后
		i = rand()%2 + 2;
		//前
		j = rand()%2;
		//
		k = rand()%3 + 1;

		if (k == 1)
		{
			a = 2;
			b = 3;
		}
		else if (k == 2)
		{
			a = 1;
			b = 3;
		} 
		else
		{
			a = 1;
			b = 2;
		}

		if (hall->bjl[index].gold[0] > hall->bjl[index].gold[k])
		{
			hall->bjl[index].card[0][0] = hands[site[i]][0];
			hall->bjl[index].card[0][1] = hands[site[i]][1];
			hall->bjl[index].card[0][2] = hands[site[i]][2];

			hall->bjl[index].card[k][0] = hands[site[j]][0];
			hall->bjl[index].card[k][1] = hands[site[j]][1];
			hall->bjl[index].card[k][2] = hands[site[j]][2];
		}
		else
		{
			hall->bjl[index].card[0][0] = hands[site[j]][0];
			hall->bjl[index].card[0][1] = hands[site[j]][1];
			hall->bjl[index].card[0][2] = hands[site[j]][2];

			hall->bjl[index].card[k][0] = hands[site[i]][0];
			hall->bjl[index].card[k][1] = hands[site[i]][1];
			hall->bjl[index].card[k][2] = hands[site[i]][2];
		}

		if (hall->bjl[index].gold[a] > hall->bjl[index].gold[b])
		{
			hall->bjl[index].card[a][0] = hands[site[5-i]][0];
			hall->bjl[index].card[a][1] = hands[site[5-i]][1];
			hall->bjl[index].card[a][2] = hands[site[5-i]][2];

			hall->bjl[index].card[b][0] = hands[site[1-j]][0];
			hall->bjl[index].card[b][1] = hands[site[1-j]][1];
			hall->bjl[index].card[b][2] = hands[site[1-j]][2];
		}
		else
		{
			hall->bjl[index].card[a][0] = hands[site[1-j]][0];
			hall->bjl[index].card[a][1] = hands[site[1-j]][1];
			hall->bjl[index].card[a][2] = hands[site[1-j]][2];

			hall->bjl[index].card[b][0] = hands[site[5-i]][0];
			hall->bjl[index].card[b][1] = hands[site[5-i]][1];
			hall->bjl[index].card[b][2] = hands[site[5-i]][2];
		}
		
		/*
		print_card(hall->bjl[index].banker_card);
		for (i=0;i<BJL_BET_MAX;i++)
		{
			print_card(hall->bjl[index].card[i]);
		}*/
	}
	else if (index == 2)
	{
		//龙虎
		a = get_point(hall->bjl[index].card[0][0],1);
		b = get_point(hall->bjl[index].card[1][0],1);

		while (a == b)
		{
			getMoreCards(1,1,cards,&max,hall->bjl[index].card[1]);
			b = get_point(hall->bjl[index].card[1][0],1);
		}

		if (hall->bjl[index].gold[0] > hall->bjl[index].gold[1])
		{
			//需 龙输虎赢
			if (a > b)
			{
				tmp[0] = hall->bjl[index].card[0][0];
				hall->bjl[index].card[0][0] = hall->bjl[index].card[1][0];
				hall->bjl[index].card[1][0] = tmp[0];
			}
		}
		else 
		{
			//需龙赢虎输
			if (a < b)
			{
				tmp[0] = hall->bjl[index].card[0][0];
				hall->bjl[index].card[0][0] = hall->bjl[index].card[1][0];
				hall->bjl[index].card[1][0] = tmp[0];
			}
		}

	}
	else if (index == 3)
	{
		//豹子王
		if (hall->bjl[index].gold[0] > hall->bjl[index].gold[1])
		{
			//需 大输小赢
			if (hall->bjl[index].banker_card[0]+hall->bjl[index].banker_card[1]+hall->bjl[index].banker_card[2] >= 11)
			{
				//开的大 生成小
				hall->bjl[index].banker_card[0] = rand()%6 + 1;
				a = 9 - hall->bjl[index].banker_card[0];
				if (a > 6)
				{
					hall->bjl[index].banker_card[1] = rand()%6 + 1;
				}
				else
				{
					hall->bjl[index].banker_card[1] = rand()%a + 1;
				}
				b = 10 - hall->bjl[index].banker_card[0] - hall->bjl[index].banker_card[1];
				if (b > 6)
				{
					hall->bjl[index].banker_card[2] = rand()%6 + 1;
				}
				else
				{
					hall->bjl[index].banker_card[2] = rand()%b + 1;
				}
			}
		}
		else 
		{
			//需 大赢小输
			if (hall->bjl[index].banker_card[0]+hall->bjl[index].banker_card[1]+hall->bjl[index].banker_card[2] < 11)
			{
				//开的小 生成大
				hall->bjl[index].banker_card[0] = rand()%6 + 1;
				a = 4 - hall->bjl[index].banker_card[0];
				if (a > 0)
				{
					hall->bjl[index].banker_card[1] = a + rand()%(6-a) + 1;
				}
				else
				{
					hall->bjl[index].banker_card[1] = rand()%6 + 1;
				}
				b = 10 - hall->bjl[index].banker_card[0] - hall->bjl[index].banker_card[1];
				if (b > 0)
				{
					hall->bjl[index].banker_card[2] = b + rand()%(6-b) + 1;
				}
				else
				{
					hall->bjl[index].banker_card[2] = rand()%6 + 1;
				}
			}
		}
	}
	else if (index == 4)
	{
		//奔驰宝马
		short int var[BJL_BET_MAX],times;
		j=0;
		for (i=0;i<BJL_BET_MAX;i++)
		{
			times = getBJLTimes(index,i);
			if (hall->bjl[index].gold[i]*times <= hall->bjl[index].gold_total)
			{
				var[j] = i;
				j++;
			}
		}

		hall->bjl[index].banker_card[0] = var[rand()%j] + 10*(rand()%3);
	}

}
//value:0-7 奔驰(大)，宝马(大)，奥迪(大)，大众(大)，奔驰(小)，宝马(小)，奥迪(小)，大众(小)
//site:1,2,3
//result=value+(site-1)*10
int getType(int index)
{
	//printf("getType start...\n");
	int type,win[BJL_BET_MAX];
	//type = myRandom()%10000 + 1;
	
	switch (index)
	{
		case 4:
			//奔驰宝马
			type = rand()%4 + 4;
			int i,j;
			j = 0;
			for (i=0;i<BJL_BET_MAX;i++)
			{
				win[i] = -1;
			}

			for (i=0;i<BJL_BET_MAX;i++)
			{
				if (hall->bjl[index].no_flag[i] <= 0)
				{
					win[j] = i;
					j++;
				}
				else
				{
					hall->bjl[index].no_flag[i]--;
				}
			}
			
			for (i=0;i<j;i++)
			{
				if (win[i] < 4)
				{
					type = win[i];
					if (myRandom()%(40-type*10) != type)
					{
						if (myRandom()%40 == type)
						{
							hall->bjl[index].no_flag[type] = (rand()%2 + 1)*(4-type)*10 + (rand()%(4-type))*10 + rand()%20 + 1;
						}
						else
						{
							hall->bjl[index].no_flag[type] = (rand()%(4-type))*10 + rand()%20 + 1;
						}
					}
					else
					{
						//连开
					}
					break;
				}
				else
				{
					type = myRandom()%4 + 4;
					break;
				}
			}

			return type + (rand()%3)*10;
			
			/*
			win[0] = 248;
			win[1] = 331;
			win[2] = 496;
			win[3] = 992;
			win[4] = 1983;
			win[5] = 1983;
			win[6] = 1983;
			win[7] = 1984;
			*/
			/*
			win[0] = 250;
			win[1] = 333;
			win[2] = 500;
			win[3] = 1000;
			win[4] = 1979;
			win[5] = 1979;
			win[6] = 1979;
			win[7] = 1980;*/

			/*
			if (type <= win[0])
			{
				type = 0 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1])
			{
				type = 1 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1] + win[2])
			{
				type = 2 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1] + win[2] + win[3])
			{
				type = 3 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1] + win[2] + win[3] + win[4])
			{
				type = 4 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1] + win[2] + win[3] + win[4] + win[5])
			{
				type = 5 + 10*(rand()%3);
			}
			else if (type <= win[0] + win[1] + win[2] + win[3] + win[4] + win[5] + win[6])
			{
				type = 6 + 10*(rand()%3);
			}
			else
			{
				type = 7 + 10*(rand()%3);
			}
			
			return type;
			*/
		default:
			return -1;
	}
}

//保存开奖记录
void bjlRecord(int index)
{
	int i,j;
	//0~20 最新~最旧
	for (i=BJL_L_MAX-1;i>0;i--)
	{
		for (j=0;j<BJL_BET_MAX;j++)
		{
			hall->bjl[index].list[i][j] = hall->bjl[index].list[i-1][j];
		}
	}

	for (j=0;j<BJL_BET_MAX;j++)
	{
		hall->bjl[index].list[0][j] = hall->bjl[index].win[j];
	}
}

//开奖
int bjlDraw(int index)
{
	int i,type;
	/*
	char buf[128];
	char path[64];

	sprintf(path,"/root/bjl_%d.log",index);
	sprintf(buf,"getType begin banker=%d",hall->bjl[index].banker);
	writeToFile(path,buf);
	
	if (hall->bjl[index].banker < 0)
	{
		type = getType(index);
	}
	else
	{
		type = -1;
	}

	sprintf(buf,"getType end type=%d",type);
	writeToFile(path,buf);*/

	type = -1;
	createMyCards(index,type);
	
	switch (index)
	{
		case 0:
			for (i=0;i<BJL_BET_MAX;i++)
			{
				if (cards_pk(hall->bjl[index].banker_card,hall->bjl[index].card[i]) == 1)
				{
					//庄胜
					hall->bjl[index].win[i] = 1;
				}
				else 
				{
					//庄负
					hall->bjl[index].win[i] = 2;
				}
			}
			break;
		case 1:
			for (i=0;i<TTZ_BET_MAX;i++)
			{
				if (cards_pk_ttz(hall->bjl[index].banker_card,hall->bjl[index].card[i]) == 1)
				{
					//庄胜
					hall->bjl[index].win[i] = 1;
				}
				else 
				{
					//庄负
					hall->bjl[index].win[i] = 2;
				}
			}
			break;
		case 2:
			if (get_point(hall->bjl[index].card[0][0],1) > get_point(hall->bjl[index].card[1][0],1))
			{
				//龙
				hall->bjl[index].win[0] = 2;
				hall->bjl[index].win[1] = 1;
				hall->bjl[index].win[2] = 3;			//和 闲输 系统赢 与庄无关
			}
			else if (get_point(hall->bjl[index].card[0][0],1) < get_point(hall->bjl[index].card[1][0],1))
			{
				//虎
				hall->bjl[index].win[0] = 1;
				hall->bjl[index].win[1] = 2;
				hall->bjl[index].win[2] = 3;			//和 闲输 系统赢 与庄无关
			}
			else
			{
				//和
				hall->bjl[index].win[0] = 0;			//退注 不退水
				hall->bjl[index].win[1] = 0;			//退注 不退水
				hall->bjl[index].win[2] = 4;			//和 闲胜 系统输 与庄无关
			}
			break;
		case 3:
			if (hall->bjl[index].banker_card[0] == hall->bjl[index].banker_card[1] && hall->bjl[index].banker_card[1] == hall->bjl[index].banker_card[2])
			{
				//豹子通杀
				hall->bjl[index].win[0] = 1;
				hall->bjl[index].win[1] = 1;
			}
			else if (hall->bjl[index].banker_card[0]+hall->bjl[index].banker_card[1]+hall->bjl[index].banker_card[2] >= 11)
			{
				//开大
				hall->bjl[index].win[0] = 2;
				hall->bjl[index].win[1] = 1;
			}
			else
			{
				//开小
				hall->bjl[index].win[0] = 1;
				hall->bjl[index].win[1] = 2;
			}
			break;
		case 4:
			for (i=0;i<BJL_BET_MAX;i++)
			{
				//printf("banker_card = %d\n",hall->bjl[index].banker_card[0]%10);
				if (hall->bjl[index].banker_card[0]%10 == i)
				{
					hall->bjl[index].win[i] = 2;
				}
				else
				{
					hall->bjl[index].win[i] = 1;
				}
			}
			break;
		default:
			for (i=0;i<BJL_BET_MAX;i++)
			{
				if (cards_pk(hall->bjl[index].banker_card,hall->bjl[index].card[i]) == 1)
				{
					//庄胜
					hall->bjl[index].win[i] = 1;
				}
				else 
				{
					//庄负
					hall->bjl[index].win[i] = 2;
				}
			}
			break;
	}
	/*
	printf("庄家->%d杀:",type);
	print_card(hall->bjl[index].card[0]);
	for (i=0;i<4;i++)
	{
		printf("%d方位win=%d:",i,hall->bjl[index].win[i]);
		print_card(hall->bjl[index].card[i]);
	}*/
	return 0;
}

int drawRecord(int index)
{
		//入库
	hall->bjl[index].qh = -1;
	if (hall->bjl[index].banker < 0)
	{
		//系统
		hall->bjl[index].qh = add_game_bjl_reward(index,-1,GOLD_BANKER,"57O757uf5bqE5a62",hall->bjl[index].banker_card,hall->bjl[index].win,hall->bjl[index].gold,(int)hall->time_now);
	}
	else
	{
		//玩家
		hall->bjl[index].qh = add_game_bjl_reward(index,hall->user[hall->bjl[index].banker].uid,hall->user[hall->bjl[index].banker].money,hall->user[hall->bjl[index].banker].name,hall->bjl[index].banker_card,hall->bjl[index].win,hall->bjl[index].gold,(int)hall->time_now);
	}

	if (hall->bjl[index].qh < 0)
	{
		return -1;
	}

	return 0;
}

//是否全部结算完成 1是，其它否
int bjlBalance(char *buf,int forkid,int index)
{
	long long int total=0,win_max;
	int i,j,k,l,uid,ret=1,times;
	
	//user = hall->bjl[index].banker;
	//机器人结算
	for (i=0;i<BJL_BET_MAX;i++)
	{
		if (hall->bjl[index].win[i] == 1)
		{
			//庄胜
			total -= hall->bjl[index].robot_gold[i];
		}
		else if (hall->bjl[index].win[i] == 2)
		{
			//庄负
			times = getBJLTimes(index,i) - 1;
			total += (hall->bjl[index].robot_gold[i]*times);
		}
	}

	while (add_system_bjl_record(index,total,hall->bjl[index].qh,hall->time_now) < 0)
	{
		sleep(1);
	}
	//庄家结算
	total = 0;
	for (i=0;i<BJL_BET_MAX;i++)
	{
		//printf("i=%d times=%d gold=%lld,total=%lld\n",i,times,hall->bjl[index].gold[i],total);
		if (hall->bjl[index].win[i] == 1)
		{
			//庄胜
			hall->bjl[index].gold_win[i] = hall->bjl[index].gold[i];
			total += hall->bjl[index].gold_win[i];
		}
		else if (hall->bjl[index].win[i] == 2)
		{
			//庄负
			times = getBJLTimes(index,i) - 1;
			hall->bjl[index].gold_win[i] = -(hall->bjl[index].gold[i]*times);
			total += hall->bjl[index].gold_win[i];
		}
		else if (hall->bjl[index].win[i] == 3)
		{
			//庄胜
			hall->bjl[index].gold_win[i] = hall->bjl[index].gold[i];
			total += hall->bjl[index].gold_win[i];
		}
		else if (hall->bjl[index].win[i] == 4)
		{
			//庄负
			hall->bjl[index].gold_win[i] = -(hall->bjl[index].gold[i]*TIMES_HE);
			total += hall->bjl[index].gold_win[i];
		}
	}
	//printf("index->%d total = %lld\n",index,total);
	if (hall->bjl[index].banker < 0)
	{
		//系统
		uid = -1;
	}
	else
	{
		//玩家
		
		uid = hall->user[hall->bjl[index].banker].uid;

		for (l=0;l<WINNER_MAX;l++)
		{
			if (total > hall->bjl[index].win_max[l])
			{
				sem_wait(&(hall->bjl[index].lock_winner));
				if (total > hall->bjl[index].win_max[l])
				{
					for (k=WINNER_MAX-1;k>l;k--)
					{
						hall->bjl[index].win_max[k] = hall->bjl[index].win_max[k-1];
						hall->bjl[index].win_uid[k] = hall->bjl[index].win_uid[k-1];
						sprintf(hall->bjl[index].win_name[k],hall->bjl[index].win_name[k-1]);
						sprintf(hall->bjl[index].win_url[k],hall->bjl[index].win_url[k-1]);
					}

					hall->bjl[index].win_max[l] = total;
					hall->bjl[index].win_uid[l] = uid;
					sprintf(hall->bjl[index].win_name[l],hall->user[hall->bjl[index].banker].name);
					sprintf(hall->bjl[index].win_url[l],hall->user[hall->bjl[index].banker].url);
					sem_post(&(hall->bjl[index].lock_winner));
					break;
				}
				sem_post(&(hall->bjl[index].lock_winner));
			}
		}
		
	}

	while ((hall->bjl[index].gold_total > 0) && update_game_bjl_banker(index,hall->bjl[index].qh,total) < 0)
	{
		//sql error
		sleep(1);
	}

	hall->bjl[index].banker_flag = 1;
	if (hall->bjl[index].banker >= 0)
	{
		//hall->user[hall->bjl[index].banker].bjl_balance[index] = 1;
		//抽水
		if (index < 2)
		{
			hall->user[hall->bjl[index].banker].money -= hall->bjl[index].gold_total*3/100;
			hall->user[hall->bjl[index].banker].cake += hall->bjl[index].gold_total*3/100;
		}
		else
		{
			hall->user[hall->bjl[index].banker].money -= abs(total)*3/100;
			hall->user[hall->bjl[index].banker].cake += abs(total)*3/100;
		}
		hall->user[hall->bjl[index].banker].money += total;
		hall->user[hall->bjl[index].banker].msg_gold_update = 1;
		//更新金币
		updateUserGold(hall->bjl[index].banker,0,0,0,0);
	}
	//生成游戏记录
	while ((hall->bjl[index].gold_total > 0) && add_game_bjl_record(index,uid,hall->bjl[index].qh,hall->bjl[index].gold,hall->bjl[index].gold_win,total,hall->user[hall->bjl[index].banker].money,(int)hall->time_now) < 0)
	{
		//sql error
		sleep(1);
	}
	//闲家结算
	for (j=0;j<USER_MAX;j++)
	{
		if (hall->user[j].bjl_flag[index] == 1)
		{
			//已下注
			total = 0;
			win_max = 0;
			for (i=0;i<BJL_BET_MAX;i++)
			{
				total -= hall->user[j].bet[index][i];
				times = getBJLTimes(index,i);
				if (hall->bjl[index].win[i] == 1)
				{
					//庄胜
					hall->user[j].gold_win[index][i] = 0;
				}
				else if (hall->bjl[index].win[i] == 2)
				{
					//庄负
					hall->user[j].gold_win[index][i] = hall->user[j].bet[index][i]*times;
					total += hall->user[j].gold_win[index][i];
					win_max += hall->user[j].gold_win[index][i];
				}
				else if (hall->bjl[index].win[i] == 0)
				{
					//和 退 
					hall->user[j].gold_win[index][i] = hall->user[j].bet[index][i];
					total += hall->user[j].gold_win[index][i];
					win_max += hall->user[j].gold_win[index][i];
				}
				else if (hall->bjl[index].win[i] == 3)
				{
					//闲负
					hall->user[j].gold_win[index][i] = 0;
				}
				else if (hall->bjl[index].win[i] == 4)
				{
					//闲胜
					hall->user[j].gold_win[index][i] = hall->user[j].bet[index][i]*(TIMES_HE+1);
					total += hall->user[j].gold_win[index][i];
					win_max += hall->user[j].gold_win[index][i];
				}
			}
			//大赢家
			for (l=0;l<WINNER_MAX;l++)
			{
				if (total > hall->bjl[index].win_max[l])
				{
					sem_wait(&(hall->bjl[index].lock_winner));
					if (total > hall->bjl[index].win_max[l])
					{
						for (k=WINNER_MAX-1;k>l;k--)
						{
							hall->bjl[index].win_max[k] = hall->bjl[index].win_max[k-1];
							hall->bjl[index].win_uid[k] = hall->bjl[index].win_uid[k-1];
							sprintf(hall->bjl[index].win_name[k],hall->bjl[index].win_name[k-1]);
							sprintf(hall->bjl[index].win_url[k],hall->bjl[index].win_url[k-1]);
						}

						hall->bjl[index].win_max[l] = total;
						hall->bjl[index].win_uid[l] = hall->user[j].uid;
						sprintf(hall->bjl[index].win_name[l],hall->user[j].name);
						sprintf(hall->bjl[index].win_url[l],hall->user[j].url);
						sem_post(&(hall->bjl[index].lock_winner));
						break;
					}
					sem_post(&(hall->bjl[index].lock_winner));
				}
			}
			//sem_wait(&(hall->user[j].lock_user));
			hall->user[j].money += win_max;
			hall->user[j].bjl_flag[index] = 0;
			hall->user[j].msg_gold_update = 1;
			hall->user[j].bjl_balance[index] = 1;
			
			//生成游戏记录
			while (add_game_bjl_record(index,hall->user[j].uid,hall->bjl[index].qh,hall->user[j].bet[index],hall->user[j].gold_win[index],total,hall->user[j].money,(int)hall->time_now) < 0)
			{
				//insert error
				//ret = 0;
				sleep(1);
			}

			for (i=0;i<BJL_BET_MAX;i++)
			{
				hall->user[j].bet[index][i] = 0;
			}
			//sem_post(&(hall->user[j].lock_user));
			updateUserGold(j,0,0,0,0);
			//清理
			if (hall->user[j].status == 4 || (hall->user[j].status == 2 && hall->time_now - hall->user[j].time > 10))
			{
				//断线清理
				printf("bjl flush user = %d\n",j);
				bjlCleanUser(index,j);
			}
			//
		}
	}

	return ret;
}

int bjlUserMsg(char *buf,int index,int user)
{
	if (hall->user[user].hds < 0)
	{
		//已离桌
		return 0;
	}
	int i,j,length;
	/*
	short int type,seat;
	recv_int16_from(msg,&type);
	if (type == 5)
	{
		seat = (hall->user[user].hds)%10;

		if (seat == BJL_SEAT_MAX+1)
		{
			return -1;
		}

		if (hall->user[user].money < MSG_FEE)
		{
			return 0;
		}

		//hall->user[user].money -= MSG_FEE;
		//hall->user[user].msg_gold_update = 1;
	}*/

	sem_wait(&(hall->bjl[index].lock_msg));
	if (hall->bjl[index].msg_list_flag > 0)
	{
		sem_post(&(hall->bjl[index].lock_msg));
		//return 0;
		length = packReturnFlag(buf,2026,0);
		return UserSend(user,buf,length);
	}

	for (i=0;i<MSG_LIST_MAX;i++)
	{
		if (hall->bjl[index].bjl_msg[i].length == 0)
		{
			hall->bjl[index].bjl_msg[i].length = pack3026(hall->bjl[index].bjl_msg[i].msg,buf+17,user);
			
			hall->bjl[index].msg_list_flag = 0;
			for (j=0;j<hall->bjl[index].house_num;j++)
			{
				hall->bjl[index].flag_msg[j] = 1;
			}
			sem_post(&(hall->bjl[index].lock_msg));

			length = packReturnFlag(buf,2026,1);
			return UserSend(user,buf,length);
		}
	}
	sem_post(&(hall->bjl[index].lock_msg));
	//return 0;
	length = packReturnFlag(buf,2026,0);
	return UserSend(user,buf,length);
}

void bjlCheckBankerList(int index)
{
	int i,j,k;
	int banker[BANKER_MAX];

	for (i=0;i<BANKER_MAX;i++)
	{
		banker[i] = -1;
	}
	
	sem_wait(&(hall->bjl[index].lock_banker));
	k = 0;
	for (i=0;i<hall->bjl[index].list_num;i++)
	{
		if (hall->bjl[index].banker_list[i] >= 0 && hall->user[hall->bjl[index].banker_list[i]].money < UP_BANKER)
		{
			hall->user[hall->bjl[index].banker_list[i]].banker_flag[index] = 0;
			hall->bjl[index].banker_list[i] = -1;
		}

		if (hall->bjl[index].banker_list[i] >= 0)
		{
			banker[k] = hall->bjl[index].banker_list[i];
			k++;
		}
	}
	
	for (i=0;i<BANKER_MAX;i++)
	{
		hall->bjl[index].banker_list[i] = banker[i];
	}
	hall->bjl[index].list_num = k;

	//按金额排序
	if (hall->bjl[index].list_num > 1)
	{
		short int tmp;
		for (i=0;i<hall->bjl[index].list_num-1;i++)
		{
			for (j=0;j<hall->bjl[index].list_num-1-i;j++)
			{
				if (hall->bjl[index].banker_list[j] >= 0 && hall->bjl[index].banker_list[j+1] >= 0)
				{
					if (hall->user[hall->bjl[index].banker_list[j]].money < hall->user[hall->bjl[index].banker_list[j+1]].money)
					{
						tmp = hall->bjl[index].banker_list[j];
						hall->bjl[index].banker_list[j] = hall->bjl[index].banker_list[j+1];
						hall->bjl[index].banker_list[j+1] = tmp;
					}
				}
			}
		}
	}

	//
	sem_post(&(hall->bjl[index].lock_banker));
}

int getBJLTimes(int index,int site)
{
	switch (index)
	{
		case 4:
			if (site > 3)
			{
				return 5;
			}
			else
			{
				return (4-site)*10;
			}
			break;
		default:
			return 2;
	}
	
}

//机器人下注
void robotOrder(int index)
{
	long long int gold_max,gold;
	int i,j,times;
	switch (index)
	{
		case 3:
			//豹子王
			if (hall->bjl[index].gold[0] > 0 || hall->bjl[index].gold[1] > 0)
			{
				gold = (rand()%5 + 1)*100;
				sem_wait(&(hall->bjl[index].lock_bet));
				hall->bjl[index].gold[0]+=gold;
				hall->bjl[index].gold[1]+=gold;
				hall->bjl[index].gold_total+=gold;
				sem_post(&(hall->bjl[index].lock_bet));
				hall->bjl[index].robot_gold[0]+=gold;
				hall->bjl[index].robot_gold[1]+=gold;
			}
			break;
		case 4:
			//奔驰宝马
			for (i=0;i<BJL_BET_MAX;i++)
			{
				if (hall->bjl[index].gold[i] > 0)
				{
					break;
				}
			}

			if (i >= BJL_BET_MAX)
			{
				break;
			}

			//
			sem_wait(&(hall->bjl[index].lock_bet));
			//gold = (rand()%5 + 1)*100;
			//gold = 100;
			for (j=4;j<BJL_BET_MAX;j++)
			{
				
				gold = (rand()%2)*100;
				if (gold == 0)
				{
					continue;
				}
				//gold = 100;

				if (hall->bjl[index].banker >= 0)
				{
					gold_max = hall->user[hall->bjl[index].banker].money/100*95;
				}
				else
				{
					gold_max = GOLD_BANKER/100*95;
				}

				for (i=0;i<BJL_BET_MAX;i++)
				{
					if (j == i)
					{
						times = getBJLTimes(index,i) - 1;
						gold_max -= times*(hall->bjl[index].gold[i]+gold);
					}
					else
					{
						gold_max += hall->bjl[index].gold[i];
					}
					//printf("%d gold_max = %lld\n",i,gold_max);
				}
				
				//printf("last gold_max=%lld\n",gold_max);
				if (gold_max < 0)
				{
					continue;
				}
				
				hall->bjl[index].gold[j] += gold;
				hall->bjl[index].robot_gold[j] += gold;
				hall->bjl[index].gold_total += gold;
			}
			sem_post(&(hall->bjl[index].lock_bet));
			//
			break;
		default:
			return;
	}
}