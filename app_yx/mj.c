#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "cards.h"
#include "tcppack.h"
#include "mj.h"

extern HALL *hall;

int MJCommand(int forkid,int user,int command,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	int hds=-1;
	short int parm,type;
	short int card[4];
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
		case 1014:
			//桌内玩家消息
			return User_msg(user,buf);
		case 1015:
			//获取游戏初始化信息
			return gameInfo(buf,user);
		case 1073:
			//请求换牌
			recv_int16_from(buf+17,&(card[0]));
			recv_int16_from(buf+19,&(card[1]));
			recv_int16_from(buf+21,&(card[2]));
			sem_wait(&(hall->fork[forkid].zjh[(hall->user[user].hds%1000)/10].lock));
			setMJChangeCards(buf,forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10,card,3);
			sem_post(&(hall->fork[forkid].zjh[(hall->user[user].hds%1000)/10].lock));
			return 1;
		case 1074:
			//定缺
			recv_int16_from(buf+17,&parm);
			MJ_lack(buf,forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10,parm);
			return 1;
		case 1076:
			recv_int16_from(buf+17,&type);
			recv_int16_from(buf+19,&parm);
			return MJ_action(buf,forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10,type,parm);
		case 1077:
			recv_int16_from(buf+17,&parm);
			return MJ_cardOut(buf,forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10,parm);
		case 1082:
			MJ_giveUp(buf,forkid,(hall->user[user].hds%1000)/10,hall->user[user].hds%10);
			break;
		default:
			//未定义命令
			break;
	}
	return 0;
}

void MJTimeOut(int forkid,int user,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return;
	}
	int desk,i,length,flag,seat;

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
				if (flag != hall->fork[forkid].zjh[desk].seat_max)
				{
					//人数不足
					hall->fork[forkid].zjh[desk].start_time = 0;
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return;
				}
				//游戏开始
				hall->fork[forkid].zjh[desk].player = flag;
				hall->fork[forkid].zjh[desk].start_time = 0;

				if (hall->fork[forkid].zjh[desk].banker < 0)
				{
					hall->fork[forkid].zjh[desk].banker = rand()%hall->fork[forkid].zjh[desk].seat_max;
				}
				//hall->fork[forkid].zjh[desk].banker = 0;
				hall->fork[forkid].zjh[desk].turn = hall->fork[forkid].zjh[desk].banker;

				//洗牌
				initCards(hall->fork[forkid].zjh[desk].cards,hall->fork[forkid].cards_num);
				hall->fork[forkid].zjh[desk].card_num = hall->fork[forkid].cards_num;
				
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					//
					//玩家已准备 游戏开始初始化
					hall->fork[forkid].zjh[desk].seat[i].status = 3;
					//现金
					hall->fork[forkid].zjh[desk].gold_old[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money -= hall->fork[forkid].zjh[desk].bottom*3/10;	//抽水
					hall->user[hall->fork[forkid].zjh[desk].seat[i].user].cake += hall->fork[forkid].zjh[desk].bottom*3/10;
					hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					

					hall->fork[forkid].zjh[desk].uid[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;			//记录uid
					hall->fork[forkid].zjh[desk].gold[i] = 0;																	//记录投入
					sprintf(hall->fork[forkid].zjh[desk].name[i],hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name);	//记录名称
					
					//发牌
					if (i == hall->fork[forkid].zjh[desk].banker)
					{
						getMJCards(14,hall->fork[forkid].zjh[desk].cards,&(hall->fork[forkid].zjh[desk].card_num),hall->fork[forkid].zjh[desk].seat[i].card);
						hall->fork[forkid].zjh[desk].seat[i].card_num = 14;
					}
					else
					{
						getMJCards(13,hall->fork[forkid].zjh[desk].cards,&(hall->fork[forkid].zjh[desk].card_num),hall->fork[forkid].zjh[desk].seat[i].card);
						hall->fork[forkid].zjh[desk].seat[i].card_num = 13;
					}

					length = pack3016(buf,hall->fork[forkid].zjh[desk].seat[i].user);
					AllSeatSend(forkid,desk,buf,length);
				}

				
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					length = pack2071(buf,forkid,desk,i);
					UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
				}
				
				if (hall->fork[forkid].zjh[desk].mj_change == 1)
				{
					//换牌
					hall->fork[forkid].zjh[desk].flag = 1;
				}
				else
				{
					hall->fork[forkid].zjh[desk].flag = 3;
				}
				hall->fork[forkid].zjh[desk].start_time = hall->time_now + MJ_LACK_TIME;

				length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,MJ_LACK_TIME);
				AllSeatSend(forkid,desk,buf,length);

				
				//
				//广播当前操作用户
				//length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
				//AllSeatSend(forkid,desk,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 1:
			//第一次换三张
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].flag != 1)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return;
			}

			if ((hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1) || MJIsTab(forkid,desk) == 1)
			{
				//超时 强制设置要换的牌 并交换
				doMJChange(buf,forkid,desk,3);

				hall->fork[forkid].zjh[desk].flag = 3;
				hall->fork[forkid].zjh[desk].start_time = hall->time_now + MJ_LACK_TIME;

				length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,MJ_LACK_TIME);
				AllSeatSend(forkid,desk,buf,length);
			}
			
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 3:
			//定缺中
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].flag != 3)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return;
			}
			
			if ((hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1) || MJIsTab(forkid,desk) == 1)
			{
				setMJLack(forkid,desk);

				length = pack4074(buf,forkid,desk);
				AllSeatSend(forkid,desk,buf,length);

				hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time = hall->time_now + MJ_ACTION_TIME;
				hall->fork[forkid].zjh[desk].flag = 4;
				hall->fork[forkid].zjh[desk].start_time = 0;

				length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,-1);
				AllSeatSend(forkid,desk,buf,length);

				length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
				AllSeatSend(forkid,desk,buf,length);
				MJCheckMyself(buf,forkid,desk,hall->fork[forkid].zjh[desk].turn);
			}

			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 4:
			//行牌中 庄家起手或摸牌判断自己，出牌或巴杠判断其余三家
			//轮座是否出牌
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].flag != 4 || seat != 0)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return;
			}
			
			//轮方判断
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				//非轮流方
				//除胡牌 其余操作过
				if (hall->fork[forkid].zjh[desk].seat[i].begin_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].seat[i].begin_time)
				{	
					if (hall->fork[forkid].zjh[desk].seat[i].flag_win == 1)
					{
						//默认自动胡牌
						hall->fork[forkid].zjh[desk].seat[i].flag_chi = 0;
						hall->fork[forkid].zjh[desk].seat[i].flag_peng = 0;
						hall->fork[forkid].zjh[desk].seat[i].flag_gang = 0;
						hall->fork[forkid].zjh[desk].seat[i].flag_win = 3;
						length = packReturnFlag(buf,2076,1);
						UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
					}

					if (hall->fork[forkid].zjh[desk].seat[i].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[i].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[i].flag_chi == 1)
					{
						//默认吃碰杠过
						printf("吃碰杠过\n");
						hall->fork[forkid].zjh[desk].seat[i].flag_chi = 0;
						hall->fork[forkid].zjh[desk].seat[i].flag_peng = 0;
						hall->fork[forkid].zjh[desk].seat[i].flag_gang = 0;
						
						length = packReturnFlag(buf,2076,1);
						UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
					}
					
				}
			}
			
			//操作判断
			if (MJActionCheck(buf,forkid,desk) == 1)
			{
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				break;
			}

			if (MJCheckAllSeat(forkid,desk) == 1)
			{
				//全部已操作
				//令牌方判断
				seat = hall->fork[forkid].zjh[desk].turn;

				if (hall->fork[forkid].zjh[desk].seat[seat].ranking > 0)
				{
					//已胡牌 或 已认输
					i = MJPassOn(buf,forkid,desk);
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					if (i == 1)
					{
						//结算记录
						MJBalance(forkid,desk);
						init_desk(forkid,desk,0);
						for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
						{
							if (hall->fork[forkid].zjh[desk].seat[i].status > 0)
							{
								//座位有人
								updateUserGold(hall->fork[forkid].zjh[desk].seat[i].user,0,0,0,0);
								if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
								(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && \
								hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10))
								{
									//清理
									cleanUser(buf,hall->fork[forkid].zjh[desk].seat[i].user);
								}
							}
						}
					}
					break;
				}
				//printf("turn = %d,begint_time=%d,card_out=%d\n",seat,(int)hall->fork[forkid].zjh[desk].seat[seat].begin_time,hall->fork[forkid].zjh[desk].seat[seat].card_out);
				if (hall->fork[forkid].zjh[desk].seat[seat].begin_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].seat[seat].begin_time && hall->fork[forkid].zjh[desk].seat[seat].ranking == 0)
				{
					//超时
					if (hall->fork[forkid].zjh[desk].seat[seat].card_out == 0)
					{
						printf("打牌超时且全部已操作,未出牌强制出牌\n");
						//未出牌
						hall->fork[forkid].zjh[desk].seat[seat].card_out = getMJCardOut(forkid,desk,seat);
						//出牌
						MJSetOneCard(buf,forkid,desk,seat);
					}
				}

				//
				if ((hall->fork[forkid].zjh[desk].seat[seat].card_out > 0 || hall->fork[forkid].zjh[desk].seat[seat].ranking > 0) && MJCheckAllSeat(forkid,desk) == 1)
				{
					//确认出牌 令牌转动
					hall->fork[forkid].zjh[desk].seat[seat].card_out_list[hall->fork[forkid].zjh[desk].seat[seat].card_out_num] = hall->fork[forkid].zjh[desk].seat[seat].card_out;
					hall->fork[forkid].zjh[desk].seat[seat].card_out_num++;

					i = MJPassOn(buf,forkid,desk);
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					if (i == 1)
					{
						//结算记录
						MJBalance(forkid,desk);
						init_desk(forkid,desk,0);
						for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
						{
							if (hall->fork[forkid].zjh[desk].seat[i].status > 0)
							{
								//座位有人
								updateUserGold(hall->fork[forkid].zjh[desk].seat[i].user,0,0,0,0);
								if(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
								(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && \
								hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10))
								{
									//清理
									cleanUser(buf,hall->fork[forkid].zjh[desk].seat[i].user);
								}
							}
						}
					}
					break;
				}
				//
			}
		
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			break;
		case 9:
		sem_wait(&(hall->fork[forkid].zjh[desk].lock));
		if (hall->fork[forkid].zjh[desk].flag != 9)
		{
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return;
		}

		if (hall->fork[forkid].zjh[desk].start_time != 0 && hall->time_now > hall->fork[forkid].zjh[desk].start_time+1)
		{
			//超时
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].ranking == 0 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money <= 0)
				{
					//强制认输
					hall->fork[forkid].zjh[desk].seat[i].ranking = 4;
					length = pack3082(buf,i);
					AllSeatSend(forkid,desk,buf,length);
				}
			}
		}

		if (MJIsTab(forkid,desk) == 1)
		{
			//判断游戏是否结束
			
			//全部已操作
			hall->fork[forkid].zjh[desk].flag = 4;
			hall->fork[forkid].zjh[desk].start_time = 0;

			length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,-1);
			AllSeatSend(forkid,desk,buf,length);
			
			length = 0;
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].ranking > 0)
				{
					length++;
				}
			}
			
			if (length >= hall->fork[forkid].zjh[desk].seat_max-1)
			{
				//全部已胡牌或认输 游戏结束
			}
			else if (hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].ranking == 0)
			{
				hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time = hall->time_now + MJ_ACTION_TIME;
				length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
				AllSeatSend(forkid,desk,buf,length);
			}
			
		}

		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		break;
	}
	
}

