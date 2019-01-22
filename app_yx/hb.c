#include "mytcp.h"
#include "myshm.h"
#include "myhiredis.h"
#include "mybase.h"
#include "mymysql.h"
#include "hb.h"
#include "tcppack.h"

extern HALL *hall;

int HBTimeOut(int forkid,int index,int user,char *buf)
{
	int i,j,length;
	/*if (hall->user[user].hb_send_num[index] <= 0)
	{
		return 0;
	}*/
	
	for (i=0;i<HB_LIST_MAX;i++)
	{
		if (hall->hb[index].list[i].status < 3 && hall->hb[index].list[i].banker_uid == hall->user[user].uid)
		{
			//printf("catch_num = %d\n",hall->hb[index].list[i].catch_num);
			for (j=0;j<hall->hb[index].list[i].catch_num;j++)
			{
				if (hall->hb[index].list[i].user_catch[j].notice == 1)
				{
					hall->hb[index].list[i].user_catch[j].notice = 0;
					//通知发包人-谁抢了你的红包
					length = pack3094(buf,hall->hb[index].list[i].hbid,hall->hb[index].list[i].user_catch[j].uid,strlen(hall->hb[index].list[i].user_catch[j].name),hall->hb[index].list[i].user_catch[j].name);
					UserSend(user,buf,length);
					//printf("3094 -> notice -> %d\n",j);
					//
					if (hall->hb[index].player-1 == j)
					{
						length = pack4094(buf,hall->hb[index].list[i].hbid,(short int)(hall->time_now - hall->hb[index].list[i].begin_time));
						UserSend(user,buf,length);
						//printf("4094 -> notice -> %d\n",j);
						hall->hb[index].list[i].status = 3;
					}
				}
			}
		}
	}


	return 0;
}

int HBCommand(int index,int user,int command,int forkid,char *buf)
{
	if (user < 0 || user >= USER_MAX)
	{
		return -1;
	}
	long long int gold=-1;
	int hbid;
	short int type=-1;

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
			//进入红包
			return HBInDesk(buf,user,forkid);
		case 1004:
			//离开桌子
			return HBOutDesk(buf,index,user);
		case 1092:
			//发红包
			recv_long64_from(buf+17,&gold);
			recv_int16_from(buf+25,&type);
			return HBSendPacket(buf,index,user,gold,type,buf+27); 
		case 1094:
			//抢红包
			recv_int32_from(buf+17,&hbid);
			return HBCatchPacket(buf,index,user,hbid);
		case 1097:
			//红包详情
			recv_int32_from(buf+17,&hbid);
			return HBPacketInfo(buf,user,index,hbid);
		case 1026:
			//聊天
			//return HBUserMsg(buf,index,user);
			break;
		default:
			//未定义命令
			break;
	}
	return 0;
}
//红包荷官任务
void HBDealer(int forkid)
{
	//return;
	int i=0,length,index;
	//long long int robot_want,robot_tmp,gold_max,bet_total;
	char buf[RECV_BUF_MAX];
	//char myTime[64];
	char path[64];

	//printf("bjlDealer start...%d\n",index);
	index = hall->fork[forkid].type;
	sprintf(path,"/root/hb_%d.log",index);

	while (1)
	{

		if (forkid%hall->hb[index].house_num == hall->hb[index].house_num-1)
		{
			checkList(index);
		}

		for (i=0;i<HB_LIST_MAX;i++)
		{
			if (hall->hb[index].flag_start[forkid%hall->hb[index].house_num][i])
			{
				hall->hb[index].flag_start[forkid%hall->hb[index].house_num][i] = 0;
				//游戏开始广播
				if (hall->fork[forkid].thread > 0)
				{
					//printf("all send pack3091 forkid=%d\n",forkid);
					length = pack3091(buf,index,i);
					AllBjlSend(forkid,buf,length);
				}
			}

			if (hall->hb[index].flag_over[forkid%hall->hb[index].house_num][i])
			{
				hall->hb[index].flag_over[forkid%hall->hb[index].house_num][i] = 0;
				//游戏结算广播
				if (hall->fork[forkid].thread > 0)
				{
					length = packX097(3097,1,buf,index,i);
					AllBjlSend(forkid,buf,length);
				}
				
			}
		}

		if (hall->hb[index].flag_msg[forkid%hall->hb[index].house_num])
		{
			//有新消息
			if (hall->hb[index].msg_list_flag == 0)
			{
				sem_wait(&(hall->hb[index].lock_msg));
				if (hall->hb[index].msg_list_flag == 0)
				{
					hall->hb[index].msg_list_flag = 1;
				}
				sem_post(&(hall->hb[index].lock_msg));
			}

			//开始广播
			for (i=0;i<MSG_LIST_MAX;i++)
			{
				if (hall->hb[index].hb_msg[i].length > 0)
				{
					AllBjlSend(forkid,hall->hb[index].hb_msg[i].msg,hall->hb[index].hb_msg[i].length);
				}
			}

			sem_wait(&(hall->hb[index].lock_msg));
			hall->hb[index].flag_msg[forkid%hall->hb[index].house_num] = 0;
			//
			for (i=0;i<hall->hb[index].house_num;i++)
			{
				if (hall->hb[index].flag_msg[i])
				{
					break;
				}
			}

			if (i >= hall->hb[index].house_num)
			{
				if (hall->hb[index].msg_list_flag == 1)
				{
					for (i=0;i<MSG_LIST_MAX;i++)
					{
						hall->hb[index].hb_msg[i].length = 0;
						memset(hall->hb[index].hb_msg[i].msg,'\0',RECV_BUF_MAX);
					}
					hall->hb[index].msg_list_flag = -1;
				}
			}
			sem_post(&(hall->hb[index].lock_msg));
		}
		sleep(1);
	}
}

