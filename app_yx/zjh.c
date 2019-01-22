#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "cards.h"
#include "tcppack.h"
#include "zjh.h"

extern HALL *hall;

int ZJHNormalCommand(int forkid,int user,int command,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	long long int gold=-1;
	int hds=-1;
	short int parm=-1;

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
			return ZJH_inDesk(user,forkid,buf,hds);
		case 1004:
			//离桌
			//giveUp(forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10);
			return deskOut(buf,user);
		case 1005:
			//准备
			return ZJH_ready(buf,user);
		case 1007:
			//看牌
			return ZJH_check(buf,forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10);
		case 1008:
			//上注
			recv_long64_from(buf+17,&gold);
			return ZJH_bet(buf,forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10,gold);
		case 1009:
			//比牌
			recv_int16_from(buf+17,&parm);
			return ZJH_PK(buf,forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10,(int)parm);
		case 1010:
			//弃牌
			return giveUp(buf,forkid,((hall->user[user].hds)%1000)/10,(hall->user[user].hds)%10);
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

void ZJHNormalTimeOut(int forkid,int user,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return;
	}
	int desk,i,turn,length,flag;

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
			//游戏未开始
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].flag != 0)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return;
			}
			if (hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time)
			{
				//游戏开始时间到
				//统计已准备人数
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
					//人数不足2人
					hall->fork[forkid].zjh[desk].start_time = 0;
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return;
				}
				//游戏开始
				hall->fork[forkid].zjh[desk].flag = 1;
				hall->fork[forkid].zjh[desk].start_time = 0;
				hall->fork[forkid].zjh[desk].player = flag;
				hall->fork[forkid].zjh[desk].order_max = hall->fork[forkid].zjh[desk].bottom;

				if (hall->fork[forkid].zjh[desk].turn > 0)
				{
					//上把赢家座位准备了 下家就当庄先说话
					turn = hall->fork[forkid].zjh[desk].turn+1;
					for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
					{
						if (turn >= hall->fork[forkid].zjh[desk].seat_max)
						{
							turn = 0;
						}

						if (hall->fork[forkid].zjh[desk].seat[turn].status == 2)
						{
							hall->fork[forkid].zjh[desk].turn = turn;
							break;
						}
						turn++;
					}

				}
				else
				{
					//随机庄家
					turn = myRandom()%hall->fork[forkid].zjh[desk].seat_max;
					for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
					{
						if (hall->fork[forkid].zjh[desk].seat[turn].status == 2)
						{
							hall->fork[forkid].zjh[desk].turn = turn;
							break;
						}
						turn++;
						if (turn >= hall->fork[forkid].zjh[desk].seat_max)
						{
							turn = 0;
						}
					}
				}
				hall->fork[forkid].zjh[desk].banker = hall->fork[forkid].zjh[desk].turn;
				//洗牌
				short int cards[52];
				short int cards_num = hall->fork[forkid].cards_num;
				printf("cards_num = %d\n",cards_num);
				initCards(cards,cards_num);
				
				hall->fork[forkid].zjh[desk].in_total = 0;

				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					//初始化
					hall->fork[forkid].zjh[desk].uid[i] = -1;
					hall->fork[forkid].zjh[desk].gold[i] = 0;
					hall->fork[forkid].zjh[desk].gold_old[i] = 0;
					hall->fork[forkid].zjh[desk].gold_new[i] = 0;
					memset(hall->fork[forkid].zjh[desk].name[i],'\0',USER_NAME_MAX);
					hall->fork[forkid].zjh[desk].seat[i].card[0] = 0;
					//
					if (hall->fork[forkid].zjh[desk].seat[i].status == 2)
					{
						//玩家已准备 游戏开始初始化
						hall->fork[forkid].zjh[desk].seat[i].status = 3;
						hall->fork[forkid].zjh[desk].seat[i].in_all = hall->fork[forkid].zjh[desk].bottom;

						if (hall->fork[forkid].gold_type == 1)
						{
							//金币
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold -= hall->fork[forkid].zjh[desk].bottom;		//押底
						}
						else
						{
							//现金
							hall->fork[forkid].zjh[desk].gold_old[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money -= hall->fork[forkid].zjh[desk].bottom;		//押底
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money -= hall->fork[forkid].zjh[desk].bottom*6/10;	//抽水
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].cake += hall->fork[forkid].zjh[desk].bottom*6/10;
							hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						}
						

						hall->fork[forkid].zjh[desk].uid[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;			//记录uid
						hall->fork[forkid].zjh[desk].gold[i] -= hall->fork[forkid].zjh[desk].bottom;								//记录投入
						sprintf(hall->fork[forkid].zjh[desk].name[i],hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name);	//记录名称


						hall->fork[forkid].zjh[desk].in_total += hall->fork[forkid].zjh[desk].bottom;
						//发牌
						getMoreCards(0,3,cards,&cards_num,hall->fork[forkid].zjh[desk].seat[i].card);

						length = pack3016(buf,hall->fork[forkid].zjh[desk].seat[i].user);
						AllSeatSend(forkid,desk,buf,length);
					}
				}
				/*
				hall->fork[forkid].zjh[desk].seat[0].card[0] = 14;
				hall->fork[forkid].zjh[desk].seat[0].card[1] = 28;
				hall->fork[forkid].zjh[desk].seat[0].card[2] = 42;

				hall->fork[forkid].zjh[desk].seat[1].card[0] = 13;
				hall->fork[forkid].zjh[desk].seat[1].card[1] = 27;
				hall->fork[forkid].zjh[desk].seat[1].card[2] = 41;
				*/ 
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					hall->fork[forkid].zjh[desk].seat[i].card_copy[0] = hall->fork[forkid].zjh[desk].seat[i].card[0];
					hall->fork[forkid].zjh[desk].seat[i].card_copy[1] = hall->fork[forkid].zjh[desk].seat[i].card[1];
					hall->fork[forkid].zjh[desk].seat[i].card_copy[2] = hall->fork[forkid].zjh[desk].seat[i].card[2];
				}

				hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time = hall->time_now + TIME_ACTION + 2;
				

				length = pack3006(buf,forkid,desk,TIME_ACTION);
				AllSeatSend(forkid,desk,buf,length);

				length = pack3015(buf,forkid,desk,-1);
				AllSeatSend(forkid,desk,buf,length);
				//
				//广播当前操作用户
				//length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
				//AllSeatSend(forkid,desk,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 1:
			//游戏进行中
			if (hall->time_now > hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time)
			{
				//玩家
				giveUp(buf,forkid,desk,hall->fork[forkid].zjh[desk].turn);
			}
			break;
	}
}

