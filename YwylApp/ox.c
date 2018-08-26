#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "cards.h"
#include "ox.h"
#include "tcppack.h"

extern HALL *hall;

void OXTimeOut(int forkid,int user,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return;
	}
	int desk,seat,i,j,k,length,flag;
	//char file[64];

	if (hall->user[user].hds < 0)
	{
		//已离桌
		return;
	}
	desk = ((hall->user[user].hds)%1000)/10;
	seat = hall->user[user].hds%10;
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
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
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
					hall->fork[forkid].zjh[desk].start_time = 0;
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return;
				}
				
				//printf("fork%d desk%d game start ...\n",forkid,desk);
				//游戏开始
				hall->fork[forkid].zjh[desk].start_time = hall->time_now + OX_BANKER_TIME;
				hall->fork[forkid].zjh[desk].flag = 1;
				hall->fork[forkid].zjh[desk].player = flag;
				
				//洗牌
				short int cards[OX_CARD_MAX];
				short int card_num = OX_CARD_MAX;
				//initCards(hall->fork[forkid].zjh[desk].cards,OX_CARD_MAX);
				initCards(cards,OX_CARD_MAX);
				//hall->fork[forkid].zjh[desk].card_num = OX_CARD_MAX;
				hall->fork[forkid].zjh[desk].in_total = 1;	//牛牛庄至少1倍
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					//玩家已准备 游戏开始初始化
					hall->fork[forkid].zjh[desk].uid[i] = -1;
					memset(hall->fork[forkid].zjh[desk].name[i],'\0',USER_NAME_MAX);
					hall->fork[forkid].zjh[desk].gold[i] = 0;	//记录
					hall->fork[forkid].zjh[desk].gold_old[i] = 0;
					hall->fork[forkid].zjh[desk].gold_new[i] = 0;

					//
					if (hall->fork[forkid].zjh[desk].seat[i].status == 2)
					{
						hall->fork[forkid].zjh[desk].seat[i].status = 3;
						hall->fork[forkid].zjh[desk].seat[i].in_all = 0;
						//玩家
						if (hall->fork[forkid].gold_type == 1)
						{
							//金币
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold -= hall->fork[forkid].zjh[desk].bottom;		//押底
						}
						else
						{
							//现金
							hall->fork[forkid].zjh[desk].gold_old[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money -= hall->fork[forkid].zjh[desk].bottom*6/10;	//抽水
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].cake += hall->fork[forkid].zjh[desk].bottom*6/10;
							hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						}

						hall->fork[forkid].zjh[desk].uid[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;					//记录
						sprintf(hall->fork[forkid].zjh[desk].name[i],hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name);			//记录名称
						//发牌
						getMoreCards(1,5,cards,&card_num,hall->fork[forkid].zjh[desk].seat[i].card);

						length = pack3016(buf,hall->fork[forkid].zjh[desk].seat[i].user);
						AllSeatSend(forkid,desk,buf,length);
					}
																					
				}
				/*
				hall->fork[forkid].zjh[desk].seat[0].card[0] = 13;
				hall->fork[forkid].zjh[desk].seat[0].card[1] = 12;
				hall->fork[forkid].zjh[desk].seat[0].card[2] = 11;
				hall->fork[forkid].zjh[desk].seat[0].card[3] = 10;
				hall->fork[forkid].zjh[desk].seat[0].card[4] = 9;
				*/
				/*
				hall->fork[forkid].zjh[desk].seat[1].card[0] = 55;
				hall->fork[forkid].zjh[desk].seat[1].card[1] = 41;
				hall->fork[forkid].zjh[desk].seat[1].card[2] = 50;
				hall->fork[forkid].zjh[desk].seat[1].card[3] = 36;
				hall->fork[forkid].zjh[desk].seat[1].card[4] = 22;
				*/
				//打扰顺序
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status == 3)
					{
						//flag = rand()%5;
						flag = (rand()%2)*4;
						k = 0;
						for (j=0;j<5;j++)
						{
							if (j != flag)
							{
								hall->fork[forkid].zjh[desk].seat[i].card_copy[k] = hall->fork[forkid].zjh[desk].seat[i].card[j];
								k++;
							}
						}
						
						hall->fork[forkid].zjh[desk].seat[i].card_copy[k] = hall->fork[forkid].zjh[desk].seat[i].card[flag];

						/*
						if (rand()%2)
						{
							hall->fork[forkid].zjh[desk].seat[i].card_copy[0] = hall->fork[forkid].zjh[desk].seat[i].card[4];
							hall->fork[forkid].zjh[desk].seat[i].card_copy[4] = hall->fork[forkid].zjh[desk].seat[i].card[0];
						}
						else
						{
							hall->fork[forkid].zjh[desk].seat[i].card_copy[0] = hall->fork[forkid].zjh[desk].seat[i].card[0];
							hall->fork[forkid].zjh[desk].seat[i].card_copy[4] = hall->fork[forkid].zjh[desk].seat[i].card[4];
						}

						if (rand()%2)
						{
							hall->fork[forkid].zjh[desk].seat[i].card_copy[1] = hall->fork[forkid].zjh[desk].seat[i].card[3];
							hall->fork[forkid].zjh[desk].seat[i].card_copy[3] = hall->fork[forkid].zjh[desk].seat[i].card[1];
						}
						else
						{
							hall->fork[forkid].zjh[desk].seat[i].card_copy[1] = hall->fork[forkid].zjh[desk].seat[i].card[1];
							hall->fork[forkid].zjh[desk].seat[i].card_copy[3] = hall->fork[forkid].zjh[desk].seat[i].card[3];
						}

						hall->fork[forkid].zjh[desk].seat[i].card_copy[2] = hall->fork[forkid].zjh[desk].seat[i].card[2];*/
					}
				}
				//广播游戏开始信息
				length = pack3006(buf,forkid,desk,OX_BANKER_TIME);
				AllSeatSend(forkid,desk,buf,length);
				//全部
				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);
				//广播游戏信息
				length = pack4015(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);
				//各发几张牌
				if (hall->fork[forkid].type == 1)
				{
					//明牌
					for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
					{
						if (hall->fork[forkid].zjh[desk].seat[i].status == 3 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 10)
						{
							length = packLookCards(buf,forkid,desk,i,4);
							UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
						}
					}
				}
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 1:
			//抢庄中
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if ((hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1) || OXIsTab(forkid,desk) == 1)
			{
				//抢庄时间到
				//printf("抢庄时间到...\n");
				if (hall->fork[forkid].zjh[desk].flag != 1)
				{
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return;
				}
				//
				//挑选庄家
				setOXBanker(forkid,desk);
				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);
				
				hall->fork[forkid].zjh[desk].start_time = hall->time_now + OX_BET_TIME;
				hall->fork[forkid].zjh[desk].flag = 2;
				//广播抢庄结果
				length = pack4015(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 2:
			//闲家上注中
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if ((hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1) || OXIsTab(forkid,desk) == 1)
			{
				//上注时间到
				//printf("上注时间到...\n");
				if (hall->fork[forkid].zjh[desk].flag != 2)
				{
					sem_post(&(hall->fork[forkid].zjh[desk].lock)); 
					return;
				}
				setOXBet(forkid,desk);
				//广播上注结果
				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);

				hall->fork[forkid].zjh[desk].start_time = hall->time_now + OX_COMPUTE_TIME;
				hall->fork[forkid].zjh[desk].flag = 3;
				//广播游戏信息
				length = pack4015(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);
				//分发5张牌
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status == 5 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 10)
					{
						length = packLookCards(buf,forkid,desk,i,5);
						UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
					}
				}
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock)); 
			break;
		case 3:
			//开牌结算中
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));

			//sprintf(file,"/root/ox_%d_%d.log",desk,seat);
			//sprintf(buf,"111111 start_time=%d,time_now=%d",(int)hall->fork[forkid].zjh[desk].start_time,(int)hall->time_now);
			//writeToFile(file,buf);

			if ((hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1) || OXIsTab(forkid,desk) == 1)
			{
				//开牌时间到
				//sprintf(buf,"222222 desk.flag=%d,time_now=%d",hall->fork[forkid].zjh[desk].flag,(int)hall->time_now);
				//writeToFile(file,buf);

				if (hall->fork[forkid].zjh[desk].flag != 3)
				{
					sem_post(&(hall->fork[forkid].zjh[desk].lock)); 
					return;
				}
				//游戏开牌
				//sprintf(buf,"333333 seat_max=%d,time_now=%d",hall->fork[forkid].zjh[desk].seat_max,(int)hall->time_now);
				//writeToFile(file,buf);

				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status == 5)
					{
						OX_PK(forkid,desk,i,buf);
					}
				}
				//length = pack3040(buf,forkid,desk,-1);
				//AllSeatSend(forkid,desk,buf,length);
				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);
				//游戏结算
				//sprintf(buf,"444444 OX_balance forkid=%d desk=%d,time_now=%d",forkid,desk,(int)hall->time_now);
				//writeToFile(file,buf);

				OX_balance(forkid,desk,seat,buf);

				//sprintf(buf,"000000 OX_balance over forkid=%d desk=%d,time_now=%d",forkid,desk,(int)hall->time_now);
				//writeToFile(file,buf);

				sem_post(&(hall->fork[forkid].zjh[desk].lock)); 
				//
				//hall->fork[forkid].zjh[desk].flag = 0;
				//hall->fork[forkid].zjh[desk].start_time = 0;
				//玩家清理
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status > 0 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status != 10)
					{
						//座位有人
						//清理
						cleanUser(hall->fork[forkid].zjh[desk].seat[i].user);
					}
				}
				//广播游戏信息
				length = pack4015(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);
				break;
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock)); 
			break;
		default:
			break;
	}

	return;
}