void doMJChange(char *buf,int forkid,int desk,short int num)
{
	int i,j,length;
	short int type;
	short int card[4];
	//强制设置要换的牌
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].card_change[0] == 0)
		{
			//未设置
			getMJChangeCards(forkid,desk,i,card,num);
			setMJChangeCards(buf,forkid,desk,i,card,num);
		}
	}

	//换牌方向确定
	type = rand()%3 + 1;
	//开始换牌
	switch (type)
	{
		case 2:
			//顺时针 0>3>2>1>0
			
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				//增加牌
				if (i == 0)
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[3].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[3].card_change,num);
				}
				else
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[i-1].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[i-1].card_change,num);
				}
				//排序
				sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);
				//减少牌
				for (j=0;j<num;j++)
				{
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_change[j]);
				}

				//结果通知
				UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
			}
			
			break;
		case 1:
			//逆时针 0>1>2>3>0
			
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				//增加牌
				if (i == 3)
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[0].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[0].card_change,num);
				}
				else
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[i+1].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[i+1].card_change,num);
				}
				//排序
				sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);
				//减少牌
				for (j=0;j<num;j++)
				{
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_change[j]);
				}
				//结果通知
				UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
			}

			break;
		case 3:
			//对家0>2 2>0 1>3 3>1
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				//增加牌
				if (i == 0)
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[2].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[2].card_change,num);
				}
				else if (i == 2)
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[0].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[0].card_change,num);
				}
				else if (i == 1)
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[3].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[3].card_change,num);
				}
				else
				{
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[1].card_change[j];
						hall->fork[forkid].zjh[desk].seat[i].card_num++;
					}
					length = pack4073(buf,type,hall->fork[forkid].zjh[desk].seat[1].card_change,num);
				}
				//排序
				sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);
				//减少牌
				for (j=0;j<num;j++)
				{
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_change[j]);
				}
				//结果通知
				UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
			}
			
			break;
	}

	//换牌完成 清空
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		hall->fork[forkid].zjh[desk].seat[i].card_change[0] = 0;
	}
}

void getMJChangeCards(int forkid,int desk,int seat,short int *card,short int num)
{
	//获取可交换的牌
	int i,j,k;
	short int tmp,color[4];
	short int color_num[4];

	for (i=0;i<num;i++)
	{
		color[i] = i;
		color_num[i] = 0;
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		color_num[hall->fork[forkid].zjh[desk].seat[seat].card[i]/10]++; 
	}

	for (i=0;i<num-1;i++)
	{
		for (j=0;j<num-1-i;j++)
		{
			if (color_num[j] > color_num[j+1])
			{
				tmp = color_num[j];
				color_num[j] = color_num[j+1];
				color_num[j+1] = tmp;

				tmp = color[j];
				color[j] = color[j+1];
				color[j+1] = tmp;
			}
		}
	}
	k = 0;
	for (i=0;i<num;i++)
	{
		if (color_num[i] > 2)
		{
			//找到花色
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[seat].card_num;j++)
			{
				if (hall->fork[forkid].zjh[desk].seat[seat].card[j]/10 == color[i])
				{
					card[k] = hall->fork[forkid].zjh[desk].seat[seat].card[j];
					k++;

					if (k == num)
					{
						return;
					}
				}
			}
		}
	}
}

void setMJChangeCards(char *buf,int forkid,int desk,int seat,short int *card,short int num)
{
	int i,j,k;
	for (i=0;i<num;i++)
	{
		printf("seat %d setMJChangeCards card = %d\n",seat,card[i]);
	}
	if (hall->fork[forkid].zjh[desk].seat[seat].card_change[0] > 0)
	{
		//i = pack2073(buf,-2,card,num);
		//UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,i);
		printf("seat %d 已经选择过换牌\n",seat);
		return;
	}

	for (i=0;i<num;i++)
	{
		if (card[i]/10 != card[0]/10)
		{
			printf("seat %d 三张花色不一致\n",seat);
			i = pack2073(buf,-1,card,num);
			UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,i);
			return;
		}
	}
	
	k = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		for (j=k;j<num;j++)
		{
			if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == card[j])
			{
				//找到牌
				k++;
				break;
			}
		}

		if (k == num)
		{
			break;
		}
	}

	if (k < num)
	{
		//牌没全找到
		printf("seat %d 牌没全找到\n",seat);
		i = pack2073(buf,0,card,num);
		UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,i);
		return;
	}

	//牌全部找到
	for (i=0;i<num;i++)
	{
		hall->fork[forkid].zjh[desk].seat[seat].card_change[i] = card[i];
	}

	//广播
	i = pack2073(buf,1,card,num);
	UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,i);

	i = pack3073(buf,seat);
	AllSeatSend(forkid,desk,buf,i);
}

void MJCheckTing(int forkid,int desk)
{
	//查听
	short int ting_cards[9];
	int i,j;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].ranking == 0)
		{
			//未胡
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].card[j]/10 + 1 == hall->fork[forkid].zjh[desk].seat[i].lack)
				{
					//无叫
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 4;
					break;
				}
			}

			if (j == hall->fork[forkid].zjh[desk].seat[i].card_num)
			{
				//
				if (is_ting_mj(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num,ting_cards) == 0)
				{
					//未听
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 4;
				}
				else
				{
					//有听
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 3;
				}
			}
		}
	}
}