//传递令牌
void passOn(char *buf,int forkid,int desk,int seat)
{
	if (hall->fork[forkid].zjh[desk].turn == seat)
	{
		//令牌传递.
		int i,length;
		
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			hall->fork[forkid].zjh[desk].turn++;
			if (hall->fork[forkid].zjh[desk].turn >= hall->fork[forkid].zjh[desk].seat_max)
			{
				hall->fork[forkid].zjh[desk].turn = 0;
			}

			if (hall->fork[forkid].zjh[desk].banker == hall->fork[forkid].zjh[desk].turn)
			{
				hall->fork[forkid].zjh[desk].round++;
			}

			if (hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].status == 3 || \
				hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].status == 4)
			{
				hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time = hall->time_now + TIME_ACTION;
				/*if (hall->time_now >= hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time)
				{
					hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time = hall->time_now + TIME_ACTION;
				}
				else
				{
					hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time += TIME_ACTION;
				}*/
				break;
			}
		}

		if (hall->fork[forkid].zjh[desk].round > 20 || (hall->flag != 1 && hall->time_now > hall->time_over - 300))
		{
			zjhBalance(buf,forkid,desk);
			//重置所有座位状态
			//重置游戏
			init_desk(forkid,desk,0);
			//玩家清理
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].status > 0 && \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && \
				hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10)))
				{
					//座位有人
					//断线清理
					cleanUser(buf,hall->fork[forkid].zjh[desk].seat[i].user);
				}
			}

			length = pack3015(buf,forkid,desk,-1);
			AllSeatSend(forkid,desk,buf,length);
		}
		else
		{
			//正常轮流
			//令牌广播
			length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
			AllSeatSend(forkid,desk,buf,length);
		}
	}
}