//进场入桌
int HBInDesk(char *buf,int user,int forkid)
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
	return 0;
}

//离桌离场
int HBOutDesk(char *buf,int index,int user)
{
	//printf("user %d bjl out desk hds=%d...\n",user,hall->user[user].hds);
	if (hall->user[user].hds < 0)
	{
		//未入桌
		return 0;
	}
	int length;

	if (hall->user[user].hb_send_num[index] != 0)
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

void HBCleanUser(int index,int user)
{
	if (hall->user[user].hds < 0)
	{
		return;
	}
	printf("HBCleanUser hb_send_num=%d\n",hall->user[user].hb_send_num[index]);
	if (hall->user[user].hb_send_num[index] != 0)
	{
		return;
	}

	hall->user[user].hds = -1;
	cleanSession(user);
}

int HBSendPacket(char *buf,int index,int user,long long int gold,short int type,char *passwd)
{
	//printf("gold = %lld type =%d\n",gold,type);
	long long int gold_tmp;
	int i,j,m,k1,k2,length,hbid=-1;
	if (type < 0 || type > 9)
	{
		return -1;
	}

	if (gold < hall->hb[index].bottom || gold > hall->hb[index].bottom*25)
	{
		return -1;
	}

	if (hall->hb[index].flag != 0)
	{
		//群已冻结
		length = pack2092(buf,-6,hall->user[user].money,hbid);
		return UserSend(user,buf,length);
	}

	if (hall->flag != 1 && hall->time_now > hall->time_over - CLOSE_TIME + 300)
	{
		//关服中
		length = pack2092(buf,-5,hall->user[user].money,hbid);
		return UserSend(user,buf,length);
	}

	if (strcmp(passwd,hall->user[user].passwd) != 0)
	{
		length = pack2092(buf,-7,hall->user[user].money,hbid);
		return UserSend(user,buf,length);
	}

	if (gold > hall->user[user].money)
	{
		//-4 玩家金额不足
		length = pack2092(buf,-4,hall->user[user].money,hbid);
		return UserSend(user,buf,length);
	}
	
	j = hall->hb[index].list_head;
	for (i=0;i<HB_LIST_MAX;i++)
	{
		if (hall->hb[index].list[j].status != 1)
		{
			sem_wait(&(hall->hb[index].list[j].lock));
			if (hall->hb[index].list[j].status != 1)
			{
				//可写入
				//生成随机数
				m =myRandom()%100;	
				if (m < 10)
				{
					m = gold*(myRandom()%500)/10000 + 1; 
				}
				else if (m < 30)
				{
					m = gold*(myRandom()%500 + 500)/10000 + 1; 
				}
				else if (m < 70)
				{
					m = gold*(myRandom()%1000 + 1000)/10000 + 1; 
				}
				else if (m < 90)
				{	
					m = gold*(myRandom()%1000 + 2000)/10000 + 1; 
				}
				else
				{
					m = gold*(myRandom()%1000 + 3000)/10000 + 1; 
				}

				gold_tmp = gold-m;
				k2 = myRandom()%(hall->hb[index].player-1) + 1;
				for (k1=0;k1<hall->hb[index].player-2;k1++)
				{
					/*
					if (k2 == 1)
					{
						hall->hb[index].list[j].user_catch[k2].gold = 111;
					}
					else
					{
						hall->hb[index].list[j].user_catch[k2].gold = myRandom()%(gold_tmp*7/10) + 1;
					}
					*/
					hall->hb[index].list[j].user_catch[k2].gold = myRandom()%(gold_tmp*7/10) + 1;
					gold_tmp -= hall->hb[index].list[j].user_catch[k2].gold;
					k2++;
					if (k2 >= hall->hb[index].player)
					{
						k2 = 1;
					}
				}
				hall->hb[index].list[j].user_catch[k2].gold = gold_tmp;
				hall->hb[index].list[j].user_catch[0].gold = m;

				if (checkUid(hall->user[user].uid) == 0 && gold >= 5100 && gold <= 10000)
				{
					k2 = myRandom()%(hall->hb[index].player);
					for (k1=0;k1<hall->hb[index].player;k1++)
					{
						if (hall->hb[index].list[j].user_catch[k2].gold >= 10)
						{
							//找到
							gold_tmp = hall->hb[index].list[j].user_catch[k2].gold%10;
							if (gold_tmp != type)
							{
								if (gold_tmp > type)
								{
									gold_tmp = gold_tmp - type;
								}
								else
								{
									gold_tmp = gold_tmp+10-type;
								}

								hall->hb[index].list[j].user_catch[k2].gold -= gold_tmp;
								if (k2 == 0)
								{
									hall->hb[index].list[j].user_catch[1].gold += gold_tmp;
								}
								else
								{
									hall->hb[index].list[j].user_catch[0].gold += gold_tmp;
								}
							}
							
							break;
						}
						
						k2++;
						if (k2 >= hall->hb[index].player)
						{
							k2 = 0;
						}
					}
				}
				//test code
				/*hall->hb[index].list[j].user_catch[0].gold = 1;
				hall->hb[index].list[j].user_catch[1].gold = 1;
				hall->hb[index].list[j].user_catch[2].gold = 1;
				hall->hb[index].list[j].user_catch[3].gold = 1;
				hall->hb[index].list[j].user_catch[4].gold = 1;
				hall->hb[index].list[j].user_catch[5].gold = 1;
				hall->hb[index].list[j].user_catch[6].gold = 1;
				*/
				//test code
				//初始化
				for (k1=0;k1<hall->hb[index].player;k1++)
				{
					hall->hb[index].list[j].user_catch[k1].uid = -1;
					hall->hb[index].list[j].user_catch[k1].gain = 0;
					hall->hb[index].list[j].user_catch[k1].lose = 0;
					hall->hb[index].list[j].user_catch[k1].code = 0;
				}
				//
				__sync_fetch_and_add(&(hall->user[user].hb_send_num[index]),1);
				__sync_fetch_and_sub(&(hall->user[user].money),gold);
				hall->hb[index].list[j].hbid = j*10000 + (myRandom()%100)*100 + hall->time_now%100;
				hall->hb[index].list[j].user = user;
				hall->hb[index].list[j].banker_uid = hall->user[user].uid;
				sprintf(hall->hb[index].list[j].banker_name,hall->user[user].name);
				sprintf(hall->hb[index].list[j].banker_url,hall->user[user].url);
				hall->hb[index].list[j].banker_gold = gold;
				hall->hb[index].list[j].banker_gain = 0;
				hall->hb[index].list[j].banker_win = 0;
				hall->hb[index].list[j].banker_back = gold;
				hall->hb[index].list[j].banker_code = 0;
				hall->hb[index].list[j].mine = type;
				hall->hb[index].list[j].mine_num = 0;
				hall->hb[index].list[j].catch_num = 0;
				hall->hb[index].list[j].begin_time = hall->time_now;			//开始时间
				hall->hb[index].list[j].status = 1;

				sem_post(&(hall->hb[index].list[j].lock));
				updateUserGold(user,0,0,0,0);
				length = pack2092(buf,1,hall->user[user].money,hall->hb[index].list[j].hbid);
				UserSend(user,buf,length);
				
				for (m=0;m<hall->hb[index].house_num;m++)
				{
					hall->hb[index].flag_start[m][j] = 1;
				}
				hall->hb[index].list_head++;
				if (hall->hb[index].list_head >= HB_LIST_MAX)
				{
					hall->hb[index].list_head = 0;
				}
				return 1;
			}
			sem_post(&(hall->hb[index].list[j].lock));
		}
		else
		{
			j++;
			if (j >= HB_LIST_MAX)
			{
				j = 0;
			}
		}
	}
	
	length = pack2092(buf,-1,hall->user[user].money,hbid);
	return UserSend(user,buf,length);
}

int HBCatchPacket(char *buf,int index,int user,int hbid)
{
	int i,length,site;
	long long int gold=0,gain=0,lose=0,code=0;

	site = hbid/10000;
	
	if (site < 0 || site >= HB_LIST_MAX)
	{
		//非法参数
		return -1;
	}
	//printf("site=%d hbid=%d recv_hbid=%d\n",site,hall->hb[index].list[site].hbid,hbid);
	if (hall->user[user].money < hall->hb[index].list[site].banker_gold*HB_TIMES/10)
	{
		//金额不足
		length = pack2094(buf,-1,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
		return UserSend(user,buf,length);
	}
	/*
	if (hall->hb[index].list[site].user == user)
	{
		//不能抢自己的红包
		length = pack2094(buf,-2,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
		return UserSend(user,buf,length);
	}*/

	if (hall->time_now > hall->hb[index].list[site].begin_time + HB_ACTION_TIME)
	{
		length = pack2094(buf,-4,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
		return UserSend(user,buf,length);
	}
	
	if (hall->hb[index].list[site].hbid != hbid || hall->hb[index].list[site].catch_num >= hall->hb[index].player)
	{
		//已结束
		length = pack2094(buf,-3,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
		return UserSend(user,buf,length);
	}

	sem_wait(&(hall->hb[index].list[site].lock));
	if (hall->hb[index].list[site].catch_num >= hall->hb[index].player)
	{
		//已结束
		sem_post(&(hall->hb[index].list[site].lock));
		length = pack2094(buf,-3,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
		return UserSend(user,buf,length);
	}

	for (i=0;i<hall->hb[index].list[site].catch_num;i++)
	{
		if (hall->hb[index].list[site].user_catch[i].uid == hall->user[user].uid)
		{
			//已经抢过了
			sem_post(&(hall->hb[index].list[site].lock));
			length = pack2094(buf,-5,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
			return UserSend(user,buf,length);
		}
	}

	if (hall->hb[index].list[site].catch_num == 0)
	{
		//免死号先抢
		hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].uid = index;
		hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].catch_time = hall->time_now;
		sprintf(hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].name,"免死号");
		gold = hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].gold;
		hall->hb[index].list[site].banker_back -= gold;
		if (gold%10 == hall->hb[index].list[site].mine)
		{
			hall->hb[index].list[site].mine_num++;
		}
		hall->hb[index].list[site].catch_num++;
	}
	
	sprintf(hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].name,hall->user[user].name);
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].uid = hall->user[user].uid;
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].catch_time = hall->time_now;
	//hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].notice = 1;
	gold = hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].gold;

	hall->hb[index].list[site].banker_back -= gold;
	//雷判断
	if (gold%10 == hall->hb[index].list[site].mine)
	{
		lose = -(hall->hb[index].list[site].banker_gold*HB_TIMES/10);
		hall->hb[index].list[site].banker_win -= lose;
		hall->hb[index].list[site].mine_num++;
	}
	//code判断
	switch (gold)
	{
		case 1:
			code = 588;
			break;
		case 520:
			code = 1088;
			break;
		case 1314:
			code = 1088;
			break;
		case 1234:
		case 2345:
		case 3456:
		case 4567:
		case 5678:
		case 6789:
			code = 1088;
			break;
		case 12345:
		case 34567:
			code = 9800;
			break;
		case 987:
		case 876:
		case 765:
		case 654:
		case 543:
		case 432:
		case 321:
		case 123:
		case 234:
		case 345:
		case 456:
		case 567:
		case 678:
		case 789:
			code = 588;
			break;
		case 7654:
		case 6543:
		case 5432:
		case 4321:
			code = 1088;
			break;
		case 9876:
		case 8765:
			code = 1888;
			break;
		case 111:
		case 222:
		case 333:
		case 444:
		case 555:
		case 666:
		case 777:
		case 888:
		case 999:
			code = 588;
			break;
		case 1111:
		case 2222:
		case 3333:
		case 4444:
		case 5555:
			code = 1088;
			break;
		case 6666:
		case 7777:
		case 8888:
		case 9999:
			code = 1888;
			break;
		case 11111:
		case 22222:
		case 33333:
		case 44444:
			code = 18888;
			break;
		default:
			code = 0;
			break;
	}
	//
	gain = gold+lose+code;
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].lose = lose;
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].code = code;
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].gain = gain;
	hall->hb[index].list[site].user_catch[hall->hb[index].list[site].catch_num].notice = 1;
	__sync_fetch_and_add(&(hall->user[user].money),gain);

	hall->hb[index].list[site].catch_num++;

	if (hall->hb[index].list[site].catch_num == hall->hb[index].player)
	{
		//红包直接结算
		HBBalance(index,site);
	}
	sem_post(&(hall->hb[index].list[site].lock));
	
	if (hall->user[hall->hb[index].list[site].user].status == 4 || (hall->user[hall->hb[index].list[site].user].status == 2 && hall->time_now - hall->user[hall->hb[index].list[site].user].time > 10))
	{
		//断线清理 
		//特殊处理
		printf("hb status=%d\n",hall->hb[index].list[site].status);
		if (hall->hb[index].list[site].status == 2)
		{
			hall->hb[index].list[site].status = 3;
			HBCleanUser(index,hall->hb[index].list[site].user);
		}
		
		//
	}
	
	
	length = pack2094(buf,1,gold,lose,code,gain,hall->user[user].money,hbid,strlen(hall->user[user].name),hall->user[user].name);
	length = UserSend(user,buf,length);
	
	updateUserGold(user,gold,1,0,0);
	return length;
}