void MJCheckTingZhu(int forkid,int desk)
{
	//退税 查大叫 花猪
	short int seat_ting[4];		//已听 k2
	short int seat_no_ting[4];	//未听 k3
	short int seat_pig[4];		//花猪 k4

	short int ting_cards[9];
	short int i,j,k,k1=0,k2=0,k3=0,k4=0,num,times_max,site_max,seat;
	
	long long int gold_tmp,gold_ting[4],gold_max;

	for (i=0;i<4;i++)
	{
		//printf("seat %d 总输赢 %lld\n",i,hall->fork[forkid].zjh[desk].gold[i]);
		seat_ting[i] = -1;
		seat_no_ting[i] =-1;
		seat_pig[i] = -1;
		gold_ting[i] = 0;
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].ranking == 0)
		{
			//未胡
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].card[j]/10 + 1 == hall->fork[forkid].zjh[desk].seat[i].lack)
				{
					//花猪
					seat_pig[k4] = i;
					k4++;
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 6;
					break;
				}
			}

			if (j == hall->fork[forkid].zjh[desk].seat[i].card_num)
			{
				//没有花猪
				num = is_ting_mj(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num,ting_cards);
				if (num == 0)
				{
					//未听
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 4;
					seat_no_ting[k3] = i;
					k3++;
				}
				else
				{
					//有听
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 3;
					seat_ting[k2] = i;
					k2++;
					times_max = 0;
					site_max = 0;
					for (j=0;j<num;j++)
					{
						hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = ting_cards[j];
						MJ_getType(forkid,desk,i,hall->fork[forkid].zjh[desk].seat[i].win[0].card_type,&(hall->fork[forkid].zjh[desk].seat[i].win[0].geng),&(hall->fork[forkid].zjh[desk].seat[i].win[0].times));
						
						if (hall->fork[forkid].zjh[desk].seat[i].card_num == 1)
						{
							//金勾钓
							hall->fork[forkid].zjh[desk].seat[i].win[0].jg = 1;
							hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
						}

						if (hall->fork[forkid].zjh[desk].seat[i].win[0].times >= hall->fork[forkid].zjh[desk].mj_times)
						{
							//满番
							times_max = hall->fork[forkid].zjh[desk].seat[i].win[0].times;
							break;
						}
						else if (hall->fork[forkid].zjh[desk].seat[i].win[0].times > times_max)
						{
							times_max = hall->fork[forkid].zjh[desk].seat[i].win[0].times;
							site_max = j;
						}
					}
					//
					if (times_max != hall->fork[forkid].zjh[desk].seat[i].win[0].times)
					{
						hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = ting_cards[site_max];
						MJ_getType(forkid,desk,i,hall->fork[forkid].zjh[desk].seat[i].win[0].card_type,&(hall->fork[forkid].zjh[desk].seat[i].win[0].geng),&(hall->fork[forkid].zjh[desk].seat[i].win[0].times));
					}

					if (hall->fork[forkid].zjh[desk].seat[i].win[0].times >= hall->fork[forkid].zjh[desk].mj_times)
					{
						//满番
						hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
					}

					gold_ting[i] = hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times);
					if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money < gold_ting[i])
					{
						gold_ting[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					}
					//
				}
			}
		}
		else if (hall->fork[forkid].zjh[desk].seat[i].ranking < 4)
		{
			//已胡
			k1++;
		}
	}
	/*
	printf("------------------------------查大叫------------------------------------\n");
	printf("k1=%d,k2=%d,k3=%d,k4=%d\n",k1,k2,k3,k4);
	printf("--------------------\n");
	for (i=0;i<k2;i++)
	{
		printf("%d 已听 %lld\n",seat_ting[i],gold_ting[seat_ting[i]]);
	}
	printf("--------------------\n");
	for (i=0;i<k3;i++)
	{
		printf("%d 未听\n",seat_no_ting[i]);
	}
	printf("--------------------\n");
	for (i=0;i<k4;i++)
	{
		printf("%d 花猪\n",seat_pig[i]);
	}
	printf("--------------------\n");
	*/
	//未听 先退税
	
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].win[0].flag == 4 || hall->fork[forkid].zjh[desk].seat[i].win[0].flag == 6)
		{
			//未听牌 或 花猪
			
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].num_gang;j++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].gang[j].flag != 3 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money > 0)
				{
					//非过手杠 且 有钱 退税
					for (k=0;k<3;k++)
					{
						//printf("----seat = %d gold=%lld------\n",hall->fork[forkid].zjh[desk].seat[i].gang[j].seat[k],hall->fork[forkid].zjh[desk].seat[i].gang[j].gold[k]);
						if (hall->fork[forkid].zjh[desk].seat[i].gang[j].seat[k] >= 0 && hall->fork[forkid].zjh[desk].seat[i].gang[j].gold[k] > 0)
						{
							seat = hall->fork[forkid].zjh[desk].seat[i].gang[j].seat[k];
							if (hall->fork[forkid].zjh[desk].seat[seat].ranking != 4)
							{
								//未认输
								if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money <= hall->fork[forkid].zjh[desk].seat[i].gang[j].gold[k])
								{
									gold_tmp = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
								
									hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money += gold_tmp;
									hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money = 0;

									hall->fork[forkid].zjh[desk].gold_new[seat] = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
									hall->fork[forkid].zjh[desk].gold[seat] += gold_tmp;

									hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
									hall->fork[forkid].zjh[desk].gold[i] -= gold_tmp;
									
									break;
								}
								else
								{
									gold_tmp = hall->fork[forkid].zjh[desk].seat[i].gang[j].gold[k];
									
									hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money += gold_tmp;
									hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money -= gold_tmp;

									hall->fork[forkid].zjh[desk].gold_new[seat] = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
									hall->fork[forkid].zjh[desk].gold[seat] += gold_tmp;

									hall->fork[forkid].zjh[desk].gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
									hall->fork[forkid].zjh[desk].gold[i] -= gold_tmp;
								}

								//printf("i=%d seat %d 收税 %lld 总输赢 %lld\n",i,seat,gold_tmp,hall->fork[forkid].zjh[desk].gold[seat]);
								//printf("i=%d seat %d 退税 %lld 总输赢 %lld\n",i,i,gold_tmp,hall->fork[forkid].zjh[desk].gold[i]);
								//printf("\n");
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			//
		}
		
	}
	//printf("------------------------------------------\n");
	
	for (i=0;i<k4;i++)
	{
		if (hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money == 0)
		{
			continue;
		}
		//赔钱判断
		if (hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money < (k1+k2+k3)*hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].mj_times))
		{
			gold_tmp = hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money/(k1+k2+k3);
			hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money = 0;
		}
		else
		{
			gold_tmp = hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].mj_times);
			hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money -= (k1+k2+k3)*gold_tmp;
		}

		hall->fork[forkid].zjh[desk].gold_new[seat_pig[i]] = hall->user[hall->fork[forkid].zjh[desk].seat[seat_pig[i]].user].money;
		hall->fork[forkid].zjh[desk].gold[seat_pig[i]] -= (k1+k2+k3)*gold_tmp;

		//printf("seat %d 花猪 总赔 %lld 总输赢 %lld \n",seat_pig[i],gold_tmp*(k1+k2+k3),hall->fork[forkid].zjh[desk].gold[seat_pig[i]]);

		for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
		{
			if (hall->fork[forkid].zjh[desk].seat[j].ranking != 4 && hall->fork[forkid].zjh[desk].seat[j].win[0].flag != 6)
			{
				//未认输 且 非花猪
				hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money += gold_tmp;

				hall->fork[forkid].zjh[desk].gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
				hall->fork[forkid].zjh[desk].gold[j] += gold_tmp;
				//printf("seat %d 收到花猪赔偿 %lld 总输赢%lld\n",j,gold_tmp,hall->fork[forkid].zjh[desk].gold[j]);
			}
		}
		
	}

	//查大叫
	gold_max = 0;
	for (i=0;i<4;i++)
	{
		gold_max += gold_ting[i];
	}
	//printf("#######gold_max=%lld\n",gold_max);
	if (k2 > 0 && k3 > 0)
	{
		for (i=0;i<k3;i++)
		{
			if (hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money == 0)
			{
				continue;
			}
			//赔钱判断
			if (hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money < gold_max)
			{
				gold_tmp = hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money;
				hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money = 0;
			}
			else
			{
				gold_tmp = gold_max;
				hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money -= gold_tmp;
			}
			

			hall->fork[forkid].zjh[desk].gold_new[seat_no_ting[i]] = hall->user[hall->fork[forkid].zjh[desk].seat[seat_no_ting[i]].user].money;
			hall->fork[forkid].zjh[desk].gold[seat_no_ting[i]] -= gold_tmp;
			//printf("seat %d 被查大叫 总赔 %lld 总输赢%lld\n",seat_no_ting[i],gold_tmp,hall->fork[forkid].zjh[desk].gold[seat_no_ting[i]]);
			for (j=0;j<k2;j++)
			{
				hall->user[hall->fork[forkid].zjh[desk].seat[seat_ting[j]].user].money += gold_ting[seat_ting[j]]*gold_tmp/gold_max;

				hall->fork[forkid].zjh[desk].gold_new[seat_ting[j]] = hall->user[hall->fork[forkid].zjh[desk].seat[seat_ting[j]].user].money;
				hall->fork[forkid].zjh[desk].gold[seat_ting[j]] += gold_ting[seat_ting[j]]*gold_tmp/gold_max;
				//查叫状态修改
				hall->fork[forkid].zjh[desk].seat[seat_ting[j]].win[0].flag = 5;
				hall->fork[forkid].zjh[desk].seat[seat_ting[j]].win[0].seat[i] = seat_no_ting[i];
				hall->fork[forkid].zjh[desk].seat[seat_ting[j]].win[0].gold[i] = gold_ting[seat_ting[j]]*gold_tmp/gold_max;

				//printf("seat %d 收到查大叫赔偿 %lld 总输赢%lld\n",seat_ting[j],gold_ting[seat_ting[j]]*gold_tmp/gold_max,hall->fork[forkid].zjh[desk].gold[seat_ting[j]]);
			}
			//
			//
		}
	}
	//
}