void zjhBalance(char *buf,int forkid,int desk)
{
	//令牌不再传递，所有人强制比牌 或者 即将关服
	int i,length;
	int win = -1;
	short int type;
	long long int prize=0;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 2 && hall->fork[forkid].zjh[desk].seat[i].status < 5)
		{
			//已参与游戏 且 未弃牌
			if (win < 0 || cards_pk(hall->fork[forkid].zjh[desk].seat[win].card,hall->fork[forkid].zjh[desk].seat[i].card) == 2)
			{
				//b大 a弃牌 win变i
				win = i;
			}
		}
	}
	//游戏结束 已产生赢家
	//printf("fork%d desk%d round>20 game over ...\n",forkid,desk);
	//
	if (hall->fork[forkid].gold_type == 1)
	{
		hall->user[hall->fork[forkid].zjh[desk].seat[win].user].gold += hall->fork[forkid].zjh[desk].in_total;
	}
	else
	{
		hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money += hall->fork[forkid].zjh[desk].in_total;
		hall->fork[forkid].zjh[desk].gold[win] += hall->fork[forkid].zjh[desk].in_total;
		/*
		type = check_type(hall->fork[forkid].zjh[desk].seat[win].card[0],hall->fork[forkid].zjh[desk].seat[win].card[1],hall->fork[forkid].zjh[desk].seat[win].card[2]);
		
		switch (type)
		{
			case 7:
				prize = 3*hall->fork[forkid].zjh[desk].bottom;
				break;
			case 6:
				if (get_point(hall->fork[forkid].zjh[desk].seat[win].card[0],14) > 10)
				{
					//J-K
					prize = 2*hall->fork[forkid].zjh[desk].bottom;
				}
				else
				{
					//2-10
					prize = hall->fork[forkid].zjh[desk].bottom;
				}
				break;
			default:
				break;
		}*/

		if (prize > 0)
		{
			hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money += prize;
			//生成记录
			while (add_user_prize_top(hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,hall->fork[forkid].gameid,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,prize,type,hall->fork[forkid].zjh[desk].seat[win].card,(int)hall->time_now) < 0)
			{
				//sql error
				sleep(1);
			}
			//大奖通知
			//insertAAAMsg(hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].name,hall->fork[forkid].gameid,prize,type);
			//广播
			length = pack3061(buf,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,prize);
			AllSeatSend(forkid,desk,buf,length);
		}

		hall->fork[forkid].zjh[desk].gold_new[win] = hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money;

		//现金场游戏记录
		while (add_game_record(hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,\
			hall->fork[forkid].zjh[desk].uid,hall->fork[forkid].zjh[desk].gold,hall->fork[forkid].zjh[desk].gold_old,hall->fork[forkid].zjh[desk].gold_new,hall->fork[forkid].zjh[desk].name,(int)(hall->time_now)) < 0)
		{
			//sql失败
			sleep(1);
		}
		
	}

	
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 2 && hall->fork[forkid].zjh[desk].seat[i].status < 5)
		{
			//统一更新金币
			updateUserGold(hall->fork[forkid].zjh[desk].seat[i].user,0,0,0,0);
		}
	}
	
	//游戏结束广播
	length = pack3017(buf,forkid,desk,(short)win,hall->fork[forkid].zjh[desk].in_total);
	//printf("###########length = %d\n",length);
	AllSeatSend(forkid,desk,buf,length);
	hall->fork[forkid].zjh[desk].turn = (short)win;
}