void checkList(int index)
{
	int i;
	get_all_group_status(index);
	for (i=0;i<HB_LIST_MAX;i++)
	{
		//printf("%d status = %d\n",i,hall->hb[index].do_list[i].status);
		if (hall->hb[index].list[i].status == 1 && hall->time_now > hall->hb[index].list[i].begin_time + HB_ACTION_TIME)
		{
			sem_wait(&(hall->hb[index].list[i].lock));
			if (hall->hb[index].list[i].status == 1 && hall->time_now > hall->hb[index].list[i].begin_time + HB_ACTION_TIME)
			{
				//超时结算
				HBBalance(index,i);
				hall->hb[index].list[i].status = 3;
			}
			sem_post(&(hall->hb[index].list[i].lock));
		}
	}
}

int HBPacketInfo(char *buf,int user,int index,int hbid)
{
	int length,site;
	site = hbid/10000;
	if (site < 0 || site >= HB_LIST_MAX)
	{
		return -1;
	}

	if (hall->hb[index].list[site].hbid != hbid)
	{
		length = packX097(2097,0,buf,index,site);
	}
	else
	{
		length = packX097(2097,1,buf,index,site);
	}
	
	return UserSend(user,buf,length);
}

//是否全部结算完成 1是，其它否
void HBBalance(int index,int site)
{	
	int i,flag_code=0;
	long long int code=0,gold=0,group_win=0,group_code=0,gain_qun=0,gain_sys=0,gain_agent[AGENT_MAX];
	
	for (i=0;i<AGENT_MAX;i++)
	{
		gain_agent[i] = 0;
	}
	//免死号结算 to do ...
	if (hall->hb[index].list[site].catch_num > 0)
	{
		gold = hall->hb[index].list[site].user_catch[0].gold;
		//群主获利40%,40%做5级返佣,剩余给平台
		gain_sys = gold;
		gain_qun = gold*40/100;
		group_win = gain_qun;
		if (gain_qun > 0)
		{
			//群主
			gain_sys -= gain_qun;
		}
		
		gain_agent[0] = gold*16/100;
		gain_sys -= gain_agent[0];

		if (hall->user[hall->hb[index].list[site].user].agent_num > 0)
		{
			for (i=0;i<hall->user[hall->hb[index].list[site].user].agent_num;i++)
			{
				switch (i)
				{
					case 0:
						gain_agent[i+1] = gold*14/100;
						break;
					case 1:
						gain_agent[i+1] = gold*5/100;
						break;
					case 2:
						gain_agent[i+1] = gold*3/100;
						break;
					case 3:
						gain_agent[i+1] = gold*2/100;
						break;
					default:
						gain_agent[i+1] = 0;
						break;
				}

				if (gain_agent[i+1] > 0)
				{
					gain_sys -= gain_agent[i+1];
					//5级返佣
					//
					while (update_user_gold_by_uid(gain_agent[i+1],hall->user[hall->hb[index].list[site].user].agent[i]) < 0)
					{
						sleep(1);
					}
					//
				}
			}

		}

		while (add_user_agent_record(hall->hb[index].group_id,hall->user[hall->hb[index].list[site].user].agent,\
				gain_agent,hall->hb[index].list[site].banker_uid,hall->hb[index].list[site].banker_name,(int)hall->time_now) < 0)
		{
			sleep(1);
		}
		//
	}
	//发红包玩家结算
	//多雷奖励判断
	switch (hall->hb[index].list[site].mine_num)
	{
		case 3:
			code = 888;
			break;
		case 4:
			code = 3888;
			break;
		case 5:
			code = 18888;
			break;
		case 6:
			code = 38888;
			break;
		case 7:
			code = 58888;
			break;
		default:
			code = 0;
			break;
	}
	group_code += code;

	hall->hb[index].list[site].banker_code = code;
	hall->hb[index].list[site].banker_gain = (hall->hb[index].list[site].banker_win + hall->hb[index].list[site].banker_back + code + gain_agent[0]);
	__sync_fetch_and_add(&(hall->user[hall->hb[index].list[site].user].money),hall->hb[index].list[site].banker_gain);
	//printf("user = %d money=%lld\n",hall->hb[index].do_list[site].user,hall->user[hall->hb[index].do_list[site].user].money);
	for (i=1;i<hall->hb[index].list[site].catch_num;i++)
	{
		if (hall->hb[index].list[site].user_catch[i].code > 0)
		{
			flag_code = 1;
			group_code += hall->hb[index].list[site].user_catch[i].code;
		}
	}

	if (code > 0)
	{
		flag_code = 1;
	}
	hall->hb[index].list[site].status = 2;
	
	if (flag_code == 1)
	{
		for (i=0;i<hall->hb[index].house_num;i++)
		{
			hall->hb[index].flag_over[i][site] = 1;
		}
	}
	
	updateUserGold(hall->hb[index].list[site].user,0,0,hall->hb[index].list[site].banker_gold,1);
	gain_qun = group_win - group_code;
	hall->hb[index].list[site].user_catch[0].gain = gain_qun;
	hall->hb[index].list[site].user_catch[0].code = gain_sys;
	hall->hb[index].list[site].user_catch[0].lose = gold - gain_sys - gain_qun;
	//群主结算完成->记录 系统结算完成->记录
	while (update_group_gain(hall->hb[index].group_id,group_win,group_code,gain_qun,gain_sys) < 0)
	{
		sleep(1);
	}
	//
	//生成游戏记录
	while (add_game_hb_record(index,site,(int)hall->time_now) < 0)
	{
		sleep(1);
	}
	//
	
	hall->user[hall->hb[index].list[site].user].msg_gold_update = 1;
	__sync_fetch_and_sub(&(hall->user[hall->hb[index].list[site].user].hb_send_num[index]),1);

	if (hall->user[hall->hb[index].list[site].user].status == 4 || (hall->user[hall->hb[index].list[site].user].status == 2 && hall->time_now - hall->user[hall->hb[index].list[site].user].time > 10))
	{
		//断线清理
		HBCleanUser(index,hall->hb[index].list[site].user);
		hall->hb[index].list[site].status = 3;
	}
}

int HBUserMsg(char *buf,int index,int user)
{
	if (hall->user[user].hds < 0)
	{
		//已离桌
		return 0;
	}
	int i,j,length;

	sem_wait(&(hall->hb[index].lock_msg));
	if (hall->hb[index].msg_list_flag > 0)
	{
		sem_post(&(hall->hb[index].lock_msg));
		//return 0;
		length = packReturnFlag(buf,2026,0);
		return UserSend(user,buf,length);
	}

	for (i=0;i<MSG_LIST_MAX;i++)
	{
		if (hall->hb[index].hb_msg[i].length == 0)
		{
			hall->hb[index].hb_msg[i].length = pack3026(hall->hb[index].hb_msg[i].msg,buf+17,user);
			
			hall->hb[index].msg_list_flag = 0;
			for (j=0;j<hall->hb[index].house_num;j++)
			{
				hall->hb[index].flag_msg[j] = 1;
			}
			sem_post(&(hall->hb[index].lock_msg));

			length = packReturnFlag(buf,2026,1);
			return UserSend(user,buf,length);
		}
	}
	sem_post(&(hall->hb[index].lock_msg));
	//return 0;
	length = packReturnFlag(buf,2026,0);
	return UserSend(user,buf,length);
}