//玩家操作检查
int MJActionCheck(char *buf,int forkid,int desk)
{
	long long int gold_tmp,gold[4],gold_new[4],gold_win[4],gold_gang=0;
	short int last_card,num,lose=0,turn;
	int i,j,k1,k2,flag=0,length;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].flag_win == 1 || hall->fork[forkid].zjh[desk].seat[i].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[i].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[i].flag_chi == 1)
		{
			//有人未操作
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].flag_win == 3 || hall->fork[forkid].zjh[desk].seat[i].flag_peng == 3 || hall->fork[forkid].zjh[desk].seat[i].flag_gang == 3 || hall->fork[forkid].zjh[desk].seat[i].flag_chi == 3)
		{
			//有操作
			break;
		}
	}

	if (i == hall->fork[forkid].zjh[desk].seat_max)
	{
		//没有任何操作
		return 0;
	}

	turn = hall->fork[forkid].zjh[desk].turn;
	if (hall->fork[forkid].zjh[desk].seat[turn].card_out > 0)
	{
		last_card = hall->fork[forkid].zjh[desk].seat[turn].card_out;
		hall->fork[forkid].zjh[desk].seat[turn].card_out = 0;
	}
	else
	{
		last_card = hall->fork[forkid].zjh[desk].seat[turn].card_gang;
	}
	//全部已操作
	for (i=0;i<3;i++)
	{
		//杠钱
		gold_gang += hall->fork[forkid].zjh[desk].seat[turn].gang[hall->fork[forkid].zjh[desk].seat[turn].num_gang-1].gold[i];
	}
	
	flag = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		gold[i] = 0;
		gold_new[i] = 0;
		gold_win[i] = 0;
		if (hall->fork[forkid].zjh[desk].seat[i].flag_win == 3)
		{
			flag++;
		}
	}
	//
	k1 = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (i!=turn && hall->fork[forkid].zjh[desk].seat[i].flag_win == 3)
		{
			hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = last_card;
			MJ_getType(forkid,desk,i,hall->fork[forkid].zjh[desk].seat[i].win[0].card_type,&(hall->fork[forkid].zjh[desk].seat[i].win[0].geng),&(hall->fork[forkid].zjh[desk].seat[i].win[0].times));
			
			if (hall->fork[forkid].zjh[desk].seat[turn].ba_gang > 0)
			{
				//抢杠胡
				hall->fork[forkid].zjh[desk].seat[turn].ba_gang = 2;
				
				hall->fork[forkid].zjh[desk].seat[i].win[0].gh = 2;
				hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
			}
			else if (hall->fork[forkid].zjh[desk].seat[turn].card_gang > 0)
			{
				//接杠上炮
				hall->fork[forkid].zjh[desk].seat[i].win[0].gh = 3;
				hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
				//呼叫转移结算
				gold[i] += gold_gang/flag;
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold_gang/flag;
				
				gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
				hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
				hall->fork[forkid].zjh[desk].gold[i] += gold[i];
				//
				k1++;
			}
			else
			{
				//接炮
				if (hall->fork[forkid].zjh[desk].mj_td == 1)
				{
					//有天地胡规则
					if (hall->fork[forkid].zjh[desk].card_num == 55)
					{
						for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
						{
							if (hall->fork[forkid].zjh[desk].seat[j].num_gang > 0 || hall->fork[forkid].zjh[desk].seat[j].num_peng > 0)
							{
								break;
							}
						}

						if (j == hall->fork[forkid].zjh[desk].seat_max)
						{
							//地胡
							hall->fork[forkid].zjh[desk].seat[i].win[0].td = 2;
							hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
						}
					}
				}
				
			}

			hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 1;
			hall->fork[forkid].zjh[desk].seat[i].win[0].seat[0] = turn;
			
			hall->fork[forkid].zjh[desk].seat[turn].lose_num++;
			
			if (hall->fork[forkid].zjh[desk].seat[i].card_num == 1)
			{
				//金勾钓
				hall->fork[forkid].zjh[desk].seat[i].win[0].jg = 1;
				hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
			}

			if (hall->fork[forkid].zjh[desk].card_num == 0)
			{
				//海底
				hall->fork[forkid].zjh[desk].seat[i].win[0].hd = 1;
				hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
			}

			if (hall->fork[forkid].zjh[desk].seat[i].win[0].times > hall->fork[forkid].zjh[desk].mj_times)
			{
				//3番封顶
				hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
			}
			//
			gold_win[i] = hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times);
			if (gold_win[i] > hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money)
			{
				gold_win[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
			}
			//
		}
	}

	if (k1 > 0)
	{
		//有呼叫转移
		hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money -= gold_gang;
		gold[turn] = -gold_gang;
		gold_new[turn] = hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money;
		hall->fork[forkid].zjh[desk].gold_new[turn] = gold_new[turn];
		hall->fork[forkid].zjh[desk].gold[turn] += gold[turn];
		for (i=0;i<3;i++)
		{
			hall->fork[forkid].zjh[desk].seat[turn].gang[hall->fork[forkid].zjh[desk].seat[turn].num_gang-1].gold[i] = 0;
		}
		length = pack3081(buf,4,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
		AllSeatSend(forkid,desk,buf,length);
	}

	//有胡，吃碰杠全作废
	if (flag > 0)
	{
		if (flag > 1)
		{
			//一炮多响
			for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[i].ranking > 0 && hall->fork[forkid].zjh[desk].seat[i].ranking < 4)
				{
					break;
				}
			}

			if (i == hall->fork[forkid].zjh[desk].seat_max)
			{
				//还没有人胡牌
				hall->fork[forkid].zjh[desk].banker = turn;
			}
		}

		gold_tmp = 0;
		for (i=0;i<4;i++)
		{
			gold[i] = gold_win[i];
			gold_tmp += gold_win[i];
		}
		
		if (gold_tmp > 0)
		{
			//放炮
			//if (1 || gold_tmp >= hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money)
			if (gold_tmp >= hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money)
			{
				lose = 1;
				//输家钱不够
				for (i=0;i<4;i++)
				{
					if (gold_win[i] > 0)
					{
						gold[i] = gold_win[i]*hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money/gold_tmp;

						hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold[i];
						hall->fork[forkid].zjh[desk].seat[i].win[0].seat[0] = turn;
						hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = gold[i];
						gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
						hall->fork[forkid].zjh[desk].gold[i] += gold[i];

						hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = gold[i];
					}
				}
				
				gold[turn] = -hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money;
				hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money = 0;
				gold_new[turn] = hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money;
				hall->fork[forkid].zjh[desk].gold_new[turn] = gold_new[turn];
				hall->fork[forkid].zjh[desk].gold[turn] += gold[turn];
			}
			else
			{
				for (i=0;i<4;i++)
				{
					if (gold_win[i] > 0)
					{
						gold[i] = gold_win[i];
						hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold[i];

						hall->fork[forkid].zjh[desk].seat[i].win[0].seat[0] = turn;
						hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = gold[i];
						gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
						hall->fork[forkid].zjh[desk].gold[i] += gold[i];

						hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = gold[i];
					}
				}

				gold[turn] = -gold_tmp;
				hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money -= gold_tmp;
				gold_new[turn] = hall->user[hall->fork[forkid].zjh[desk].seat[turn].user].money;
				hall->fork[forkid].zjh[desk].gold_new[turn] = gold_new[turn];
				hall->fork[forkid].zjh[desk].gold[turn] += gold[turn];
			}
		}
		
		//
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].flag_win == 3)
			{
				//胡牌
				//to do...
				num = 0;
				k1 = 0;
				for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
				{
					if (hall->fork[forkid].zjh[desk].seat[j].ranking > 0)
					{
						//已胡牌
						k1++;
					}
					else if (i!=j && hall->fork[forkid].zjh[desk].seat[j].status == 3)
					{
						//非自己 未胡牌 且 未认输 
						//hall->fork[forkid].zjh[desk].seat[i].win[0].seat[num] = j;
						num++;
					}
				}
				hall->fork[forkid].zjh[desk].seat[i].ranking = k1+1;

				if (i == turn)
				{
					//自摸
					if (hall->fork[forkid].zjh[desk].seat[i].ranking == 1)
					{
						//首胡成为庄家
						hall->fork[forkid].zjh[desk].banker = i;
					}
					//广播
					hall->fork[forkid].zjh[desk].seat[i].win[0].flag = 2;
					if (hall->fork[forkid].zjh[desk].seat[i].card_in <= 0)
					{
						hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = hall->fork[forkid].zjh[desk].seat[i].card[rand()%14];
						//hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = 4;
					}
					else
					{
						hall->fork[forkid].zjh[desk].seat[i].win[0].card_win = hall->fork[forkid].zjh[desk].seat[i].card_in;
						
					}
					length = pack3076(buf,i,7,i,hall->fork[forkid].zjh[desk].seat[i].win[0].card_win,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,hall->fork[forkid].zjh[desk].seat[i].ranking);
					AllSeatSend(forkid,desk,buf,length);


					MJ_getType(forkid,desk,i,hall->fork[forkid].zjh[desk].seat[i].win[0].card_type,&(hall->fork[forkid].zjh[desk].seat[i].win[0].geng),&(hall->fork[forkid].zjh[desk].seat[i].win[0].times));
					//printf("get type times = %d\n",hall->fork[forkid].zjh[desk].seat[i].win[0].times);

					if (hall->fork[forkid].zjh[desk].mj_td == 1)
					{
						//有天地胡规则
						if (hall->fork[forkid].zjh[desk].card_num == 55)
						{
							//天胡
							hall->fork[forkid].zjh[desk].seat[i].win[0].td = 1;
							hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
						}
						else if (hall->fork[forkid].zjh[desk].card_num >= 52)
						{
							for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
							{
								if (hall->fork[forkid].zjh[desk].seat[j].num_gang > 0 || hall->fork[forkid].zjh[desk].seat[j].num_peng > 0)
								{
									break;
								}
							}

							if (j == hall->fork[forkid].zjh[desk].seat_max)
							{
								//地胡
								hall->fork[forkid].zjh[desk].seat[i].win[0].td = 2;
								hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
							}
						}
					}
					

					//printf("自摸 times = %d card_gang=%d\n",hall->fork[forkid].zjh[desk].seat[i].win[0].times,hall->fork[forkid].zjh[desk].seat[i].card_gang);
					if (hall->fork[forkid].zjh[desk].seat[i].card_gang > 0)
					{
						//杠上花
						hall->fork[forkid].zjh[desk].seat[i].win[0].gh = 1;
						hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
					}
					//printf("杠上花 times = %d\n",hall->fork[forkid].zjh[desk].seat[i].win[0].times);
					if (hall->fork[forkid].zjh[desk].seat[i].card_num == 1)
					{
						//金勾钓
						hall->fork[forkid].zjh[desk].seat[i].win[0].jg = 1;
						hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
					}

					if (hall->fork[forkid].zjh[desk].card_num == 0)
					{
						//海底
						hall->fork[forkid].zjh[desk].seat[i].win[0].hd = 1;
						hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
					}

					if (hall->fork[forkid].zjh[desk].mj_zm == 2)
					{
						//自摸加番
						hall->fork[forkid].zjh[desk].seat[i].win[0].times++;
					}

					if (hall->fork[forkid].zjh[desk].seat[i].win[0].times > hall->fork[forkid].zjh[desk].mj_times)
					{
						//3番封顶
						hall->fork[forkid].zjh[desk].seat[i].win[0].times = hall->fork[forkid].zjh[desk].mj_times;
					}
					//
					
					//结算
					for (j=0;j<4;j++)
					{
						gold[j] = 0;
						gold_new[j] = 0;
					}
					
					if (hall->fork[forkid].zjh[desk].seat_dg >= 0 && hall->fork[forkid].zjh[desk].mj_dg == 2)
					{
						//点杠上花，算点炮
						if (hall->fork[forkid].zjh[desk].mj_zm == 1)
						{
							//自摸加底
							gold_tmp = hall->fork[forkid].zjh[desk].bottom*((1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times) + 1);
						}
						else
						{
							gold_tmp = hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times);
						}

						if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money < gold_tmp)
						{
							gold_tmp = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						}

						j = hall->fork[forkid].zjh[desk].seat_dg;

						hall->fork[forkid].zjh[desk].seat[i].win[0].seat[0] = j;
						hall->fork[forkid].zjh[desk].seat[j].lose_num++;
						
						//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < gold_tmp)
						if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < gold_tmp)
						{
							lose = 1;
							gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
							gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
							
							hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
							hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;
						}
						else
						{
							gold[i] += gold_tmp;
							gold[j] -= gold_tmp;
							
							hall->fork[forkid].zjh[desk].seat[i].win[0].gold[0] = gold_tmp;
							hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold_tmp;
							hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= gold_tmp;
						}
						gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
						hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
						hall->fork[forkid].zjh[desk].gold[j] += gold[j];

					}
					else
					{
						//自摸
						if (hall->fork[forkid].zjh[desk].mj_zm == 1)
						{
							//自摸加底
							gold_tmp = hall->fork[forkid].zjh[desk].bottom*((1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times) + 1)*num;
						}
						else
						{
							gold_tmp = hall->fork[forkid].zjh[desk].bottom*(1<<hall->fork[forkid].zjh[desk].seat[i].win[0].times)*num;
						}

						if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money < gold_tmp)
						{
							gold_tmp = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
						}

						gold_tmp /= num;
					
						k1 = 0;
						for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
						{
							if (i!=j && hall->fork[forkid].zjh[desk].seat[j].status == 3 && hall->fork[forkid].zjh[desk].seat[j].ranking == 0)
							{
								//输赢结算
								hall->fork[forkid].zjh[desk].seat[i].win[0].seat[k1] = j;
								hall->fork[forkid].zjh[desk].seat[j].lose_num++;
								
								//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < gold_tmp)
								if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < gold_tmp)
								{
									lose = 1;
									gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									
									hall->fork[forkid].zjh[desk].seat[i].win[0].gold[k1] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;
								}
								else
								{
									gold[i] += gold_tmp;
									gold[j] -= gold_tmp;
									
									hall->fork[forkid].zjh[desk].seat[i].win[0].gold[k1] = gold_tmp;
									hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += gold_tmp;
									hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= gold_tmp;
								}
								k1++;
								gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
								hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
								hall->fork[forkid].zjh[desk].gold[j] += gold[j];
							}
						}
						//
					}
					
					//printf("自摸 gold[%d]=%lld\n",i,gold[i]);
					gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
					hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
					hall->fork[forkid].zjh[desk].gold[i] += gold[i];

					//length = pack3081(buf,1,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
					//AllSeatSend(forkid,desk,buf,length);
					hall->fork[forkid].zjh[desk].seat_dg = -1;
					hall->fork[forkid].zjh[desk].seat[i].flag_win = 0;
					break;
					//
				}
				else
				{
					//点炮
					if (flag == 1)
					{
						if (hall->fork[forkid].zjh[desk].seat[i].ranking == 1)
						{
							//首胡成为庄家
							hall->fork[forkid].zjh[desk].banker = i;
						}
					}
					//广播
					length = pack3076(buf,i,4,turn,hall->fork[forkid].zjh[desk].seat[i].win[0].card_win,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,hall->fork[forkid].zjh[desk].seat[i].ranking);
					AllSeatSend(forkid,desk,buf,length);

					hall->fork[forkid].zjh[desk].turn = i;
					hall->fork[forkid].zjh[desk].seat[i].flag_win = 0;
					
					length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
					AllSeatSend(forkid,desk,buf,length);
					//
				}
			}
			
		}

		length = pack3081(buf,1,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
		AllSeatSend(forkid,desk,buf,length);
		
		//清理所有人吃碰杠状态
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			hall->fork[forkid].zjh[desk].seat[i].flag_gang = 0;
			hall->fork[forkid].zjh[desk].seat[i].flag_peng = 0;
			hall->fork[forkid].zjh[desk].seat[i].flag_chi = 0;
		}

		if (lose == 1)
		{
			//有人输光 游戏暂停
			hall->fork[forkid].zjh[desk].flag = 9;
			hall->fork[forkid].zjh[desk].start_time = hall->time_now + 45;

			length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,45);
			AllSeatSend(forkid,desk,buf,length);
		}
		return 1;
	}
	else if (flag == 0)
	{
		//没有人胡牌
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].flag_peng == 3)
			{
				//碰
				length = pack4077(buf,5,i,last_card,hall->fork[forkid].zjh[desk].seat[i].card_num);
				AllSeatSend(forkid,desk,buf,length);

				printf("碰牌成功 seat = %d card=%d card_num=%d\n",i,last_card,hall->fork[forkid].zjh[desk].seat[i].card_num);
				delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),last_card);
				delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),last_card);
				printf("碰牌完成 seat = %d card_num = %d\n",i,hall->fork[forkid].zjh[desk].seat[i].card_num);
				length = pack3076(buf,i,2,turn,last_card,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);

				hall->fork[forkid].zjh[desk].seat[i].card_peng[hall->fork[forkid].zjh[desk].seat[i].num_peng] = last_card;
				hall->fork[forkid].zjh[desk].seat[i].num_peng++;
				//hall->fork[forkid].zjh[desk].seat[turn].card_out = 0;
				
				hall->fork[forkid].zjh[desk].seat[i].begin_time = hall->time_now + MJ_ACTION_TIME;
				hall->fork[forkid].zjh[desk].seat[i].flag_peng = 0;
				hall->fork[forkid].zjh[desk].turn = i;
				//过手胡重置
				hall->fork[forkid].zjh[desk].seat[i].win_times = -1;

				//广播
				AllSeatSend(forkid,desk,buf,length);

				length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
				AllSeatSend(forkid,desk,buf,length);
				break;
			}
			else if (hall->fork[forkid].zjh[desk].seat[i].flag_gang == 3)
			{
				//杠 1暗杠，2巴杠，3过手杠，4点杠
				if (hall->fork[forkid].zjh[desk].seat[i].ba_gang == 2)
				{
					hall->fork[forkid].zjh[desk].seat[i].ba_gang = 0;
					return 0;
				}
				if (i != turn)
				{
					hall->fork[forkid].zjh[desk].seat_dg = turn;

					length = pack4077(buf,4,i,last_card,hall->fork[forkid].zjh[desk].seat[i].card_num);
					AllSeatSend(forkid,desk,buf,length);
					//点杠
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),last_card);
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),last_card);
					delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),last_card);

					hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].flag = 4;
					hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].card_gang = last_card;
					hall->fork[forkid].zjh[desk].seat[i].num_gang++;
					//hall->fork[forkid].zjh[desk].seat[turn].card_out = 0;
					hall->fork[forkid].zjh[desk].seat[turn].point_gang++;

					hall->fork[forkid].zjh[desk].seat[i].win_times = -1;

					length = pack3076(buf,i,6,turn,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);
					AllSeatSend(forkid,desk,buf,length);
					//杠钱结算
					printf("杠钱结算\n");
					for (j=0;j<4;j++)
					{
						gold[j] = 0;
						gold_new[j] = 0;
					}
					j = turn;

					hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].seat[0] = j;
					
					//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
					if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
					{
						lose = 1;
						gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
						gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
						hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
						hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;

						hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[0] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
					}
					else
					{
						gold[i] += hall->fork[forkid].zjh[desk].bottom*2;
						gold[j] -= hall->fork[forkid].zjh[desk].bottom*2;
						hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= hall->fork[forkid].zjh[desk].bottom*2;
						hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->fork[forkid].zjh[desk].bottom*2;

						hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[0] = hall->fork[forkid].zjh[desk].bottom*2;
					}
					gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
					gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;

					hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
					hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
					hall->fork[forkid].zjh[desk].gold[j] += gold[j];
					hall->fork[forkid].zjh[desk].gold[i] += gold[i];

					printf("seat %d 接杠 赢钱 %lld 总输赢 %lld\n",i,gold[i],hall->fork[forkid].zjh[desk].gold[i]);
					printf("seat %d 点杠 输钱 %lld 总输赢 %lld\n",j,gold[j],hall->fork[forkid].zjh[desk].gold[j]);
					
					length = pack3081(buf,0,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
					AllSeatSend(forkid,desk,buf,length);
				}
				else
				{

					if (hall->fork[forkid].zjh[desk].seat[i].card_gang == hall->fork[forkid].zjh[desk].seat[i].card_in)
					{
						last_card = hall->fork[forkid].zjh[desk].seat[i].card_gang;
						num = 1;
						for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
						{
							if (last_card == hall->fork[forkid].zjh[desk].seat[i].card[j])
							{
								num++;
							}
						}

						if (num < 4)
						{
							for (k1=0;k1<hall->fork[forkid].zjh[desk].seat[i].num_peng;k1++)
							{
								if (hall->fork[forkid].zjh[desk].seat[i].card_gang == hall->fork[forkid].zjh[desk].seat[i].card_peng[k1])
								{
									break;
								}
							}

							if (k1 == hall->fork[forkid].zjh[desk].seat[i].num_peng)
							{
								//没有找到 杠失败
								return 0;
							}

							//巴杠
							hall->fork[forkid].zjh[desk].seat_dg = -1;
							length = pack4077(buf,2,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].seat[i].card_num);
							AllSeatSend(forkid,desk,buf,length);

							if (hall->fork[forkid].zjh[desk].seat[i].ba_gang == 0)
							{
								hall->fork[forkid].zjh[desk].seat[i].ba_gang = 1;	
								for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
								{
									if (i != j)
									{
										MJCheckOther(buf,forkid,desk,j);
									}
								}
								return 0;
							}
							hall->fork[forkid].zjh[desk].seat[i].ba_gang = 0;
							//删除碰
							for (k1=0;k1<hall->fork[forkid].zjh[desk].seat[i].num_peng;k1++)
							{
								if (hall->fork[forkid].zjh[desk].seat[i].card_peng[k1] == hall->fork[forkid].zjh[desk].seat[i].card_gang)
								{
									hall->fork[forkid].zjh[desk].seat[i].card_peng[k1] = 0;
									for (k2=k1;k2<hall->fork[forkid].zjh[desk].seat[i].num_peng-1;k2++)
									{
										hall->fork[forkid].zjh[desk].seat[i].card_peng[k2] = hall->fork[forkid].zjh[desk].seat[i].card_peng[k2+1];
										hall->fork[forkid].zjh[desk].seat[i].card_peng[k2+1] = 0;
									}
									break;
								}
							}
							hall->fork[forkid].zjh[desk].seat[i].num_peng--;
							//生成杠
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].flag = 2;
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].card_gang = hall->fork[forkid].zjh[desk].seat[i].card_gang;
							hall->fork[forkid].zjh[desk].seat[i].num_gang++;

							length = pack3076(buf,i,3,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);
							AllSeatSend(forkid,desk,buf,length);
							//杠钱结算
							//printf("杠钱结算\n");
							for (j=0;j<4;j++)
							{
								gold[j] = 0;
								gold_new[j] = 0;
							}
							k1 = 0;
							for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
							{
								if (i!=j && hall->fork[forkid].zjh[desk].seat[j].status == 3 && hall->fork[forkid].zjh[desk].seat[j].ranking == 0)
								{
									hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].seat[k1] = j;
									
									//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom)
									if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom)
									{
										lose = 1;
										gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									}
									else
									{
										gold[i] += hall->fork[forkid].zjh[desk].bottom;
										gold[j] -= hall->fork[forkid].zjh[desk].bottom;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= hall->fork[forkid].zjh[desk].bottom;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->fork[forkid].zjh[desk].bottom;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->fork[forkid].zjh[desk].bottom;
									}
									k1++;
									gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
									hall->fork[forkid].zjh[desk].gold[j] += gold[j];
									//printf("seat %d 点杠 输钱 %lld 总输赢 %lld\n",j,gold[j],hall->fork[forkid].zjh[desk].gold[j]);
								}
							}

							gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
							hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
							hall->fork[forkid].zjh[desk].gold[i] += gold[i];

							//printf("seat %d 接杠 赢钱 %lld 总输赢 %lld\n",i,gold[i],hall->fork[forkid].zjh[desk].gold[i]);
							length = pack3081(buf,0,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
							AllSeatSend(forkid,desk,buf,length);
						}
						else
						{
							//暗杠
							printf("暗杠\n");
							hall->fork[forkid].zjh[desk].seat_dg = -1;
							length = pack4077(buf,1,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].seat[i].card_num);
							AllSeatSend(forkid,desk,buf,length);
							//暗杠
							//摸牌先补进
							hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[i].card_in;
							hall->fork[forkid].zjh[desk].seat[i].card_num++;
							//理牌
							sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);

							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);

							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].flag = 1;
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].card_gang = hall->fork[forkid].zjh[desk].seat[i].card_gang;
							hall->fork[forkid].zjh[desk].seat[i].num_gang++;

							length = pack3076(buf,i,5,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);
							AllSeatSend(forkid,desk,buf,length);
							//杠钱结算
							//printf("seat %d 杠钱结算 杠 %d\n",i,hall->fork[forkid].zjh[desk].seat[i].card_gang);
							for (j=0;j<4;j++)
							{
								gold[j] = 0;
								gold_new[j] = 0;
							}
							k1 = 0;
							for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
							{
								if (i!=j && hall->fork[forkid].zjh[desk].seat[j].ranking == 0)
								{
									hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].seat[k1] = j;
									
									//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
									if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
									{
										lose = 1;
										gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									}
									else
									{
										gold[i] += hall->fork[forkid].zjh[desk].bottom*2;
										gold[j] -= hall->fork[forkid].zjh[desk].bottom*2;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= hall->fork[forkid].zjh[desk].bottom*2;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->fork[forkid].zjh[desk].bottom*2;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->fork[forkid].zjh[desk].bottom*2;
									}
									k1++;
									gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
									hall->fork[forkid].zjh[desk].gold[j] += gold[j];
									//printf("seat %d 点杠 输钱 %lld 总输赢 %lld\n",j,gold[j],hall->fork[forkid].zjh[desk].gold[j]);
								}
							}
							
							gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
							hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
							hall->fork[forkid].zjh[desk].gold[i] += gold[i];
							//printf("seat %d 接杠 赢钱 %lld 总输赢 %lld\n",i,gold[i],hall->fork[forkid].zjh[desk].gold[i]);

							length = pack3081(buf,0,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
							AllSeatSend(forkid,desk,buf,length);
							
							
						}
					}
					else
					{
						last_card = hall->fork[forkid].zjh[desk].seat[i].card_gang;
						num = 1;
						for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
						{
							if (last_card == hall->fork[forkid].zjh[desk].seat[i].card[j])
							{
								num++;
							}
						}

						if (num < 4)
						{
							
							for (k1=0;k1<hall->fork[forkid].zjh[desk].seat[i].num_peng;k1++)
							{
								if (hall->fork[forkid].zjh[desk].seat[i].card_gang == hall->fork[forkid].zjh[desk].seat[i].card_peng[k1])
								{
									break;
								}
							}

							if (k1 == hall->fork[forkid].zjh[desk].seat[i].num_peng)
							{
								//没有找到 杠失败
								return 0;
							}
							//过手杠
							printf("过手杠 手牌交换前:");
							for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
							{
								printf("%d,",hall->fork[forkid].zjh[desk].seat[i].card[j]);
							}
							printf("\n-----------------------------\n");
							
						
							//过手杠
							if (hall->fork[forkid].zjh[desk].seat[i].ba_gang == 0)
							{
								hall->fork[forkid].zjh[desk].seat_dg = -1;
								length = pack4077(buf,3,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].seat[i].card_num);
								AllSeatSend(forkid,desk,buf,length);
								//摸牌先插入，交换出杠牌
								hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[i].card_in;
								hall->fork[forkid].zjh[desk].seat[i].card_num++;
								//理牌
								sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);
								delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
								
								printf("过手杠 手牌交换后:");
								for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
								{
									printf("%d,",hall->fork[forkid].zjh[desk].seat[i].card[j]);
								}
								printf("\n-----------------------------\n");

								hall->fork[forkid].zjh[desk].seat[i].ba_gang = 1;	
								for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
								{
									if (i != j)
									{
										MJCheckOther(buf,forkid,desk,j);
									}
								}
								
								return 0;
							}
							hall->fork[forkid].zjh[desk].seat[i].ba_gang = 0;
							
							//删除碰
							for (k1=0;k1<hall->fork[forkid].zjh[desk].seat[i].num_peng;k1++)
							{
								if (hall->fork[forkid].zjh[desk].seat[i].card_peng[k1] == hall->fork[forkid].zjh[desk].seat[i].card_gang)
								{
									hall->fork[forkid].zjh[desk].seat[i].card_peng[k1] = 0;
									for (k2=k1;k2<hall->fork[forkid].zjh[desk].seat[i].num_peng-1;k2++)
									{
										hall->fork[forkid].zjh[desk].seat[i].card_peng[k2] = hall->fork[forkid].zjh[desk].seat[i].card_peng[k2+1];
										hall->fork[forkid].zjh[desk].seat[i].card_peng[k2+1] = 0;
									}
									break;
								}
							}
							hall->fork[forkid].zjh[desk].seat[i].num_peng--;
							//生成杠
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].flag = 3;
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].card_gang = hall->fork[forkid].zjh[desk].seat[i].card_gang;
							hall->fork[forkid].zjh[desk].seat[i].num_gang++;

							length = pack3076(buf,i,8,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);
							AllSeatSend(forkid,desk,buf,length);
							printf("过手杠完成 手牌:");
							for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
							{
								printf("%d,",hall->fork[forkid].zjh[desk].seat[i].card[j]);
							}
							printf("\n-----------------------------\n");
						}
						else
						{
							//暗杠
							printf("暗杠\n");
							hall->fork[forkid].zjh[desk].seat_dg = -1;
							length = pack4077(buf,1,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].seat[i].card_num);
							AllSeatSend(forkid,desk,buf,length);
							//暗杠
							//摸牌先补进
							if (hall->fork[forkid].zjh[desk].seat[i].card_in > 0)
							{
								hall->fork[forkid].zjh[desk].seat[i].card[hall->fork[forkid].zjh[desk].seat[i].card_num] = hall->fork[forkid].zjh[desk].seat[i].card_in;
								hall->fork[forkid].zjh[desk].seat[i].card_num++;
							}
							
							//理牌
							sort_asc(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);

							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);
							delMJCardsByCard(hall->fork[forkid].zjh[desk].seat[i].card,&(hall->fork[forkid].zjh[desk].seat[i].card_num),hall->fork[forkid].zjh[desk].seat[i].card_gang);

							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].flag = 1;
							hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang].card_gang = hall->fork[forkid].zjh[desk].seat[i].card_gang;
							hall->fork[forkid].zjh[desk].seat[i].num_gang++;

							length = pack3076(buf,i,5,i,hall->fork[forkid].zjh[desk].seat[i].card_gang,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[i].card_num,0);
							AllSeatSend(forkid,desk,buf,length);
							//杠钱结算
							//printf("seat %d 杠钱结算 杠 %d\n",i,hall->fork[forkid].zjh[desk].seat[i].card_gang);
							for (j=0;j<4;j++)
							{
								gold[j] = 0;
								gold_new[j] = 0;
							}
							k1 = 0;
							for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
							{
								if (i!=j && hall->fork[forkid].zjh[desk].seat[j].ranking == 0)
								{
									hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].seat[k1] = j;
									
									//if (1 || hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
									if (hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money < hall->fork[forkid].zjh[desk].bottom*2)
									{
										lose = 1;
										gold[i] += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										gold[j] -= hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money = 0;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									}
									else
									{
										gold[i] += hall->fork[forkid].zjh[desk].bottom*2;
										gold[j] -= hall->fork[forkid].zjh[desk].bottom*2;
										hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money -= hall->fork[forkid].zjh[desk].bottom*2;
										hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money += hall->fork[forkid].zjh[desk].bottom*2;

										hall->fork[forkid].zjh[desk].seat[i].gang[hall->fork[forkid].zjh[desk].seat[i].num_gang-1].gold[k1] = hall->fork[forkid].zjh[desk].bottom*2;
									}
									k1++;
									gold_new[j] = hall->user[hall->fork[forkid].zjh[desk].seat[j].user].money;
									hall->fork[forkid].zjh[desk].gold_new[j] = gold_new[j];
									hall->fork[forkid].zjh[desk].gold[j] += gold[j];
									//printf("seat %d 点杠 输钱 %lld 总输赢 %lld\n",j,gold[j],hall->fork[forkid].zjh[desk].gold[j]);
								}
							}
							
							gold_new[i] = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
							hall->fork[forkid].zjh[desk].gold_new[i] = gold_new[i];
							hall->fork[forkid].zjh[desk].gold[i] += gold[i];
							//printf("seat %d 接杠 赢钱 %lld 总输赢 %lld\n",i,gold[i],hall->fork[forkid].zjh[desk].gold[i]);

							length = pack3081(buf,0,hall->fork[forkid].zjh[desk].seat_max,gold,gold_new);
							AllSeatSend(forkid,desk,buf,length);
							
						}

					}
					
					hall->fork[forkid].zjh[desk].seat[i].card_in = 0;
				}

				hall->fork[forkid].zjh[desk].turn = i;
				hall->fork[forkid].zjh[desk].seat[i].begin_time = hall->time_now + MJ_ACTION_TIME;

				hall->fork[forkid].zjh[desk].seat[i].flag_gang = 0;
				
				length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
				AllSeatSend(forkid,desk,buf,length);
				//摸一张牌
				MJGetOneCard(buf,forkid,desk,i);
				break;
			}
		}
	}

	if (lose == 1)
	{
		//有人输光 游戏暂停
		hall->fork[forkid].zjh[desk].flag = 9;
		hall->fork[forkid].zjh[desk].start_time = hall->time_now + MJ_FREEZE_TIME;

		length = pack2072(buf,hall->fork[forkid].zjh[desk].flag,MJ_FREEZE_TIME);
		AllSeatSend(forkid,desk,buf,length);
		return 1;
	}

	return 0;
}