int OXCommand(int forkid,int user,int command,char *buf)
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
			return 0;
		}
	}

	switch (command)
	{
		case 1003:
			//入桌
			recv_int32_from(buf+49,&hds);
			return ZJH_inDesk(user,forkid,buf+17,hds);
		case 1004:
			//离桌
			return deskOut(user);
		case 1005:
			//准备
			return ZJH_ready(user);
		case 1008:
			//上注
			recv_long64_from(buf+17,&gold);
			return OX_bet(forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10,gold);
		case 1040:
			//牛牛开牌
			return OX_PK(forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10,buf);
		case 1014:
			//桌内玩家消息
			return User_msg(user,buf+17);
		case 1015:
			//获取游戏初始化信息
			return gameInfo(user);
		default:
			//未定义命令
			break;
	}
	return 0;
}
//牛牛庄家设置
void setOXBanker(int forkid,int desk)
{
	int i,j,k;
	int banker[5][5]={{0}};
	int index[5]={0};
	//按倍数生成抢庄数组
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 2)
		{
			j = hall->fork[forkid].zjh[desk].seat[i].in_all;
			banker[j][index[j]] = i+1;
			index[j]++;
		}
	}
	//倒序查找
	for (i=4;i>=0;i--)
	{
		if (banker[i][0] > 0)
		{
			break;
		}
	}
	//同倍累加
	k = 0;
	for (j=0;j<5;j++)
	{
		if (banker[i][j] > 0)
		{
			k++;
		}
	}
	//随机庄家座位号
	j = banker[i][rand()%k] - 1;
	hall->fork[forkid].zjh[desk].turn = j;
	hall->fork[forkid].zjh[desk].in_total = i>0?i:1;
	
	//printf("turn = %d total=%lld\n",hall->fork[forkid].zjh[desk].turn,hall->fork[forkid].zjh[desk].in_total);
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status == 3)
		{
			hall->fork[forkid].zjh[desk].seat[i].status = 4;
		}
	}
}

