#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "cards.h"
#include "wzq.h"
#include "tcppack.h"

extern HALL *hall;

void wzqDealer(int forkid)
{
	int i,user,length;
	char buf[RECV_BUF_MAX];
	while (1)
	{
		for (i=0;i<DESK_MAX;i++)
		{
			if (hall->fork[forkid].zjh[i].desk_status != 0 && hall->fork[forkid].zjh[i].start_time != 0 && hall->time_now - hall->fork[forkid].zjh[i].start_time > WZQ_TIME_OUT)
			{
				//清理桌子
				sem_wait(&(hall->fork[forkid].zjh[i].lock));
				user = hall->fork[forkid].zjh[i].seat[1].user;
				if (user >= 0)
				{
					//对座有人
					length = packPublicSeat(buf,3004,1);
					AllSeatSend(forkid,i,buf,length);
					__sync_fetch_and_sub(&(hall->fork[forkid].zjh[i].people),1);
					hall->user[user].hds = -1;
					hall->fork[forkid].zjh[i].seat[1].user = -1;
					hall->fork[forkid].zjh[i].seat[1].status = 0;
				}
				user = hall->fork[forkid].zjh[i].seat[0].user;
				if (user >= 0 && hall->user[user].status != 10)
				{
					__sync_fetch_and_sub(&(hall->fork[forkid].zjh[i].people),1);
					hall->user[user].hds = -1;
					hall->fork[forkid].zjh[i].seat[0].user = -1;
					hall->fork[forkid].zjh[i].seat[0].status = 0;
					init_desk(forkid,i,1);
					cleanSession(user);
				}

				sem_post(&(hall->fork[forkid].zjh[i].lock));
			}
		}
		sleep(30);
	}
}

void WZQTimeOut(int forkid,int user,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return;
	}
	int desk,i,length,flag;

	if (hall->user[user].hds < 0)
	{
		//已离桌
		return;
	}
	desk = ((hall->user[user].hds)%1000)/10;
	//printf("flag = %d,time_now=%d,start_time=%d\n",hall->fork[forkid].zjh[desk].flag,(int)hall->time_now,(int)hall->fork[forkid].zjh[desk].start_time);
	switch (hall->fork[forkid].zjh[desk].flag)
	{
		case 0:
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].flag != 0)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return;
			}
			
			if (hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time)
			{
				flag = 0;
				for (i=0;i<2;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status == 2)
					{
						flag++;
					}
				}

				if (flag < 2)
				{
					//人数不够，游戏开始失败
					//printf("people < 2\n");
					//hall->fork[forkid].zjh[desk].start_time = 0;
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return;
				}
				
				//printf("fork%d desk%d game start ...\n",forkid,desk);
				//游戏开始
				hall->fork[forkid].zjh[desk].flag = 1;
				hall->fork[forkid].zjh[desk].start_time = 0;
				hall->fork[forkid].zjh[desk].player = flag;
				
				for (i=0;i<2;i++)
				{
					//玩家已准备 游戏开始初始化
					hall->fork[forkid].zjh[desk].seat[i].status = 3;
					hall->fork[forkid].zjh[desk].uid[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;					//记录uid
					sprintf(hall->fork[forkid].zjh[desk].name[i],hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name);			//记录name
					hall->fork[forkid].zjh[desk].gold[i] = 0;																			//记录gold
				}
				//广播游戏开始信息
				length = pack3006(buf,forkid,desk,120);
				AllSeatSend(forkid,desk,buf,length);

				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);
				//广播游戏信息
				length = pack4015(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 1:
			//游戏中
			break;
		default:
			break;
	}

	return;
}

int WZQCommand(int forkid,int user,int command,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	long long int gold=-1;
	int hds=-1;
	if (command != 1003)
	{
		if (hall->user[user].hds < 0 || hall->user[user].hds/1000 != forkid)
		{
			return -1;
		}
	}
	switch (command)
	{
		case 1003:
			//入桌
			recv_int32_from(buf+49,&hds);
			return ZJH_inDesk(user,forkid,buf,hds);
		case 1004:
			//离桌
			return deskOut(buf,user);
		case 1005:
			//准备
			return ZJH_ready(buf,user);
		case 1008:
			//上注
			recv_long64_from(buf+17,&gold);
			return WZQ_bet(buf,user,gold);
		case 1010:
			//认输
			return WZQ_balance(buf,user);
		case 1014:
			//桌内玩家消息
			return User_msg(user,buf);
		case 1015:
			//获取游戏初始化信息
			return gameInfo(buf,user);
		default:
			//未定义命令
			break;
	}
	return 0;
}