//检查所有座位操作情况
int MJCheckAllSeat(int forkid,int desk)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{

		if (hall->fork[forkid].zjh[desk].seat[i].flag_win != 0 || hall->fork[forkid].zjh[desk].seat[i].flag_peng != 0 || hall->fork[forkid].zjh[desk].seat[i].flag_gang != 0)
		{
			return 0;
		}
	}

	return 1;
}

//庄家起手或摸牌后检查手牌
void MJCheckMyself(char *buf,int forkid,int desk,int seat)
{
	if (hall->fork[forkid].zjh[desk].seat[seat].ranking > 0)
	{
		return;
	}
	int length;
	hall->fork[forkid].zjh[desk].seat[seat].flag_gang = isMJGang(forkid,desk,seat);
	hall->fork[forkid].zjh[desk].seat[seat].flag_win = isMJWin(forkid,desk,seat);
	
	if (hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
	{
		hall->fork[forkid].zjh[desk].seat[seat].begin_time = hall->time_now + MJ_ACTION_TIME;
		length = pack2075(buf,forkid,desk,seat,0,hall->fork[forkid].zjh[desk].seat[seat].card_in);
		UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
}

//其他人出牌后检查手牌
void MJCheckOther(char *buf,int forkid,int desk,int seat)
{
	if (hall->fork[forkid].zjh[desk].seat[seat].ranking > 0)
	{
		return;
	}
	int length;
	hall->fork[forkid].zjh[desk].seat[seat].flag_peng = isMJPeng(forkid,desk,seat);
	hall->fork[forkid].zjh[desk].seat[seat].flag_gang = isMJGang(forkid,desk,seat);
	hall->fork[forkid].zjh[desk].seat[seat].flag_win = isMJWin(forkid,desk,seat);
	
	if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
	{
		hall->fork[forkid].zjh[desk].seat[seat].begin_time = hall->time_now + MJ_ACTION_TIME;
		length = pack2075(buf,forkid,desk,seat,1,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out);
		UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
}

//是否有碰
short int isMJPeng(int forkid,int desk,int seat)
{
	int num;
	if (seat != hall->fork[forkid].zjh[desk].turn && hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out/10 + 1 != hall->fork[forkid].zjh[desk].seat[seat].lack)
	{
		num = getNumSameCard(hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out,forkid,desk,seat);
		if (num >= 2)
		{
			//有碰
			return 1;
		}
	}
	return 0;
}

//是否有杠
short int isMJGang(int forkid,int desk,int seat)
{
	short int i,j,num=0;
	short int last_card;
	if (hall->fork[forkid].zjh[desk].card_num <= 0)
	{
		return 0;
	}

	if (seat == hall->fork[forkid].zjh[desk].turn)
	{
		//轮到自己
		if (hall->fork[forkid].zjh[desk].seat[seat].card_in > 0)
		{
			//有摸牌
			for (i=0;i<MJ_CPK_MAX;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[seat].card_peng[i] == hall->fork[forkid].zjh[desk].seat[seat].card_in)
				{
					//有巴杠
					return 1;
				}
			}

			for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
			{
				for (j=0;j<MJ_CPK_MAX;j++)
				{
					if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == hall->fork[forkid].zjh[desk].seat[seat].card_peng[j])
					{
						//过手杠
						return 1;
					}
				}
			}
		}

		if (hall->fork[forkid].zjh[desk].seat[seat].card_in/10 + 1 != hall->fork[forkid].zjh[desk].seat[seat].lack)
		{
			num=1;
			for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == hall->fork[forkid].zjh[desk].seat[seat].card_in)
				{
					num++;
				}

				if (num == 4)
				{
					return 1;
				}
			}
		}
		

		num=1;
		last_card = hall->fork[forkid].zjh[desk].seat[seat].card[0];
		for (i=1;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
		{
			if (last_card == hall->fork[forkid].zjh[desk].seat[seat].card[i] && last_card/10 + 1 != hall->fork[forkid].zjh[desk].seat[seat].lack)
			{
				num++;
			}
			else 
			{
				last_card = hall->fork[forkid].zjh[desk].seat[seat].card[i];
				num = 1;
			}

			if (num == 4)
			{
				//有暗杠
				return 1;
			}
		}
	}
	else
	{
		if (hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out/10 + 1 != hall->fork[forkid].zjh[desk].seat[seat].lack)
		{
			num = getNumSameCard(hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out,forkid,desk,seat);
			if (num == 3)
			{
				//有点杠
				return 1;
			}
		}
		
	}
	

	return 0;
}