int isGameOver(char *buf,int forkid,int desk,int seat)
{
	int i,length;

	if (hall->fork[forkid].zjh[desk].player == 1)
	{
		//游戏结束 结算to do... 
		int win = -1;
		short int type;
		long long int prize=0;

		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].status == 3 || hall->fork[forkid].zjh[desk].seat[i].status == 4)
			{
				win = i;
				break;
			}
		}

		if (win < 0)
		{
			return -1;
		}
		//printf("fork%d desk%d game over ...\n",forkid,desk);
		
		if (hall->fork[forkid].gold_type == 1)
		{
			hall->user[hall->fork[forkid].zjh[desk].seat[win].user].gold += hall->fork[forkid].zjh[desk].in_total;
		}
		else
		{
			hall->fork[forkid].zjh[desk].gold[win] += hall->fork[forkid].zjh[desk].in_total;
			hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money += hall->fork[forkid].zjh[desk].in_total;
			
			/*
			type = check_type(hall->fork[forkid].zjh[desk].seat[win].card[0],hall->fork[forkid].zjh[desk].seat[win].card[1],hall->fork[forkid].zjh[desk].seat[win].card[2]);
			switch (type)
			{
				case 7:
					prize = 3*hall->fork[forkid].zjh[desk].bottom;
					break;
				case 6:
					if (get_point(hall->fork[forkid].zjh[desk].seat[win].card[0],14) > 10)
					{
						//J-K
						prize = 2*hall->fork[forkid].zjh[desk].bottom;
					}
					else
					{
						//2-10
						prize = hall->fork[forkid].zjh[desk].bottom;
					}
					break;
				default:
					break;
			}*/

			if (prize > 0)
			{
				hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money += prize;
				//生成记录
				while (add_user_prize_top(hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,hall->fork[forkid].gameid,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,prize,type,hall->fork[forkid].zjh[desk].seat[win].card,(int)hall->time_now) < 0)
				{
					//sql error
					sleep(1);
				}
				//大奖通知
				//insertAAAMsg(hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].name,hall->fork[forkid].gameid,prize,type);
				//广播
				length = pack3061(buf,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].uid,prize);
				AllSeatSend(forkid,desk,buf,length);
			}

			hall->fork[forkid].zjh[desk].gold_new[win] = hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money;

			//现金场游戏记录
			while (add_game_record(hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,\
				hall->fork[forkid].zjh[desk].uid,hall->fork[forkid].zjh[desk].gold,hall->fork[forkid].zjh[desk].gold_old,hall->fork[forkid].zjh[desk].gold_new,hall->fork[forkid].zjh[desk].name,(int)(hall->time_now)) < 0)
			{
				//sql error
				sleep(1);
			}
		}

		//更新赢家金币
		updateUserGold(hall->fork[forkid].zjh[desk].seat[win].user,0,0,0,0);
		//广播结果
		length = pack3013(buf,forkid,desk,(short)win,hall->fork[forkid].zjh[desk].in_total);
		AllSeatSend(forkid,desk,buf,length);

		hall->fork[forkid].zjh[desk].turn = (short)win;
		return 1;
	}

	passOn(buf,forkid,desk,seat);
	return 0;
}

int giveUp(char *buf,int forkid,int desk,int seat)
{
	int i,length;
	//座位状态 0无人，1（游戏未开始）未准备，2（游戏未开始）已准备，3（游戏已开始）未看牌，4（游戏已开始）已看牌，5（游戏已开始）已弃牌
	
	if (hall->fork[forkid].zjh[desk].flag == 0 || hall->fork[forkid].zjh[desk].seat[seat].status == 5 || seat != hall->fork[forkid].zjh[desk].turn)
	{
		//游戏未开始，或已弃牌 必须轮到自己操作
		return 0;
	}
	//printf("giveUp fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].uid);
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].zjh[desk].seat[seat].status < 3 || hall->fork[forkid].zjh[desk].seat[seat].status == 5\
		|| hall->fork[forkid].zjh[desk].player <= 1)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		return 0;
	}

	__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].player),1);
	hall->fork[forkid].zjh[desk].seat[seat].status = 5;
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	//printf("giveUp ok\n");
	//弃牌广播
	length = packPublicSeat(buf,3010,(short)seat);
	AllSeatSend(forkid,desk,buf,length);
	
	//更新金币
	updateUserGold(hall->fork[forkid].zjh[desk].seat[seat].user,0,0,0,0);

	if (isGameOver(buf,forkid,desk,seat) == 1)
	{
		//桌子信息重置
		init_desk(forkid,desk,0);
		//玩家清理
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].status > 0 && \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && \
				hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10)))
			{
				//座位有人
				//清理
				cleanUser(buf,hall->fork[forkid].zjh[desk].seat[i].user);
			}
		}
		//
		printf("give up zjh over!\n");
		length = pack3015(buf,forkid,desk,-1);
		AllSeatSend(forkid,desk,buf,length);
	}
	
	return 0;
}