int OXIsTab(int forkid,int desk)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		switch (hall->fork[forkid].zjh[desk].flag)
		{
			case 1:
				if (hall->fork[forkid].zjh[desk].seat[i].status == 3)
				{
					//printf("seat %d status=%d OXIsTab 0\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
					return 0;
				}
				break;
			case 2:
				if (hall->fork[forkid].zjh[desk].seat[i].status == 4 && i != hall->fork[forkid].zjh[desk].turn)
				{
					//printf("seat %d status=%d OXIsTab 00\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
					return 0;
				}
				break;
			case 3:
				if (hall->fork[forkid].zjh[desk].seat[i].status == 5)
				{
					//printf("seat %d status=%d OXIsTab 000\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
					return 0;
				}
				break;
			default:
				break;
		}
	}
	//printf("OXIsTab 1111111\n");
	return 1;
}

void setOXBet(int forkid,int desk)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status == 4)
		{
			hall->fork[forkid].zjh[desk].seat[i].status = 5;
			hall->fork[forkid].zjh[desk].seat[i].in_all = 1;
		}
	}
}

//牛牛结算
void OX_balance(int forkid,int desk,int seat,char *buf)
{
	//char file[64];
	int i,length,type_a,type_b;
	long long int gold_balance[SEAT_MAX],gold_win,gold_lost,gold_tmp;
	int times,banker;
	
	gold_win = 0;
	gold_lost = 0;
	for (i=0;i<SEAT_MAX;i++)
	{
		gold_balance[i] = 0;
	}

	banker = hall->fork[forkid].zjh[desk].turn;

	//sprintf(file,"/root/ox_%d_%d.log",desk,seat);
	//sprintf(buf,"555555 OX_balance start forkid=%d desk=%d,banker=%d,seat_max=%d,time_now=%d",forkid,desk,banker,hall->fork[forkid].zjh[desk].seat_max,(int)hall->time_now);
	//writeToFile(file,buf);
	//printf("banker = %d\n",banker);
	if (banker < 0 || banker >= hall->fork[forkid].zjh[desk].seat_max)
	{
		return;
	}
	
	gold_balance[banker] = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (i != banker && hall->fork[forkid].zjh[desk].seat[i].status > 2)
		{
			//此座非庄家 且 有参与游戏
			type_a = check_type_ox(hall->fork[forkid].zjh[desk].seat[banker].card,hall->fork[forkid].zjh[desk].model,hall->fork[forkid].zjh[desk].kan);
			type_b = check_type_ox(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].model,hall->fork[forkid].zjh[desk].kan);
			if (cards_pk_ox(hall->fork[forkid].zjh[desk].seat[banker].card,hall->fork[forkid].zjh[desk].seat[i].card,type_a,type_b) == 1)
			{
				//庄大
				times = get_times_by_type(type_a,hall->fork[forkid].zjh[desk].model);
				gold_tmp = times*hall->fork[forkid].zjh[desk].in_total*hall->fork[forkid].zjh[desk].seat[i].in_all*hall->fork[forkid].zjh[desk].bottom;

				if (hall->fork[forkid].gold_type == 1)
				{
					if (gold_tmp > hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold)
					{
						//闲家钱不足
						gold_balance[i] = -hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold;
					}
					else
					{
						gold_balance[i] = -gold_tmp;
					}
				}
				else
				{
					if (gold_tmp > hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money)
					{
						//闲家钱不足
						gold_balance[i] = -hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					}
					else
					{
						gold_balance[i] = -gold_tmp;
					}
				}
				
			}
			else
			{
				//闲大
				times = get_times_by_type(type_b,hall->fork[forkid].zjh[desk].model);
				gold_tmp = times*hall->fork[forkid].zjh[desk].in_total*hall->fork[forkid].zjh[desk].seat[i].in_all*hall->fork[forkid].zjh[desk].bottom;
				//玩家

				if (hall->fork[forkid].gold_type == 1)
				{
					if (gold_tmp > hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold)
					{
						//闲家钱不足
						gold_balance[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold;
					}
					else
					{
						gold_balance[i] = gold_tmp;
					}
				}
				else
				{
					if (gold_tmp > hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money)
					{
						//闲家钱不足
						gold_balance[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					}
					else
					{
						gold_balance[i] = gold_tmp;
					}
				}
				
				
			}
			gold_balance[banker] -= gold_balance[i];

			if (gold_balance[i] > 0)
			{
				gold_win += gold_balance[i];
			}
			else
			{
				gold_lost += (-gold_balance[i]);
			}
			//
		}
	}

	//玩家
	if (hall->fork[forkid].gold_type == 1)
	{
		if (abs(gold_balance[banker]) > hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].gold)
		{
			//庄家钱不足
			if (gold_balance[banker] > 0)
			{
				//庄总赢 少赢gold_tmp
				gold_tmp = gold_balance[banker] - hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].gold;
				gold_balance[banker] = hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].gold;
				
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (gold_balance[i] < 0 && i != banker)
					{
						gold_balance[i] -= gold_balance[i]*gold_tmp/gold_lost;
					}
				}
			}
			else
			{
				//庄总负 少输gold_tmp
				gold_tmp = -gold_balance[banker] - hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].gold;
				gold_balance[banker] = -hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].gold;

				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (gold_balance[i] > 0 && i != banker)
					{
						gold_balance[i] -= gold_balance[i]*gold_tmp/gold_win;
					}
				}
			}
		}
	}
	else
	{
		if (abs(gold_balance[banker]) > hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].money)
		{
			//庄家钱不足
			if (gold_balance[banker] > 0)
			{
				//庄总赢 少赢gold_tmp
				gold_tmp = gold_balance[banker] - hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].money;
				gold_balance[banker] = hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].money;
				
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (gold_balance[i] < 0 && i != banker)
					{
						gold_balance[i] -= gold_balance[i]*gold_tmp/gold_lost;
					}
				}
			}
			else
			{
				//庄总负 少输gold_tmp
				gold_tmp = -gold_balance[banker] - hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].money;
				gold_balance[banker] = -hall->user[hall->fork[forkid].zjh[desk].seat[banker].user].money;

				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (gold_balance[i] > 0 && i != banker)
					{
						gold_balance[i] -= gold_balance[i]*gold_tmp/gold_win;
					}
				}
			}
		}
	}
	
	//sprintf(buf,"666666 OX_balance forkid=%d desk=%d,time_now=%d",forkid,desk,(int)hall->time_now);
	//writeToFile(file,buf);
	//同步金币 暂不处理
	//游戏记录
	if (hall->fork[forkid].gold_type == 1)
	{
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (gold_balance[i] != 0)
			{
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold += gold_balance[i];
			}
		}
	}
	else
	{
		short int type;
		long long int prize=0;
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (gold_balance[i] != 0)
			{
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold_balance[i];
			}
			
			if (hall->fork[forkid].zjh[desk].seat[i].status == 6)
			{
				prize = 0;
				type = check_type_ox(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].model,hall->fork[forkid].zjh[desk].kan);
				if (type > 10 && type < 18)
				{
					//产生彩金
					if (type > 10 && type < 15)
					{
						prize = (rand()%10 + 1)*hall->fork[forkid].zjh[desk].bottom;
					}
					else 
					{
						prize = 30*hall->fork[forkid].zjh[desk].bottom;
					}
					//sprintf(buf,"777777 OX_balance forkid=%d desk=%d i=%d type=%d,time_now=%d",forkid,desk,i,type,(int)hall->time_now);
					//writeToFile(file,buf);

					//sem_wait(&(hall->pool_lock));
					if (prize > hall->pool)
					{
						prize = hall->pool;
					}
					__sync_fetch_and_sub(&(hall->pool),prize);
					//sem_post(&(hall->pool_lock));
					
					//sprintf(buf,"888888 OX_balance forkid=%d desk=%d i=%d type=%d,prize=%lld,time_now=%d",forkid,desk,i,type,prize,(int)hall->time_now);
					//writeToFile(file,buf);
				}
				if (prize > 0)
				{
					hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += prize;

					//sprintf(buf,"999999 OX_balance forkid=%d desk=%d i=%d type=%d,prize=%lld,time_now=%d",forkid,desk,i,type,prize,(int)hall->time_now);
					//writeToFile(file,buf);
					//生成记录
					if (add_user_prize_top(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid,hall->fork[forkid].gameid,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,prize,type,hall->fork[forkid].zjh[desk].seat[i].card,(int)hall->time_now) < 0)
					{
						//sql error
					}

					//sprintf(buf,"xxxxxx OX_balance forkid=%d desk=%d i=%d type=%d,prize=%lld,time_now=%d",forkid,desk,i,type,prize,(int)hall->time_now);
					//writeToFile(file,buf);
					//大奖通知
					insertAAAMsg(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name,hall->fork[forkid].gameid,prize,type);

					//sprintf(buf,"yyyyyy OX_balance forkid=%d desk=%d i=%d type=%d,prize=%lld,time_now=%d",forkid,desk,i,type,prize,(int)hall->time_now);
					//writeToFile(file,buf);
					//广播
					length = pack3061(buf,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid,prize);
					AllSeatSend(forkid,desk,buf,length);
				}

				//更新金币
				updateUserGold(hall->fork[forkid].zjh[desk].seat[i].user);
			}

			hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
		}
		
		//sprintf(buf,"zzzzzz OX_balance forkid=%d desk=%d,time_now=%d",forkid,desk,(int)hall->time_now);
		//writeToFile(file,buf);
		if (add_game_record(hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,hall->fork[forkid].zjh[desk].uid,gold_balance,hall->fork[forkid].zjh[desk].gold_old,hall->fork[forkid].zjh[desk].gold_new,hall->fork[forkid].zjh[desk].name,(int)(hall->time_now)) < 0)
		{
			//sql error
		}
		//sprintf(buf,"aaaaaa OX_balance forkid=%d desk=%d,time_now=%d",forkid,desk,(int)hall->time_now);
		//writeToFile(file,buf);
	}
	

	//游戏结束广播
	length = pack3041(buf,gold_balance,forkid,desk);
	AllSeatSend(forkid,desk,buf,length);
	//桌子信息重置
	init_desk(forkid,desk,0);
	//
	length = pack3015(buf,forkid,desk,-1);
	AllSeatSend(forkid,desk,buf,length);
	//
}