//是否有胡
short int isMJWin(int forkid,int desk,int seat)
{
	short int i,j,num,flag,times,geng;
	short int type[8];
	short int card[20];

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card[i]/10 + 1 == hall->fork[forkid].zjh[desk].seat[seat].lack)
		{
			//未缺
			return 0;
		}
		card[i] = hall->fork[forkid].zjh[desk].seat[seat].card[i];
	}
	

	if (seat != hall->fork[forkid].zjh[desk].turn)
	{
		
		if (hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out > 0)
		{
			card[i] = hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out;
		}
		else
		{
			card[i] = hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_gang;
		}
		i++;
	}
	else if (hall->fork[forkid].zjh[desk].seat[seat].card_in > 0)
	{
		//自己有摸牌
		card[i] = hall->fork[forkid].zjh[desk].seat[seat].card_in;
		i++;
	}
	
	//排序
	sort_asc(card,i);
	//M*AAA + N*ABC + CC
	
	flag = is_win_mj(card,i);
	if (flag == 1)
	{
		//有胡
		num = i;
		times = 0;
		geng = check_type_mj(card,i,type,is_jiang(forkid,desk,seat),is_me_qing(forkid,desk,seat),is_zhong_zhang(forkid,desk,seat),is_qing(forkid,desk,seat),is_yaojiu(forkid,desk,seat));
		for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
		{
			for (j=0;j<num;j++)
			{
				if (card[j] == hall->fork[forkid].zjh[desk].seat[seat].card_peng[i])
				{
					//在碰上 根+1
					geng++;
					break;
				}
			}
		}

		geng += hall->fork[forkid].zjh[desk].seat[seat].num_gang;
		//顺序是清一色 对对胡 七对 全幺九 中张 将 门清
		if (type[0] == 1)
		{
			//清一色
			times+=2;
		}

		if (type[1] == 1)
		{
			//对对胡
			times++;
		}

		if (type[2] == 1)
		{
			//七对
			times+=2;
		}
		
		if (hall->fork[forkid].zjh[desk].mj_yjjd == 1)
		{
			if (type[3] == 1)
			{
				//全19
				times+=3;
			}

			if (type[5] == 1)
			{
				//将
				if (type[2] == 1)
				{
					times++;
				}
				else
				{
					times+=2;
				}
				
			}
		}
		else
		{
			type[3] = 0;
			type[5] = 0;
		}

		if (hall->fork[forkid].zjh[desk].mj_mqzz == 1)
		{
			if (type[4] == 1)
			{
				//中张
				times++;
			}

			if (type[6] == 1)
			{
				//门清
				times++;
			}
		}
		else
		{
			type[4] = 0;
			type[6] = 0;
		}

		times += geng;

		if (times > hall->fork[forkid].zjh[desk].seat[seat].win_times)
		{
			hall->fork[forkid].zjh[desk].seat[seat].win_times = times;
			return 1;
		}
	}

	return 0;
}