int ZJH_check(char *buf,int forkid,int desk,int seat)
{
	//看牌
	int length;

	if (hall->fork[forkid].zjh[desk].flag == 0)
	{
		//游戏未开始
		if (hall->fork[forkid].zjh[desk].seat[seat].card_copy[0] != 0)
		{
			length = packLookCards(buf,forkid,desk,seat,3);
			return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
		}
	}
	else
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].status != 3 || hall->fork[forkid].zjh[desk].round <= hall->fork[forkid].zjh[desk].round_max)
		{
			//不能看牌
			return 0;
		}
		//看牌应答
		length = packLookCards(buf,forkid,desk,seat,3);
		if (UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length) < 0)
		{
			return -1;
		}
		//printf("checkCards fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
		
		hall->fork[forkid].zjh[desk].seat[seat].status = 4;
		//看牌广播
		length = packPublicSeat(buf,3007,(short)seat);
		AllSeatSend(forkid,desk,buf,length);
		if (hall->fork[forkid].zjh[desk].turn == seat)
		{
			//当前操作者 重置操作时间
			hall->fork[forkid].zjh[desk].seat[seat].begin_time = hall->time_now + TIME_ACTION;
			length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
			AllSeatSend(forkid,desk,buf,length);

		}
	}
	
	
	return 0;
}

int ZJH_bet(char *buf,int forkid,int desk,int seat,long long gold)
{
	int length;
	
	if (hall->fork[forkid].zjh[desk].flag == 0 || hall->fork[forkid].zjh[desk].seat[seat].status < 3 \
		|| hall->fork[forkid].zjh[desk].seat[seat].status > 4 || seat != hall->fork[forkid].zjh[desk].turn)
	{
		//游戏未开始 或 不在游戏中 或 已弃牌 或 非当前操作
		return 0;
	}
	
	if (gold > (5*hall->fork[forkid].zjh[desk].bottom)*(hall->fork[forkid].zjh[desk].seat[seat].status-2)\
			|| gold < hall->fork[forkid].zjh[desk].order_max*(hall->fork[forkid].zjh[desk].seat[seat].status-2))
	{
		return 0;
	}
	//gold合法性判断
	if (hall->fork[forkid].gold_type == 1)
	{
		//金币场
		if (gold > hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold)
		{
			//跟注金额不足 或 上注金额不在范围内
			return 0;
		}
		hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold -= gold;
		
	}
	else
	{
		//现金场
		if (gold > hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money)
		{
			//跟注金额不足 或 上注金额不在范围内
			return 0;
		}

		hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money -= gold;
		hall->fork[forkid].zjh[desk].gold_new[seat] = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
	}
	

	if (gold/(hall->fork[forkid].zjh[desk].seat[seat].status-2) > hall->fork[forkid].zjh[desk].order_max)
	{
		hall->fork[forkid].zjh[desk].order_max = gold/(hall->fork[forkid].zjh[desk].seat[seat].status-2);
	}
	hall->fork[forkid].zjh[desk].seat[seat].in_all += gold;
	hall->fork[forkid].zjh[desk].in_total += gold;
	hall->fork[forkid].zjh[desk].gold[seat] -= gold;
	
	printf("bet fork%d desk%d seat%d gold=%lld,total=%lld\n",forkid,desk,seat,gold,hall->fork[forkid].zjh[desk].in_total);
	//上注成功广播
	
	length = pack3008(buf,forkid,desk,seat,gold);
	AllSeatSend(forkid,desk,buf,length);
	//
	//令牌传递
	passOn(buf,forkid,desk,seat);
	//
	return 0;
}