int OX_PK(int forkid,int desk,int seat,char *buf)
{
	int length;
	
	if (hall->fork[forkid].zjh[desk].seat[seat].status != 5)
	{
		return 0;
	}
	hall->fork[forkid].zjh[desk].seat[seat].status = 6;
	length = pack3040(buf,forkid,desk,seat);
	AllSeatSend(forkid,desk,buf,length);
	return 0;
}

int OX_bet(int forkid,int desk,int seat,long long int gold)
{
	long long int gold_max=0,gold_user;
	int length;
	char buf[RECV_BUF_MAX];
	printf("bet fork%d desk%d seat%d gold=%lld\n",forkid,desk,seat,gold);
	
	switch (hall->fork[forkid].zjh[desk].flag)
	{
		case 1:
			//抢庄中
			if (hall->fork[forkid].zjh[desk].seat[seat].status != 3 || gold < 0 || gold > 4)
			{
				return 0;
			}
			
			if (hall->fork[forkid].gold_type == 1)
			{
				gold_user = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold;
			}
			else
			{
				gold_user = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
			}

			if (hall->fork[forkid].zjh[desk].model == 1)
			{
				//激情
				gold_max = gold_user/(hall->fork[forkid].zjh[desk].bottom*5*(hall->fork[forkid].zjh[desk].player-1));
			}
			else
			{
				//普通
				gold_max = gold_user/(hall->fork[forkid].zjh[desk].bottom*4*(hall->fork[forkid].zjh[desk].player-1));
			}
			
			if (gold > gold_max) return 0;
			hall->fork[forkid].zjh[desk].seat[seat].status = 4;
			break;
		case 2:
			//上注中
			if (hall->fork[forkid].zjh[desk].seat[seat].status != 4 || gold < 1 || gold > 5)
			{
				return 0;
			}
			
			if (hall->fork[forkid].gold_type == 1)
			{
				gold_user = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold;
			}
			else
			{
				gold_user = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
			}

			if (hall->fork[forkid].zjh[desk].model == 1)
			{
				//激情
				gold_max = gold_user/(hall->fork[forkid].zjh[desk].bottom*5*hall->fork[forkid].zjh[desk].in_total);
			}
			else
			{
				//普通
				gold_max = gold_user/(hall->fork[forkid].zjh[desk].bottom*4*hall->fork[forkid].zjh[desk].in_total);
			}
			if (gold > 1 && gold > gold_max)
			{
				return 0;
			}

			hall->fork[forkid].zjh[desk].seat[seat].status = 5;
			break;
		default:
			return 0;
	}

	
	hall->fork[forkid].zjh[desk].seat[seat].in_all = gold;
	//printf("bet fork%d desk%d seat%d uid=%d gold=%lld\n",forkid,desk,seat,hall->user[user].uid,gold);
	//上注成功广播

	length = pack3015(buf,forkid,desk,seat);
	AllSeatSend(forkid,desk,buf,length);
	//
	return 0;
}