//摸牌
void MJGetOneCard(char *buf,int forkid,int desk,int seat)
{
	int length;

	hall->fork[forkid].zjh[desk].seat[seat].card_in = hall->fork[forkid].zjh[desk].cards[hall->fork[forkid].zjh[desk].card_num-1];
	hall->fork[forkid].zjh[desk].cards[hall->fork[forkid].zjh[desk].card_num-1] = 0;
	hall->fork[forkid].zjh[desk].card_num--;
	
	//过手胡重置
	hall->fork[forkid].zjh[desk].seat[seat].win_times = -1;

	hall->fork[forkid].zjh[desk].seat[seat].begin_time = hall->time_now + MJ_ACTION_TIME;
	
	length = pack2078(buf,(short int)seat,hall->fork[forkid].zjh[desk].card_num,0);
	OtherSeatSend(forkid,desk,seat,buf,length);

	length = pack2078(buf,(short int)seat,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[seat].card_in);
	UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	printf("seat %d 摸牌成功 card=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card_in);
	MJCheckMyself(buf,forkid,desk,seat);
}

//出牌
void MJSetOneCard(char *buf,int forkid,int desk,int seat)
{
	int i,j;
	if (hall->fork[forkid].zjh[desk].seat[seat].card_out <= 0)
	{
		//未确认要出的牌
		return;
	}

	if (hall->fork[forkid].zjh[desk].seat[seat].card_in == 0)
	{
		//没有摸牌
		for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == hall->fork[forkid].zjh[desk].seat[seat].card_out)
			{
				printf("seat %d 手牌->出牌成功 card=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card[i]);
				hall->fork[forkid].zjh[desk].seat[seat].card[i] = 0;
				for (j=i;j<hall->fork[forkid].zjh[desk].seat[seat].card_num-1;j++)
				{
					hall->fork[forkid].zjh[desk].seat[seat].card[j] = hall->fork[forkid].zjh[desk].seat[seat].card[j+1];
					hall->fork[forkid].zjh[desk].seat[seat].card[j+1] = 0;
				}
				
				break;
			}
		}

		if (i == hall->fork[forkid].zjh[desk].seat[seat].card_num)
		{
			printf("seat %d card_in = 0 出牌失败! card_out=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card_out);
		}
		else
		{
			hall->fork[forkid].zjh[desk].seat[seat].card_num--;
		}
		
	}
	else
	{
		//有摸牌
		if (hall->fork[forkid].zjh[desk].seat[seat].card_in == hall->fork[forkid].zjh[desk].seat[seat].card_out)
		{
			//出牌等于摸牌
			//hall->fork[forkid].zjh[desk].seat[seat].card_in = 0;
			printf("seat %d 出牌等于摸牌 card_in=%d card_out=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card_in,hall->fork[forkid].zjh[desk].seat[seat].card_out);
		}
		else
		{
			//交换
			for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
			{
				if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == hall->fork[forkid].zjh[desk].seat[seat].card_out)
				{
					printf("seat %d 手牌->交换成功 card_in=%d,card_out=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card_in,hall->fork[forkid].zjh[desk].seat[seat].card_out);
					hall->fork[forkid].zjh[desk].seat[seat].card[i] = hall->fork[forkid].zjh[desk].seat[seat].card_in;
					//hall->fork[forkid].zjh[desk].seat[seat].card_in = 0;
					//理牌
					sort_asc(hall->fork[forkid].zjh[desk].seat[seat].card,hall->fork[forkid].zjh[desk].seat[seat].card_num);
					break;
				}
			}

			if (i == hall->fork[forkid].zjh[desk].seat[seat].card_num)
			{
				printf("seat %d 交换失败! card_out=%d\n",seat,hall->fork[forkid].zjh[desk].seat[seat].card_out);
			}
			
		}
	}
	hall->fork[forkid].zjh[desk].seat_dg = -1;

	hall->fork[forkid].zjh[desk].seat[seat].card_in = 0;
	hall->fork[forkid].zjh[desk].seat[seat].begin_time = 0;

	j = pack2077(buf,1,hall->fork[forkid].zjh[desk].seat[seat].card_out);
	UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,j);
	//广播出牌
	j = pack3077(buf,forkid,desk,seat);
	AllSeatSend(forkid,desk,buf,j);

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (i != seat)
		{
			MJCheckOther(buf,forkid,desk,i);
		}
	}
}

//获取已知相同牌个数
int getNumSameCard(short int card,int forkid,int desk,int seat)
{
	int i,num=0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == card)
		{
			num++;
		}
	}
	return num;
}

//获取一张可出的牌
short int getMJCardOut(int forkid,int desk,int seat)
{
	int i;
	//优先定缺牌
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card[i]/10 + 1 == hall->fork[forkid].zjh[desk].seat[seat].lack)
		{
			return hall->fork[forkid].zjh[desk].seat[seat].card[i];
		}
	}
	//摸牌打牌
	if (hall->fork[forkid].zjh[desk].seat[seat].card_in > 0)
	{
		return hall->fork[forkid].zjh[desk].seat[seat].card_in;
	}
	//顺位选择
	i = hall->fork[forkid].zjh[desk].seat[seat].card_num-1;
	return hall->fork[forkid].zjh[desk].seat[seat].card[i];
}

//是否全部已定缺
int MJIsTab(int forkid,int desk)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		switch (hall->fork[forkid].zjh[desk].flag)
		{
			case 1:
				if (hall->fork[forkid].zjh[desk].seat[i].card_change[0] == 0)
				{
					return 0;
				}
				break;
			case 3:
				if (hall->fork[forkid].zjh[desk].seat[i].lack == 0)
				{
					return 0;
				}
				break;
			case 9:
				if (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money <= 0 && hall->fork[forkid].zjh[desk].seat[i].ranking == 0)
				{
					return 0;
				}
			default:
				break;
		}
	}
	return 1;
}

//自动定缺
void setMJLack(int forkid,int desk)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].lack == 0)
		{
			hall->fork[forkid].zjh[desk].seat[i].lack = getMJLack(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].seat[i].card_num);
		}
	}
}

//获取定缺花色
short int getMJLack(short int *card,short int card_num)
{
	short int i,a=0,b=0,c=0;
	for (i=0;i<card_num;i++)
	{
		if (card[i] < 10)
		{
			a++;
		}
		else if (card[i] < 20)
		{
			b++;
		}
		else 
		{
			c++;
		}
	}

	i = c;
	if (b < i)
	{
		i = b;
	}

	if (a < i)
	{
		i = a;
	}

	if (i == a)
	{
		return 1;
	}
	else if (i == b)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

//玩家定缺请求
int MJ_lack(char *buf,int forkid,int desk,int seat,short int parm)
{
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	int length;
	if (parm < 1 || parm > 3 || hall->fork[forkid].zjh[desk].seat[seat].lack != 0)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		length = pack2074(buf,0,parm);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}

	hall->fork[forkid].zjh[desk].seat[seat].lack = parm;
	length = pack2074(buf,1,parm);
	UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	length = pack3074(buf,1,seat);
	AllSeatSend(forkid,desk,buf,length);
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	return 1;
}