//wzq结算
int WZQ_balance(char *buf,int user)
{
	if (hall->flag != 1 && hall->time_now > hall->time_over - 300)
	{
		return 0;
	}

	int i,length,forkid,desk,seat,win=-1;
	
	forkid = hall->user[user].hds/1000;
	desk = (hall->user[user].hds%1000)/10;
	seat = hall->user[user].hds%10;

	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].zjh[desk].in_total > 0 && hall->fork[forkid].zjh[desk].seat[1].status != 0)
	{
		for (i=0;i<2;i++)
		{
			hall->fork[forkid].zjh[desk].gold_old[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
			hall->fork[forkid].zjh[desk].uid[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;					//记录uid
			sprintf(hall->fork[forkid].zjh[desk].name[i],hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name);			//记录name
			if (i == seat)
			{
				//认输方
				if (hall->user[user].money < hall->fork[forkid].zjh[desk].in_total)
				{
					//钱不够，认输失败
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return 0;
				}
				//是否冻结
				if (get_user_status(hall->user[user].uid) == 1)
				{
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return 0;
				}
				//
				hall->fork[forkid].zjh[desk].gold[i] = -hall->fork[forkid].zjh[desk].in_total;
			}
			else
			{
				//胜方
				win = i;
				hall->fork[forkid].zjh[desk].gold[i] = hall->fork[forkid].zjh[desk].in_total;
			}
		}
		//记录
		if (add_game_record(hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,0,hall->fork[forkid].zjh[desk].uid,hall->fork[forkid].zjh[desk].gold,hall->fork[forkid].zjh[desk].gold_old,hall->fork[forkid].zjh[desk].gold_new,hall->fork[forkid].zjh[desk].name,(int)(hall->time_now)) < 0)
		{
			//sql error
			
			//length = pack3013(buf,forkid,desk,(short)win,0);
			//AllSeatSend(forkid,desk,buf,length);
			//init_desk(forkid,desk,0);
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 0;
		}
		else
		{
			//插入mysql成功
			for (i=0;i<2;i++)
			{
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->fork[forkid].zjh[desk].gold[i];
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].msg_gold_update = 1;
				//更新金币
				updateUserGold(hall->fork[forkid].zjh[desk].seat[i].user,0,0,0,0);
			}
		}

		for (i=0;i<SEAT_MAX;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].status > 0)
			{
				//有人
				hall->fork[forkid].zjh[desk].seat[i].card_num = 0;
				hall->fork[forkid].zjh[desk].seat[i].begin_time = 0;
				hall->fork[forkid].zjh[desk].seat[i].in_all = 0;
				hall->fork[forkid].zjh[desk].seat[i].status = 1;
			}
		}
		hall->fork[forkid].zjh[desk].in_total = 0;
		hall->fork[forkid].zjh[desk].player = 0;
		hall->fork[forkid].zjh[desk].flag = 0;	
		//游戏结束广播 to do ...
		length = pack3013(buf,forkid,desk,(short)win,hall->fork[forkid].zjh[desk].gold[win]);
		AllSeatSend(forkid,desk,buf,length);
		//init_desk(forkid,desk,0);
	}
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	return 0;
}

int WZQ_bet(char *buf,int user,long long int gold)
{
	int length,forkid,desk,seat;

	if (gold <= 0 || gold > WZQ_BET_MAX)
	{
		return 0;
	}

	forkid = hall->user[user].hds/1000;
	desk = (hall->user[user].hds%1000)/10;
	seat = hall->user[user].hds%10;

	//sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (seat == 0)
	{
		hall->fork[forkid].zjh[desk].in_total = gold;
		//length = pack3008(buf,forkid,desk,seat,gold);
		//AllSeatSend(forkid,desk,buf,length);
		length = pack4015(buf,forkid,desk);
		AllSeatSend(forkid,desk,buf,length);
	}
	//sem_post(&(hall->fork[forkid].zjh[desk].lock));
	//
	return 0;
}