int ZJH_PK(char *buf,int forkid,int desk,int seat,int pk_seat)
{
	int i,length;
	int win,lose;
	long long int gold;

	if (seat == pk_seat || hall->fork[forkid].zjh[desk].flag == 0 || seat != hall->fork[forkid].zjh[desk].turn || hall->fork[forkid].zjh[desk].round <= hall->fork[forkid].zjh[desk].round_max)
	{
		//不能与自己比牌 游戏未开始 或 非当前操作玩家
		return 0;
	}

	if (hall->fork[forkid].zjh[desk].round == 1 && seat == hall->fork[forkid].zjh[desk].banker)
	{
		return 0;
	}
	
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].zjh[desk].seat[seat].status < 3 || hall->fork[forkid].zjh[desk].seat[seat].status > 4 || \
		hall->fork[forkid].zjh[desk].seat[pk_seat].status < 3 || hall->fork[forkid].zjh[desk].seat[pk_seat].status > 4)
	{
		//PK双方有人已弃牌或未加入游戏
		hall->fork[forkid].zjh[desk].seat[seat].begin_time = hall->time_now + TIME_ACTION;
		length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
		AllSeatSend(forkid,desk,buf,length);
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		return 0;
	}
	
	gold = hall->fork[forkid].zjh[desk].order_max*(hall->fork[forkid].zjh[desk].seat[seat].status-2);

	if (hall->fork[forkid].gold_type == 1)
	{
		//金币场
		if (hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold < gold)
		{
			//钱不够
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 0;
		}

		hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold -= gold;
		hall->fork[forkid].zjh[desk].seat[seat].in_all += gold;
		hall->fork[forkid].zjh[desk].in_total += gold;
		hall->fork[forkid].zjh[desk].gold[seat] -= gold;
	}
	else
	{
		//现金场
		if (hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money < gold)
		{
			//钱不够
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 0;
		}

		hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money -= gold;
		hall->fork[forkid].zjh[desk].seat[seat].in_all += gold;
		hall->fork[forkid].zjh[desk].in_total += gold;
		hall->fork[forkid].zjh[desk].gold[seat] -= gold;
	}

	//
	//开始比牌
	if (cards_pk(hall->fork[forkid].zjh[desk].seat[seat].card,hall->fork[forkid].zjh[desk].seat[pk_seat].card) == 1)
	{
		//自己大
		win = seat;
		lose = pk_seat;
	}
	else 
	{
		//别人大
		win = pk_seat;
		lose = seat;
	}
	hall->fork[forkid].zjh[desk].seat[lose].status = 6;
	__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].player),1);

	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	//广播比牌结果
	length = pack3009(buf,(short)seat,win,lose);
	AllSeatSend(forkid,desk,buf,length);
	
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		hall->fork[forkid].zjh[desk].seat[i].begin_time = hall->time_now + 5;
	}
	//上注广播
	length = pack3008(buf,forkid,desk,seat,gold);
	AllSeatSend(forkid,desk,buf,length);

	//更新金币
	updateUserGold(hall->fork[forkid].zjh[desk].seat[lose].user,0,0,0,0);
	//
	if (isGameOver(buf,forkid,desk,seat) == 1)
	{
		//已结束 展示牌
		length = pack3012(buf,forkid,desk,(short)win,(short)lose);
		AllSeatSend(forkid,desk,buf,length);

		//桌子信息重置
		init_desk(forkid,desk,0);
		//玩家清理
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].status > 0 && \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && \
				hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10)))
			{
				//座位有人
				//清理
				cleanUser(buf,hall->fork[forkid].zjh[desk].seat[i].user);
			}
		}
		//
	}
	return 0;
}