//玩家操作请求
int MJ_action(char *buf,int forkid,int desk,int seat,short int type,short int card)
{
	int i,length;
	if (hall->fork[forkid].zjh[desk].flag == 9)
	{
		length = packReturnFlag(buf,2076,-1);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
	else if (hall->fork[forkid].zjh[desk].flag != 4)
	{
		length = packReturnFlag(buf,2076,-3);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
	switch (type)
	{
		case 0:
			//过
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_chi == 1)
			{
				hall->fork[forkid].zjh[desk].seat[seat].flag_chi = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_win = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_peng = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_gang = 0;
				
				length = packReturnFlag(buf,2076,1);
				UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		case 2:
			//碰
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1)
			{
				//有碰
				hall->fork[forkid].zjh[desk].seat[seat].flag_chi = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_win = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_gang = 0;
				
				hall->fork[forkid].zjh[desk].seat[seat].flag_peng = 3;
				length = packReturnFlag(buf,2076,1);
				UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		case 3:
			//杠
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 && card > 0)
			{
				if (seat == hall->fork[forkid].zjh[desk].turn)
				{
					//轮到自己
					if (hall->fork[forkid].zjh[desk].seat[seat].card_in != card)
					{
						for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
						{
							if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == card)
							{
								break;
							}
						}

						if (i == hall->fork[forkid].zjh[desk].seat[seat].card_num)
						{
							sem_post(&(hall->fork[forkid].zjh[desk].lock));
							return 1;
						}
					}
				}
				else
				{
					if (hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out != card)
					{
						sem_post(&(hall->fork[forkid].zjh[desk].lock));
						return 1;
					}
				}
				//有杠
				hall->fork[forkid].zjh[desk].seat[seat].flag_chi = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_win = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_peng = 0;
				
				hall->fork[forkid].zjh[desk].seat[seat].card_gang = card;
				hall->fork[forkid].zjh[desk].seat[seat].flag_gang = 3;
				length = packReturnFlag(buf,2076,1);
				UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		case 4:
		case 5:
			//胡
			//printf("请求胡 seat=%d\n",seat);
			sem_wait(&(hall->fork[forkid].zjh[desk].lock));
			if (hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
			{
				//有胡
				//printf("请求胡成功\n");
				hall->fork[forkid].zjh[desk].seat[seat].flag_chi = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_peng = 0;
				hall->fork[forkid].zjh[desk].seat[seat].flag_gang = 0;
				
				hall->fork[forkid].zjh[desk].seat[seat].flag_win = 3;
				length = packReturnFlag(buf,2076,1);
				UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
			}
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		default:
			return 0;
	}

}

//请求认输
void MJ_giveUp(char *buf,int forkid,int desk,int seat)
{
	int length;
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].zjh[desk].seat[seat].ranking == 0 && hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money == 0)
	{
		//hall->fork[forkid].zjh[desk].seat[seat].status = 4;
		hall->fork[forkid].zjh[desk].seat[seat].ranking = 4;
		length = pack3082(buf,seat);
		AllSeatSend(forkid,desk,buf,length);
	}
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
}

//请求出牌
int MJ_cardOut(char *buf,int forkid,int desk,int seat,short int card)
{
	int i,length;
	
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].zjh[desk].flag == 9)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		length = pack2077(buf,-1,card);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
	else if (hall->fork[forkid].zjh[desk].flag != 4)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		length = pack2077(buf,-3,card);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
	
	if (card <= 0 || hall->fork[forkid].zjh[desk].turn != seat || hall->fork[forkid].zjh[desk].seat[seat].card_out != 0)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		length = pack2077(buf,0,card);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}

	if (hall->fork[forkid].zjh[desk].seat[seat].flag_win != 0 || hall->fork[forkid].zjh[desk].seat[seat].flag_gang != 0 || hall->fork[forkid].zjh[desk].seat[seat].flag_peng != 0 || hall->fork[forkid].zjh[desk].seat[seat].flag_chi != 0)
	{
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		length = pack2077(buf,-2,card);
		return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
	}
	printf("seat %d 请求出牌 card=%d\n",seat,card);
	if (card == hall->fork[forkid].zjh[desk].seat[seat].card_in)
	{
		hall->fork[forkid].zjh[desk].seat[seat].card_out = card;
		MJSetOneCard(buf,forkid,desk,seat);
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		return 1;
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card[i] == card)
		{
			//找到牌
			hall->fork[forkid].zjh[desk].seat[seat].card_out = card;
			MJSetOneCard(buf,forkid,desk,seat);
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		}
	}
	
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	length = pack2077(buf,0,card);
	return UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
}

//传递令牌
int MJPassOn(char *buf,int forkid,int desk)
{
	printf("@@@@@@@@@@@@@@@@@@@@@令牌传递!@@@@@@@@@@@@@@@@@@@@@@@\n");
	short int turn;
	int i,length;
	turn = hall->fork[forkid].zjh[desk].turn;
	hall->fork[forkid].zjh[desk].seat[turn].ba_gang = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_gang = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_in = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_out = 0;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		turn++;
		if (turn >= hall->fork[forkid].zjh[desk].seat_max)
		{
			turn = 0;
		}

		if (hall->fork[forkid].zjh[desk].seat[turn].ranking == 0)
		{
			break;
		}
	}

	//游戏是否结束
	length = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].ranking > 0)
		{
			//已胡牌 | 已认输
			length++;
		}
	}
	//printf("length = %d,seat_max = %d\n",length,hall->fork[forkid].zjh[desk].seat_max);
	if (length >= hall->fork[forkid].zjh[desk].seat_max-1)
	{
		//三家已胡牌或认输 游戏直接结束
		//查叫
		MJCheckTing(forkid,desk);

		//广播
		length = pack3080(buf,forkid,desk);
		AllSeatSend(forkid,desk,buf,length);
		
		hall->fork[forkid].zjh[desk].flag = 100;
		//init_desk(forkid,desk,0);
		return 1;
	}
	
	if (hall->fork[forkid].zjh[desk].card_num == 0)
	{
		//牌摸完了 游戏结束 退税 查大叫 查花猪
		MJCheckTingZhu(forkid,desk);
		//广播
		length = pack3080(buf,forkid,desk);
		AllSeatSend(forkid,desk,buf,length);

		hall->fork[forkid].zjh[desk].flag = 100;
		return 1;
	}

	hall->fork[forkid].zjh[desk].seat[turn].ba_gang = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_gang = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_in = 0;
	hall->fork[forkid].zjh[desk].seat[turn].card_out = 0;
	hall->fork[forkid].zjh[desk].turn = turn;
	

	length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,MJ_ACTION_TIME);
	AllSeatSend(forkid,desk,buf,length);
	
	MJGetOneCard(buf,forkid,desk,hall->fork[forkid].zjh[desk].turn);
	return 0;
}

void MJ_getType(int forkid,int desk,int seat,short int *type,short int *geng,short int *times)
{	
	short int i,j,num;
	short int card[14];
	if (hall->fork[forkid].zjh[desk].seat[seat].card_num == 14)
	{
		num = 14;
		for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
		{
			card[i] = hall->fork[forkid].zjh[desk].seat[seat].card[i];
		}
	}
	else
	{
		num = hall->fork[forkid].zjh[desk].seat[seat].card_num+1;
		for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
		{
			card[i] = hall->fork[forkid].zjh[desk].seat[seat].card[i];
		}
		card[i] = hall->fork[forkid].zjh[desk].seat[seat].win[0].card_win;
	}
	
	(*times) = 0;
	(*geng) = check_type_mj(card,num,type,is_jiang(forkid,desk,seat),is_me_qing(forkid,desk,seat),is_zhong_zhang(forkid,desk,seat),is_qing(forkid,desk,seat),is_yaojiu(forkid,desk,seat));
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
	{
		for (j=0;j<num;j++)
		{
			if (card[j] == hall->fork[forkid].zjh[desk].seat[seat].card_peng[i])
			{
				//在碰上 根+1
				(*geng)++;
				break;
			}
		}
	}

	(*geng) += hall->fork[forkid].zjh[desk].seat[seat].num_gang;
	//顺序是清一色 对对胡 七对 全幺九 中张 将 门清
	if (type[0] == 1)
	{
		//清一色
		(*times)+=2;
	}

	if (type[1] == 1)
	{
		//对对胡
		(*times)++;
	}

	if (type[2] == 1)
	{
		//七对
		(*times)+=2;
	}
	
	if (hall->fork[forkid].zjh[desk].mj_yjjd == 1)
	{
		if (type[3] == 1)
		{
			//全19
			(*times)+=3;
		}

		if (type[5] == 1)
		{
			//将
			if (type[2] == 1)
			{
				(*times)++;
			}
			else
			{
				(*times)+=2;
			}
			
		}
	}
	else
	{
		type[3] = 0;
		type[5] = 0;
	}

	if (hall->fork[forkid].zjh[desk].mj_mqzz == 1)
	{
		if (type[4] == 1)
		{
			//中张
			(*times)++;
		}

		if (type[6] == 1)
		{
			//门清
			(*times)++;
		}
	}
	else
	{
		type[4] = 0;
		type[6] = 0;
	}

	(*times) += (*geng);
}
//结算
void MJBalance(int forkid,int desk)
{
	//现金场游戏记录
	while (add_game_record(hall->fork[forkid].gameid,hall->fork[forkid].type,hall->fork[forkid].grade,hall->fork[forkid].zjh[desk].bottom,\
		hall->fork[forkid].zjh[desk].uid,hall->fork[forkid].zjh[desk].gold,hall->fork[forkid].zjh[desk].gold_old,hall->fork[forkid].zjh[desk].gold_new,hall->fork[forkid].zjh[desk].name,(int)(hall->time_now)) < 0)
	{
		sleep(1);
	}
}

short int is_jiang(int forkid,int desk,int seat)
{
	short int i;
	
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 != 2 && hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 != 5 && hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 != 8)
		{
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_gang;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 != 2 && hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 != 5 && hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 != 8)
		{
			return 0;
		}
	}

	return 1;
}

short int is_me_qing(int forkid,int desk,int seat)
{
	short int i;

	if (hall->fork[forkid].zjh[desk].seat[seat].num_peng > 0)
	{
		return 0;
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_gang;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].gang[i].flag != 1)
		{
			return 0;
		}
	}

	return 1;
}

short int is_zhong_zhang(int forkid,int desk,int seat)
{
	short int i;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 == 1 || hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 == 9)
		{
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_gang;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 == 1 || hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 == 9)
		{
			return 0;
		}
	}

	return 1;
}

short int is_qing(int forkid,int desk,int seat)
{
	short int i;
	short int color = hall->fork[forkid].zjh[desk].seat[seat].card[0]/10 + 1;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]/10 + 1 != color)
		{
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_gang;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang/10 +1 != color)
		{
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card[i]/10 +1 != color)
		{
			return 0;
		}
	}

	return 1;
}

short int is_yaojiu(int forkid,int desk,int seat)
{
	short int i;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_gang;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 != 1 && hall->fork[forkid].zjh[desk].seat[seat].gang[i].card_gang%10 != 9)
		{
			return 0;
		}
	}

	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].num_peng;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 != 1 && hall->fork[forkid].zjh[desk].seat[seat].card_peng[i]%10 != 9)
		{
			return 0;
		}
	}

	return 1;
}