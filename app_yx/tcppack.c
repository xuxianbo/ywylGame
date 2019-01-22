#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "myshm.h"
#include "mybase.h"
#include "mytcp.h"
#include "cards.h"
#include "tcppack.h"
extern HALL *hall;

int pack555(char *buf)
{
	char *p;
	p = buf;
	*p='|';
	p++;
	send_int32_2buf(p,9);
	p+=4;
	send_int32_2buf(p,555);

	return 9;
}

int pack666(char *buf,short int flag)
{
	char *p;
	p = buf;
	*p='|';
	p++;
	send_int32_2buf(p,11);
	p+=4;
	send_int32_2buf(p,666);
	p+=4;
	send_int16_2buf(p,flag);

	return 11;
}

int pack777(char *buf)
{
	char *p;
	p = buf;
	*p='|';
	p++;
	send_int32_2buf(p,9);
	p+=4;
	send_int32_2buf(p,777);

	return 9;
}

int pack888(char *buf,int ret)
{
	int length;
	char mybuf[RECV_BUF_MAX];
	char *p;
	
	if (ret > 60 && ret%60 == 0)
	{
		sprintf(mybuf,"亲爱的的玩家，官方将于%d分钟之后进行5~10分钟的系统维护,请广大玩家及时下线,以免出现金币异常,无法找回!",ret/60);
	}
	else if (ret > 10 && ret%10 == 0 && ret <= 60)
	{
		sprintf(mybuf,"亲爱的的玩家，官方将于%d秒之后进行5~10分钟的系统维护,请广大玩家及时下线,以免出现金币异常,无法找回!",ret);
	}
	else if (ret > 0 && ret <= 10)
	{
		sprintf(mybuf,"亲爱的的玩家，官方将于%d秒之后进行5~10分钟的系统维护,以免出现金币异常,无法找回!",ret);
	}
	else
	{
		return -1;
	}
	
	length = strlen(mybuf)+11;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,888);
	p+=4;
	send_int16_2buf(p,ret);
	p+=2;
	send_string_2buf(p,mybuf);

	return length;
}

int pack998(char *buf)
{
	char *p;
	int i,length=9+hall->normal_num*10;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,998);
	p+=4;
	for (i=0;i<hall->normal_num;i++)
	{
		send_int16_2buf(p,hall->fork[i].gameid);
		p+=2;
		send_int16_2buf(p,hall->fork[i].type);
		p+=2;
		send_int16_2buf(p,hall->fork[i].grade);
		p+=2;
		send_int16_2buf(p,i);
		p+=2;
		send_int16_2buf(p,hall->fork[i].thread);
		p+=2;
	}
	
	return length;
}

int pack999(char *buf)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,17);
	p+=4;
	send_int32_2buf(p,999);
	p+=4;
	send_long64_2buf(p,hall->pool);
	return 17;
}

int pack1000(char *buf,int session)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,1000);
	p+=4;
	send_int32_2buf(p,session);

	return 13;
}

int pack2001(char *buf,short int flag,short int ret,short int gid,short int type,short int grade,int hds,int session)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,27);
	p+=4;
	send_int32_2buf(p,2001);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,ret);
	p+=2;
	send_int16_2buf(p,gid);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,grade);
	p+=2;
	send_int32_2buf(p,hds);
	p+=4;
	send_int32_2buf(p,session);

	return 27;
}

int pack2002(char *buf,short int flag,short int gid,short int type,short int grade)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,17);
	p+=4;
	send_int32_2buf(p,2002);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,gid);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,grade);
	return 17;
}

int pack2003(char *buf,short int flag,int forkid,int desk,short int seat)
{
	int length=21;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,2003);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].gameid);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].type);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].grade);
	p+=2;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	if (hall->fork[forkid].gameid == 1)
	{
		//炸金花
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].round_max);
		//p+=2;
		length += 2;
	}
	else if (hall->fork[forkid].gameid == 2)
	{
		//牛牛
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].model);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].kan);
		//p+=2;
		length += 4;
	}
	else if (hall->fork[forkid].gameid == 6)
	{
		//麻将
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_change);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_zm);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_times);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_dg);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_yjjd);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_mqzz);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].mj_td);
		//p+=2;
		length += 14;
	}
	
	send_int32_2buf(buf+1,length);
	return length;
}

int pack3015(char *buf,int forkid,int desk,int seat)
{
	long long int gold;
	int i,uid,length,length_name,url_len,ip_len;
	short int sex,status;
	char *p;
	char *name;
	char *url;
	char *ip;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3015);
	p+=4;
	length = 11;

	if (seat < 0)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
		p+=2;
		//全部
		//printf("-------------------------pack 3015 begin----------------------------\n");
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			//printf("pack 3015 seat->%d : status->%d\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
			if (hall->fork[forkid].zjh[desk].seat[i].status != 0)
			{
				url = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].url;
				if (hall->fork[forkid].gold_type == 1)
				{
					gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold;
				}
				else
				{
					gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
				}
				uid = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;
				sex = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].sex;
				name = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name;
				ip = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].ip;
				status = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status;
				length_name = strlen(name);
				url_len = strlen(url);
				ip_len = strlen(ip);
			}
			else
			{
				ip_len = 0;
				url_len = 0;
				length_name = 0;
				gold = 0;
				uid = 0;
				sex = 0;
				status = 0;
			}
			
			send_int16_2buf(p,(short)i);
			p+=2;
			send_int32_2buf(p,uid);
			p+=4;
			send_int16_2buf(p,status);
			p+=2;
			send_int16_2buf(p,sex);
			p+=2;
			send_long64_2buf(p,gold);
			p+=8;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].status);
			p+=2;
			send_long64_2buf(p,hall->fork[forkid].zjh[desk].seat[i].in_all);
			p+=8;
			send_int16_2buf(p,(short)length_name);
			p+=2;
			if (length_name > 0)
			{
				send_string_2buf(p,name);
				p+=length_name;
			}
			send_int16_2buf(p,(short)url_len);
			p+=2;
			if (url_len > 0)
			{
				send_string_2buf(p,url);
				p+=url_len;
			}
			send_int16_2buf(p,(short)ip_len);
			p+=2;
			if (ip_len > 0)
			{
				send_string_2buf(p,ip);
				p+=ip_len;
			}
			length += (length_name+url_len+ip_len+34);
			
			//printf("i=%d name = [%s] name_len = %d\n",i,name,length_name);
		}
		//printf("-------------------------pack 3015 end----------------------------\n");
	}
	else
	{
		send_int16_2buf(p,1);
		p+=2;
		//单人
		url = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].url;
		if (hall->fork[forkid].gold_type == 1)
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold;
		}
		else
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
		}
		uid = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].uid;
		sex = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].sex;
		name = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].name;
		ip = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].ip;
		
		send_int16_2buf(p,(short)seat);
		p+=2;
		send_int32_2buf(p,uid);
		p+=4;
		send_int16_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].status);
		p+=2;
		send_int16_2buf(p,sex);
		p+=2;
		send_long64_2buf(p,gold);
		p+=8;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].status);
		p+=2;
		send_long64_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].in_all);
		p+=8;
		length_name = strlen(name);
		url_len = strlen(url);
		send_int16_2buf(p,(short)length_name);
		p+=2;
		send_string_2buf(p,name);
		p+=length_name;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,url);
		p+=url_len;
		ip_len = strlen(ip);
		send_int16_2buf(p,(short)ip_len);
		p+=2;
		send_string_2buf(p,ip);
		p+=ip_len;
		length += (length_name+url_len+ip_len+34);
	}

	send_int32_2buf(buf+1,length);
	//printf("3015 legnth = %d\n",length);
	return length;
}

int pack4015(char *buf,int forkid,int desk)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,57);
	p+=4;
	send_int32_2buf(p,4015);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].flag);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].turn);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].round);
	p+=2;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].order_max);
	p+=8;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].in_total);
	p+=8;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].round_max);
	p+=2;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].bottom*5);
	p+=8;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].bottom);
	p+=8;
	send_int16_2buf(p,-1);
	p+=2;
	send_int16_2buf(p,0);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].start_time-hall->time_now);
	
	return 57;
}
/*
int pack3055(char *buf,int forkid,int desk,int seat)
{
	long long int gold;
	int i,uid,length,length_name,url_len;
	short int sex;
	char *p;
	char *name;
	char *url;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3055);
	p+=4;
	length = 11;
	if (seat < 0)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
		p+=2;
		//全部
		//printf("-------------------------pack 3055 begin----------------------------\n");
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			//printf("pack 3055 seat->%d : status->%d\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
			url = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].url;
			if (hall->fork[forkid].gold_type == 1)
			{
				gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold;
			}
			else
			{
				gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
			}
			uid = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;
			sex = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].sex;
			name = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name;
			send_int16_2buf(p,(short)i);
			p+=2;
			send_int32_2buf(p,uid);
			p+=4;
			send_int16_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status);
			p+=2;
			send_int16_2buf(p,sex);
			p+=2;
			send_long64_2buf(p,gold);
			p+=8;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].status);
			p+=2;
			length_name = strlen(name);
			url_len = strlen(url);
			send_int16_2buf(p,(short)length_name);
			p+=2;
			send_string_2buf(p,name);
			p+=length_name;
			send_int16_2buf(p,(short)url_len);
			p+=2;
			send_string_2buf(p,url);
			p+=url_len;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].ranking);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_num);
			p+=2;
			length += (length_name+url_len+28);
			//printf("i=%d name = [%s] name_len = %d\n",i,name,length_name);
		}
		//printf("-------------------------pack 3055 end----------------------------\n");
	}
	else
	{
		send_int16_2buf(p,1);
		p+=2;
		//单人
		url = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].url;
		
		if (hall->fork[forkid].gold_type == 1)
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold;
		}
		else
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
		}
		uid = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].uid;
		sex = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].sex;
		name = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].name;
		
		send_int16_2buf(p,(short)seat);
		p+=2;
		send_int32_2buf(p,uid);
		p+=4;
		send_int16_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].status);
		p+=2;
		send_int16_2buf(p,sex);
		p+=2;
		send_long64_2buf(p,gold);
		p+=8;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].status);
		p+=2;
		length_name = strlen(name);
		url_len = strlen(url);
		send_int16_2buf(p,(short)length_name);
		p+=2;
		send_string_2buf(p,name);
		p+=length_name;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,url);
		p+=url_len;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].ranking);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_num);
		p+=2;
		length += (length_name+url_len+28);
	}

	send_int32_2buf(buf+1,length);
	return length;
}

int pack4055(char *buf,int forkid,int desk)
{
	int i,length=51+hall->fork[forkid].zjh[desk].card_num*2;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,4055);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].flag);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].banker);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].turn);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].power);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].is_pay);
	p+=2;
	for (i=0;i<4;i++)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].pay[i]);
		p+=2;
	}
	for (i=0;i<2;i++)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].level[i]);
		p+=2;
	}
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].card_type);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].card_num);
	p+=2;
	for (i=0;i<hall->fork[forkid].zjh[desk].card_num;i++)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].cards[i]);
		p+=2;
	}
	//普通场
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].bottom);
	p+=8;
	send_int16_2buf(p,-1);
	p+=2;
	send_int16_2buf(p,0);
	p+=2;
	printf("------------------4055 flag=%d time=%d------------------\n",hall->fork[forkid].zjh[desk].flag,(short int)(hall->fork[forkid].zjh[desk].start_time-hall->time_now));
	send_int16_2buf(p,(short int)(hall->fork[forkid].zjh[desk].start_time-hall->time_now));
	
	return length;
}
*/
int packReturnFlag(char *buf,int command,short int flag)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,11);
	p+=4;
	send_int32_2buf(p,command);
	p+=4;
	send_int16_2buf(p,flag);

	return 11;
}

int packReturnFlagGold(char *buf,int command,short int flag,long long int gold)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,19);
	p+=4;
	send_int32_2buf(p,command);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,gold);
	return 19;
}

int packPublicSeat(char *buf,int command,short int seat)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,11);
	p+=4;
	send_int32_2buf(p,command);
	p+=4;
	send_int16_2buf(p,seat);

	return 11;
}

int pack2006(char *buf,short int gid,short int type,short int grade,int flag)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,19);
	p+=4;
	send_int32_2buf(p,2006);
	p+=4;
	send_int16_2buf(p,gid);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,grade);
	p+=2;
	send_int32_2buf(p,flag);

	return 19;
}

int pack3006(char *buf,int forkid,int desk,int time)
{
	//short int num=0;
	int i,length = 17;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3006);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].banker);
	p+=2;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 2)
		{
			send_int16_2buf(p,(short)i);
			p+=2;
			length+=2;
			//num++;
		}
	}
				
	send_int16_2buf(p,-1);
	p+=2;
	send_int16_2buf(p,0);
	p+=2;
	send_int16_2buf(p,time);
	send_int32_2buf(buf+1,length);
	//send_int16_2buf(buf+11,num);

	return length;
}

int pack4006(char *buf,int forkid,int desk,short int time,short int flag)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,25);
	p+=4;
	send_int32_2buf(p,4006);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].round);
	p+=2;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].order_max);
	p+=8;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].turn);
	p+=2;
	send_int16_2buf(p,time);
	p+=2;
	send_int16_2buf(p,flag);

	return 25;
}

int packLookCards(char *buf,int forkid,int desk,int seat,int num)
{
	int i,length=9;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,2007);
	p+=4;
	
	if (num > 0)
	{
		for (i=0;i<num;i++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[i]);
			p+=2;
		}
		length += 2*num;
	}
	else 
	{
		for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card[i]);
			p+=2;
		}
		length += 2*hall->fork[forkid].zjh[desk].seat[seat].card_num;
	}
	send_int32_2buf(buf+1,length);
	return length;
}

int pack3008(char *buf,int forkid,int desk,int seat,long long int gold)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,35);
	p+=4;
	send_int32_2buf(p,3008);
	p+=4;
	send_int16_2buf(p,(short)seat);
	p+=2;
	if (hall->fork[forkid].gold_type == 1)
	{
		send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold);
	}
	else
	{
		send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money);
	}
	p+=8;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].in_all);
	p+=8;
	send_long64_2buf(p,gold);

	return 35;
}

int pack3009(char *buf,short int seat,short int win,short int lose)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,15);
	p+=4;
	send_int32_2buf(p,3009);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,win);
	p+=2;
	send_int16_2buf(p,lose);

	return 15;
}

int pack3012(char *buf,int forkid,int desk,short int win,short int lose)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,25);
	p+=4;
	send_int32_2buf(p,3012);
	p+=4;
	send_int16_2buf(p,win);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[win].card_copy[0]);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[win].card_copy[1]);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[win].card_copy[2]);
	p+=2;
	send_int16_2buf(p,lose);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[lose].card_copy[0]);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[lose].card_copy[1]);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[lose].card_copy[2]);
	
	return 25;
}

int pack3013(char *buf,int forkid,int desk,short int win,long long int gold_win)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,27);
	p+=4;
	send_int32_2buf(p,3013);
	p+=4;
	send_int16_2buf(p,win);
	p+=2;
	if (hall->fork[forkid].gold_type == 1)
	{
		send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].gold);
	}
	else
	{
		send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money);
	}
	
	p+=8;
	send_long64_2buf(p,gold_win);
	return 27;
}
/*
int pack4013(char *buf,short int seat_max,short int flag)
{
	int i,length;
	char *p;
	length = 11;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,4013);
	p+=4;
	send_int16_2buf(p,seat_max);
	p+=2;
	for(i=0;i<seat_max;i++)
	{
		send_int16_2buf(p,(short)i);
		p+=2;
		send_int16_2buf(p,flag);
		p+=2;
		length+=4;
	}
	send_int32_2buf(buf+1,length);

	return length;
}*/

int pack3014(char *buf,char *msg,short int seat)
{
	int length;
	char *p;
	length = 11;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3014);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,*((short int *)msg));
	p+=2;
	send_string_2buf(p,msg+2);
	length += (strlen(msg+2)+2);
	send_int32_2buf(buf+1,length);

	return length;
}

int pack3016(char *buf,int user)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,37);
	p+=4;
	send_int32_2buf(p,3016);
	p+=4;
	send_int32_2buf(p,hall->user[user].uid);
	p+=4;
	send_long64_2buf(p,hall->user[user].gold);
	p+=8;
	send_long64_2buf(p,hall->user[user].money);
	p+=8;
	send_long64_2buf(p,hall->user[user].stock);
	
	return 37;
}

int pack3017(char *buf,int forkid,int desk,short int win,long long int gold_win)
{
	int i,length = 27;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3017);
	p+=22;
	
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		//printf("fork%d desk%d seat%d status = %d\n",forkid,desk,i,hall->fork[forkid].zjh[desk].seat[i].status);
		if (hall->fork[forkid].zjh[desk].seat[i].status > 2 && hall->fork[forkid].zjh[desk].seat[i].status < 5)
		{
			length += 8;
			send_int16_2buf(p,(short)i);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card[0]);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card[1]);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card[2]);
			p+=2;
		}
	}

	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,win);

	if (hall->fork[forkid].gold_type == 1)
	{
		send_long64_2buf(buf+11,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].gold);
	}
	else
	{
		send_long64_2buf(buf+11,hall->user[hall->fork[forkid].zjh[desk].seat[win].user].money);
	}
	
	send_long64_2buf(buf+19,gold_win);

	return length;
}

int pack2018(char *buf,short int gid,short int type)
{
	int i,j,length=11,hds;
	short int k=0;
	char *p;
	
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,2018);
	p+=6;
	
	for (i=0;i<hall->normal_num;i++)
	{
		if (hall->fork[i].gameid == gid && (type == 0 || hall->fork[i].type == type) && hall->fork[i].grade == PRIVATE_HOUSE)
		{
			for (j=0;j<DESK_MAX;j++)
			{
				if (hall->fork[i].zjh[j].desk_status == 1)
				{
					k++;
					//可见
					hds = i*1000+j*10;
					//printf("hds = %d,HDS=%d,seat_max=%d,people=%d\n",hds,createHDS(hds),hall->fork[i].zjh[j].seat_max,hall->fork[i].zjh[j].people);
					send_int16_2buf(p,hall->fork[i].gameid);
					p+=2;
					send_int16_2buf(p,hall->fork[i].type);
					p+=2;
					send_int32_2buf(p,createHDS(hds));
					p+=4;
					send_long64_2buf(p,hall->fork[i].zjh[j].bottom);
					p+=8;
					send_int16_2buf(p,hall->fork[i].zjh[j].seat_max);
					p+=2;
					send_int16_2buf(p,hall->fork[i].zjh[j].people);
					p+=2;
					if (gid == 1)
					{
						//炸金花
						send_int16_2buf(p,hall->fork[i].zjh[j].round_max);
						p+=2;
						length += 22;
					}
					else if (gid == 2)
					{
						//牛牛
						send_int16_2buf(p,hall->fork[i].zjh[j].model);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].kan);
						p+=2;
						length += 24;
					}
					else if (gid == 6)
					{
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_change);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_zm);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_times);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_dg);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_yjjd);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_mqzz);
						p+=2;
						send_int16_2buf(p,hall->fork[i].zjh[j].mj_td);
						p+=2;
						length += 34;
					}
				}
			}
		}
	}

	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,k);
	//printf("2018 gid=%d,type=%d,k=%d\n",gid,type,k);
	return length;
}

int pack2019(char *buf,short int flag,short int gid,short int type,short int grade,int hds)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,21);
	p+=4;
	send_int32_2buf(p,2019);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,gid);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,grade);
	p+=2;
	send_int32_2buf(p,hds);
	
	return 21;
}

int pack3020(int forkid,int index,char *buf,int bet_max,int look_max,long long int gold_banker)
{
	int i,j,length = 15,name_len,url_len,ip_len,seat_num,tmp_user;
	short int sex=0;
	char name[16]="";
	char url[2]="";
	char ip[16]="";
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3020);
	p+=6;
	send_int16_2buf(p,0);
	p+=2;
	//庄家信息
	if (hall->bjl[index].banker < 0)
	{
		//系统庄
		sprintf(name,"6LSi56We54i3");
		sprintf(url,"0");
		sprintf(ip,"127.0.0.1");

		name_len = strlen(name);
		url_len = strlen(url);
		ip_len = strlen(ip);
		send_int32_2buf(p,hall->bjl[index].banker);
		p+=4;
		send_int16_2buf(p,sex);
		p+=2;
		send_int16_2buf(p,hall->bjl[index].banker_times);
		p+=2;
		send_long64_2buf(p,gold_banker);
		p+=8;

		for (i=0;i<bet_max;i++)
		{
			send_long64_2buf(p,0);
			p+=8;
		}
		
		send_int16_2buf(p,10);
		p+=2;
		send_int16_2buf(p,(short)name_len);
		p+=2;
		send_string_2buf(p,name);
		p+=name_len;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,url);
		p+=url_len;
		send_int16_2buf(p,(short)ip_len);
		p+=2;
		send_string_2buf(p,ip);
		p+=ip_len;

	}
	else
	{
		//玩家庄
		//printf("3020 ... banker gold = %lld\n",hall->user[hall->bjl[index].banker].money);
		name_len = strlen(hall->user[hall->bjl[index].banker].name);
		url_len = strlen(hall->user[hall->bjl[index].banker].url);
		ip_len = strlen(hall->user[hall->bjl[index].banker].ip);
		send_int32_2buf(p,hall->user[hall->bjl[index].banker].uid);
		p+=4;
		send_int16_2buf(p,hall->user[hall->bjl[index].banker].sex);
		p+=2;
		send_int16_2buf(p,hall->bjl[index].banker_times);
		p+=2;
		send_long64_2buf(p,hall->user[hall->bjl[index].banker].money);
		p+=8;

		for (i=0;i<bet_max;i++)
		{
			send_long64_2buf(p,0);
			p+=8;
		}

		send_int16_2buf(p,hall->user[hall->bjl[index].banker].banker_flag[index]);
		p+=2;
		send_int16_2buf(p,(short)name_len);
		p+=2;
		send_string_2buf(p,hall->user[hall->bjl[index].banker].name);
		p+=name_len;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,hall->user[hall->bjl[index].banker].url);
		p+=url_len;
		send_int16_2buf(p,(short)ip_len);
		p+=2;
		send_string_2buf(p,hall->user[hall->bjl[index].banker].ip);
		p+=ip_len;
	}
	length += (url_len+name_len+ip_len+26+bet_max*8);
	//入座闲家信息
	seat_num = 1;
	for (i=0;i<look_max;i++)
	{
		tmp_user = hall->bjl[index].seat[i].user;
		if (tmp_user < 0 || tmp_user >= USER_MAX || hall->user[tmp_user].uid <= 0)
		{
			continue;
		}
		seat_num++;
		name_len = strlen(hall->user[tmp_user].name);
		url_len	= strlen(hall->user[tmp_user].url);
		ip_len = strlen(hall->user[tmp_user].ip);

		send_int16_2buf(p,i+1);
		p+=2;
		send_int32_2buf(p,hall->user[tmp_user].uid);
		p+=4;
		send_int16_2buf(p,hall->user[tmp_user].sex);
		p+=2;
		send_long64_2buf(p,hall->user[tmp_user].money);
		p+=8;
		for (j=0;j<bet_max;j++)
		{
			send_long64_2buf(p,hall->user[tmp_user].bet[index][j]);
			p+=8;
		}
		send_int16_2buf(p,1);
		p+=2;
		send_int16_2buf(p,(short)name_len);
		p+=2;
		send_string_2buf(p,hall->user[tmp_user].name);
		p+=name_len;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,hall->user[tmp_user].url);
		p+=url_len;
		send_int16_2buf(p,(short)ip_len);
		p+=2;
		send_string_2buf(p,hall->user[tmp_user].ip);
		p+=ip_len;
		length += (url_len+name_len+ip_len+24+bet_max*8);
	}
	send_int16_2buf(p,hall->fork[forkid].gameid);
	p+=2;
	send_int16_2buf(p,hall->bjl[index].list_num);
	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,(short)seat_num);

	return length;
}

int pack4020(int forkid,int index,char *buf,int bet_max)
{
	int i,length = 29 + bet_max*8;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,4020);
	p+=4;
	send_int16_2buf(p,hall->bjl[index].flag);
	p+=2;
	send_int16_2buf(p,hall->bjl[index].start_time - hall->time_now);
	p+=2;
	send_long64_2buf(p,0);
	p+=8;
	send_int16_2buf(p,hall->bjl[index].banker_card[0]);
	p+=2;
	send_int16_2buf(p,hall->bjl[index].banker_card[1]);
	p+=2;
	send_int16_2buf(p,hall->bjl[index].banker_card[2]);
	p+=2;
	for (i=0;i<bet_max;i++)
	{
		send_int16_2buf(p,hall->bjl[index].card[i][0]);
		p+=2;
		send_int16_2buf(p,hall->bjl[index].card[i][1]);
		p+=2;
		send_int16_2buf(p,hall->bjl[index].card[i][2]);
		p+=2;
	}

	for (i=0;i<bet_max;i++)
	{
		send_int16_2buf(p,hall->bjl[index].win[i]);
		p+=2;
	}
	send_int16_2buf(p,hall->fork[forkid].gameid);
	return length;
}

int pack5020(int forkid,int index,char *buf,int bet_max)
{
	int i,j,length = 13;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,5020);
	p+=6;
	for (i=0;i<BJL_L_MAX;i++)
	{
		if (hall->bjl[index].list[i][0] >= 0)
		{
			for (j=0;j<bet_max;j++)
			{
				send_int16_2buf(p,hall->bjl[index].list[i][j]);
				p+=2;
			}
			length += 2*bet_max;
		}
		else
		{
			break;
		}
	}
	send_int16_2buf(p,hall->fork[forkid].gameid);
	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,(short)i);

	return length;
}

int pack6020(int forkid,int index,char *buf,int bet_max)
{
	int i,length = 11+bet_max*8;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,6020);
	p+=4;
	for (i=0;i<bet_max;i++)
	{
		send_long64_2buf(p,hall->bjl[index].gold[i]);
		p+=8;
	}
	send_int16_2buf(p,hall->fork[forkid].gameid);
	return length;
}

int pack7020(int forkid,int index,char *buf,int user,int bet_max)
{
	int i,length = 11+bet_max*8;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,7020);
	p+=4;
	for (i=0;i<bet_max;i++)
	{
		//printf("!!!!!!!!!!7020 bet = %lld\n",hall->user[user].bet[i]);
		send_long64_2buf(p,hall->user[user].bet[index][i]);
		p+=8;
	}
	send_int16_2buf(p,hall->fork[forkid].gameid);
	return length;
}

int pack2021(int index,char *buf,int user,short int type,short int flag,int bet_max)
{
	int i,length = 29+bet_max*16;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,2021);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_long64_2buf(p,hall->user[user].money);
	p+=8;
	for (i=0;i<bet_max;i++)
	{
		send_long64_2buf(p,hall->user[user].bet[index][i]);
		p+=8;
	}

	for (i=0;i<bet_max;i++)
	{
		send_long64_2buf(p,hall->bjl[index].gold[i]);
		p+=8;
	}

	if (hall->bjl[index].banker < 0)
	{
		//系统庄
		send_long64_2buf(p,GOLD_BANKER);
	}
	else
	{
		//玩家
		send_long64_2buf(p,hall->user[hall->bjl[index].banker].money);
	}
	return length;
}

int pack4021(int forkid,int index,char *buf,int user,int bet_max,int look_max,long long int gold_banker)
{
	int i,j,name_len,url_len,tmp_user,seat_num,length = 25+bet_max*8;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,4021);
	p+=6;
	send_int16_2buf(p,0);
	p+=2;
	for (i=0;i<bet_max;i++)
	{
		send_long64_2buf(p,hall->bjl[index].gold_win[i]);
		p+=8;
	}
	//庄家信息
	if (hall->bjl[index].banker < 0)
	{
		//系统庄
		send_long64_2buf(p,gold_banker);
		p+=8;
	}
	else
	{
		//玩家庄
		//printf("4021 banker gold = %lld\n",hall->user[hall->bjl[index].banker].money);
		send_long64_2buf(p,hall->user[hall->bjl[index].banker].money);
		p+=8;
	}
	//闲家信息
	seat_num = 1;
	for (i=0;i<look_max;i++)
	{
		tmp_user = hall->bjl[index].seat[i].user;
		if (tmp_user < 0)
		{
			continue;
		}
		seat_num++;
		send_int16_2buf(p,(short)(i+1));
		p+=2;
		for (j=0;j<bet_max;j++)
		{
			send_long64_2buf(p,hall->user[tmp_user].gold_win[index][j]);
			p+=8;
		}
		send_long64_2buf(p,hall->user[tmp_user].money);
		p+=8;
		
		length += (10+bet_max*8);
		//
	}
	
	if (user >= 0)
	{
		seat_num++;
		send_int16_2buf(p,1000);
		p+=2;
		for (j=0;j<bet_max;j++)
		{
			send_long64_2buf(p,hall->user[user].gold_win[index][j]);
			p+=8;
		}
		send_long64_2buf(p,hall->user[user].money);
		p+=8;
		length += (10+bet_max*8);
	}
	
	//最大赢家
	name_len = strlen(hall->bjl[index].win_name[0]);
	
	url_len = strlen(hall->bjl[index].win_url[0]);
	send_int16_2buf(p,(short)name_len);
	p+=2;
	
	if (name_len > 0)
	{
		//
		send_string_2buf(p,hall->bjl[index].win_name[0]);
		p+=name_len;
		send_long64_2buf(p,hall->bjl[index].win_max[0]);
		p+=8;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,hall->bjl[index].win_url[0]);
		p+=url_len;
		length+=(name_len+url_len+10);
	}
	
	//
	send_int16_2buf(p,hall->fork[forkid].gameid);
	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,(short)seat_num);

	return length;
}

int pack3025(int index,char *buf)
{
	int name_len,i,length = 11;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3025);
	p+=6;
	for (i=0;i<BANKER_MAX;i++)
	{
		if (hall->bjl[index].banker_list[i] >= 0)
		{
			name_len = strlen(hall->user[hall->bjl[index].banker_list[i]].name);
			//url_len = strlen(hall->user[hall->bjl[index].banker_list[i]].url);
			send_int32_2buf(p,hall->user[hall->bjl[index].banker_list[i]].uid);
			p+=4;
			send_long64_2buf(p,hall->user[hall->bjl[index].banker_list[i]].money);
			p+=8;
			send_int16_2buf(p,(short)name_len);
			p+=2;
			send_string_2buf(p,hall->user[hall->bjl[index].banker_list[i]].name);
			p+=name_len;
			//send_int16_2buf(p,(short)url_len);
			//p+=2;
			//send_string_2buf(p,hall->user[hall->bjl[index].banker_list[i]].url);
			//p+=url_len;
			//length += (name_len+url_len+16);
			length += (name_len+14);
		}
		else
		{
			break;
		}
	}

	send_int32_2buf(buf+1,length);
	send_int16_2buf(buf+9,(short)i);

	return length;
}

int pack3026(char *buf,char *msg,int user)
{
	int name_len = strlen(hall->user[user].name);
	int length = strlen(msg+2)+name_len+19;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3026);
	p+=4;
	send_int32_2buf(p,hall->user[user].uid);
	p+=4;
	send_int16_2buf(p,hall->user[user].sex);
	p+=2;
	send_int16_2buf(p,(short)name_len);
	p+=2;
	send_string_2buf(p,hall->user[user].name);
	p+=name_len;
	send_int16_2buf(p,*((short int *)msg));
	p+=2;
	send_string_2buf(p,msg+2);

	return length;
}
//

int pack3027(char *buf,int uid,char *name,short int gid,short int check_type,long long int gold_win)
{
	int name_len = strlen(name);
	int length = name_len+27;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3027);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_int16_2buf(p,(short)name_len);
	p+=2;
	send_string_2buf(p,name);
	p+=name_len;
	send_int16_2buf(p,gid);
	p+=2;
	send_int16_2buf(p,check_type);
	p+=2;
	send_long64_2buf(p,gold_win);

	return length;
}

int pack3028(char *buf,char *msg,int user)
{
	int name_len = strlen(hall->user[user].name);
	int length = strlen(msg)+name_len+17;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3028);
	p+=4;
	send_int32_2buf(p,hall->user[user].uid);
	p+=4;
	send_int16_2buf(p,hall->user[user].sex);
	p+=2;
	send_int16_2buf(p,(short)name_len);
	p+=2;
	send_string_2buf(p,hall->user[user].name);
	p+=name_len;
	send_string_2buf(p,msg);

	return length;
}

int pack3029(int index,char *buf,int bet_max)
{
	int i,length=bet_max*2+11;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3029);
	p+=4;
	send_int16_2buf(p,hall->bjl[index].banker_card[0]);
	p+=2;
	for (i=0;i<bet_max;i++)
	{
		send_int16_2buf(p,hall->bjl[index].card[i][0]);
		p+=2;
	}
	return length;
}

int pack3030(char *buf)
{
	buf[0] = '|';
	send_int32_2buf(buf+1,9);
	send_int32_2buf(buf+5,3030);
	return 9;
}

//'|'+length(int)	+command(int)+seat_num(short int,座位数量)+seat(short int,座号)+ox_type(short int,牛牛牌型)+card1(short int,牌1)+...+card5(short int,牌5)+...+(重复)
int pack3040(char *buf,int forkid,int desk,int seat)
{
	int i,length=11;
	char *p;
	
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3040);
	p+=4;
	
	if (seat < 0)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].player);
		p+=2;

		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].status > 4)
			{
				send_int16_2buf(p,(short)i);
				p+=2;
				send_int16_2buf(p,check_type_ox(hall->fork[forkid].zjh[desk].seat[i].card,hall->fork[forkid].zjh[desk].model,hall->fork[forkid].zjh[desk].kan));
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_copy[0]);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_copy[1]);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_copy[2]);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_copy[3]);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_copy[4]);
				p+=2;

				length+=14;
			}
		}
	}
	else
	{
		send_int16_2buf(p,1);
		p+=2;
		send_int16_2buf(p,(short)seat);
		p+=2;
		send_int16_2buf(p,check_type_ox(hall->fork[forkid].zjh[desk].seat[seat].card,hall->fork[forkid].zjh[desk].model,hall->fork[forkid].zjh[desk].kan));
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[0]);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[1]);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[2]);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[3]);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_copy[4]);
		p+=2;

		length+=14;
	}


	send_int32_2buf(buf+1,length);
	return length;
}

int pack3041(char *buf,long long int *balance,int forkid,int desk)
{
	int i,length=11;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3041);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].player);
	p+=2;

	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 4)
		{
			send_int16_2buf(p,(short)i);
			p+=2;
			send_int32_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid);
			p+=4;
			if (hall->fork[forkid].gold_type == 1)
			{
				send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold);
			}
			else
			{
				send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money);
			}
			p+=8;
			send_long64_2buf(p,balance[i]);
			p+=8;
			length+=22;
		}
	}

	send_int32_2buf(buf+1,length);
	return length;
}

void createIP(char *ip,int uid)
{
	sprintf(ip,"223.104.250.%d",uid%256);
}
/*
int pack3050(char *buf,short int seat,short int card)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,3050);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,card);

	return 13;
}

int pack3051(char *buf,short int seat,short int *card,short int num,short int type)
{
	int i,length=15+num*2;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3051);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,num);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;

	for (i=0;i<num;i++)
	{
		send_int16_2buf(p,card[i]);
		p+=2;
	}

	return length;
}


int pack4051(char *buf,short int seat,short int ranking)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,4051);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,ranking);

	return 13;
}

int pack5051(char *buf,int forkid,int desk)
{
	int i,length=11+hall->fork[forkid].zjh[desk].seat_max*12;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,5051);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		send_int16_2buf(p,(short)i);
		p+=2;
		send_long64_2buf(p,hall->fork[forkid].zjh[desk].gold[i]);
		p+=8;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].ranking);
		p+=2;
	}

	return length;
}*/

int pack2060(char *buf,short int type,short int flag,long long int gold)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,21);
	p+=4;
	send_int32_2buf(p,2060);
	p+=4;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,gold);
	return 21;
}

int pack3061(char *buf,int uid,long long int gold)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,21);
	p+=4;
	send_int32_2buf(p,3061);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_long64_2buf(p,gold);
	return 21;
}

//MJ
int pack3070(char *buf,int forkid,int desk,int seat)
{
	long long int gold;
	int i,j,uid,length,length_name,url_len,ip_len;
	short int sex,status;
	char *p;
	char *name;
	char *url;
	char *ip;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3070);
	p+=4;
	length = 11;

	if (seat < 0)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
		p+=2;
		//全部
		//printf("-------------------------pack 3070 begin----------------------------\n");
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			//printf("pack 3070 seat->%d : status->%d\n",i,hall->fork[forkid].zjh[desk].seat[i].status);
			if (hall->fork[forkid].zjh[desk].seat[i].status != 0)
			{
				url = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].url;
				if (hall->fork[forkid].gold_type == 1)
				{
					gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].gold;
				}
				else
				{
					gold = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money;
				}
				uid = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].uid;
				sex = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].sex;
				name = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].name;
				ip = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].ip;
				status = hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status;
				length_name = strlen(name);
				url_len = strlen(url);
				ip_len = strlen(ip);
			}
			else
			{
				ip_len = 0;
				status = 0;
				gold = 0;
				uid = 0;
				sex = 0;
				length_name = 0;
				url_len = 0;
			}
			//printf("i=%d,uid=%d,status=%d,sex=%d,gold=%lld\n",i,uid,status,sex,gold);
			send_int16_2buf(p,(short)i);
			p+=2;
			send_int32_2buf(p,uid);
			p+=4;
			send_int16_2buf(p,status);
			p+=2;
			send_int16_2buf(p,sex);
			p+=2;
			send_long64_2buf(p,gold);
			p+=8;
			if (hall->fork[forkid].zjh[desk].seat[i].status == 3)
			{
				//游戏中细分状态 
				if (hall->fork[forkid].zjh[desk].flag == 1)
				{
					send_int16_2buf(p,3);
					p+=2;
				}
				else if (hall->fork[forkid].zjh[desk].flag == 2)
				{
					send_int16_2buf(p,4);
					p+=2;
				}
				else if (hall->fork[forkid].zjh[desk].flag == 3)
				{
					send_int16_2buf(p,5);
					p+=2;
				}
				else if (hall->fork[forkid].zjh[desk].flag == 4)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].ranking > 0)
					{
						send_int16_2buf(p,7);
						p+=2;
					}
					else
					{
						send_int16_2buf(p,6);
						p+=2;
					}
					
				}
				else
				{
					send_int16_2buf(p,7);
					p+=2;
				}

			}
			else
			{
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].status);
				p+=2;
			}
			
			
			send_int16_2buf(p,(short)length_name);
			p+=2;
			if (length_name > 0)
			{
				send_string_2buf(p,name);
				p+=length_name;
			}
			
			send_int16_2buf(p,(short)url_len);
			p+=2;
			if (url_len > 0)
			{
				send_string_2buf(p,url);
				p+=url_len;
			}

			send_int16_2buf(p,(short)ip_len);
			p+=2;
			if (ip_len > 0)
			{
				send_string_2buf(p,ip);
				p+=ip_len;
			}

			//
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].turn);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_num);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].lack);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].flag);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].ranking);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].card_win);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_out);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_out_num);
			p+=2;
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_out_num;j++)
			{
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_out_list[j]);
				p+=2;
			}
			send_int16_2buf(p,(hall->fork[forkid].zjh[desk].seat[i].num_peng+hall->fork[forkid].zjh[desk].seat[i].num_gang));
			p+=2;
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].num_peng;j++)
			{
				send_int16_2buf(p,5);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_peng[j]);
				p+=2;
			}
			//3明杠,4胡,5暗杠,6点杠,7自摸,8过手杠
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].num_gang;j++)
			{
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].gang[j].flag);
				p+=2;
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].gang[j].card_gang);
				p+=2;
			}
			//
			length += (length_name+url_len+ip_len+44+hall->fork[forkid].zjh[desk].seat[i].card_out_num*2+(hall->fork[forkid].zjh[desk].seat[i].num_peng+hall->fork[forkid].zjh[desk].seat[i].num_gang)*4);
			//printf("i=%d name_len=%d url_len=%d 3070 length = %d\n",i,length_name,url_len,length);
			
			//printf("i=%d name = [%s] name_len = %d\n",i,name,length_name);
		}
		//printf("-------------------------pack 3015 end----------------------------\n");
	}
	else
	{
		send_int16_2buf(p,1);
		p+=2;
		//单人
		url = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].url;
		if (hall->fork[forkid].gold_type == 1)
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].gold;
		}
		else
		{
			gold = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].money;
		}
		uid = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].uid;
		sex = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].sex;
		name = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].name;
		ip = hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].ip;
		//printf("3070 user=%d,seat=%d,uid=%d,status=%d,seat.status=%d,sex=%d,gold=%lld\n",hall->fork[forkid].zjh[desk].seat[seat].user,seat,uid,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].status,hall->fork[forkid].zjh[desk].seat[seat].status,sex,gold);
		send_int16_2buf(p,(short)seat);
		p+=2;
		send_int32_2buf(p,uid);
		p+=4;
		send_int16_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[seat].user].status);
		p+=2;
		send_int16_2buf(p,sex);
		p+=2;
		send_long64_2buf(p,gold);
		p+=8;
		if (hall->fork[forkid].zjh[desk].seat[seat].status == 3)
		{
			//游戏中细分状态 
			if (hall->fork[forkid].zjh[desk].flag == 1)
			{
				send_int16_2buf(p,3);
				p+=2;
			}
			else if (hall->fork[forkid].zjh[desk].flag == 2)
			{
				send_int16_2buf(p,4);
				p+=2;
			}
			else if (hall->fork[forkid].zjh[desk].flag == 3)
			{
				send_int16_2buf(p,5);
				p+=2;
			}
			else if (hall->fork[forkid].zjh[desk].flag == 4)
			{
				if (hall->fork[forkid].zjh[desk].seat[seat].ranking > 0)
				{
					send_int16_2buf(p,7);
					p+=2;
				}
				else
				{
					send_int16_2buf(p,6);
					p+=2;
				}
				
			}
			else
			{
				send_int16_2buf(p,7);
				p+=2;
			}

		}
		else
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].status);
			p+=2;
		}
		length_name = strlen(name);
		url_len = strlen(url);
		send_int16_2buf(p,(short)length_name);
		p+=2;
		send_string_2buf(p,name);
		p+=length_name;
		send_int16_2buf(p,(short)url_len);
		p+=2;
		send_string_2buf(p,url);
		p+=url_len;
		ip_len = strlen(ip);
		send_int16_2buf(p,(short)ip_len);
		p+=2;
		send_string_2buf(p,ip);
		p+=ip_len;
		
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].turn);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_num);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].lack);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].win[0].flag);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].ranking);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].win[0].card_win);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_out);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_out_num);
		p+=2;
		for (j=0;j<hall->fork[forkid].zjh[desk].seat[seat].card_out_num;j++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_out_list[j]);
			p+=2;
		}
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].num_peng+hall->fork[forkid].zjh[desk].seat[seat].num_gang);
		p+=2;
		for (j=0;j<hall->fork[forkid].zjh[desk].seat[seat].num_peng;j++)
		{
			send_int16_2buf(p,5);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_peng[j]);
			p+=2;
		}
		for (j=0;j<hall->fork[forkid].zjh[desk].seat[seat].num_gang;j++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].gang[j].flag);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].gang[j].card_gang);
			p+=2;
		}
		length += (length_name+url_len+ip_len+44+hall->fork[forkid].zjh[desk].seat[seat].card_out_num*2+(hall->fork[forkid].zjh[desk].seat[seat].num_peng+hall->fork[forkid].zjh[desk].seat[seat].num_gang)*4);
	}

	send_int32_2buf(buf+1,length);
	//printf("3070 all legnth = %d\n",length);
	return length;
}

int pack4070(char *buf,int forkid,int desk)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,31);
	p+=4;
	send_int32_2buf(p,4070);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].flag);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].banker);
	p+=2;
	send_long64_2buf(p,hall->fork[forkid].zjh[desk].bottom);
	p+=8;
	send_int16_2buf(p,-1);
	p+=2;
	send_int16_2buf(p,0);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].start_time - hall->time_now);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].card_num);
	
	return 31;
}


int pack2071(char *buf,int forkid,int desk,int seat)
{
	int i,length=13+hall->fork[forkid].zjh[desk].seat[seat].card_num*2;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,2071);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].banker);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].card_num);
	p+=2;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat[seat].card_num;i++)
	{
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card[i]);
		p+=2;
	}
	return length;
}

int pack2072(char *buf,short int status,short int mytime)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,2072);
	p+=4;
	send_int16_2buf(p,status);
	p+=2;
	send_int16_2buf(p,mytime);

	return 13;
}

int pack2073(char *buf,short int flag,short int *cards,short int num)
{
	int i,length=11 + num*2;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,2073);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	for (i=0;i<num;i++)
	{
		send_int16_2buf(p,cards[i]);
		p+=2;
	}
	//printf("2073 flag = %d\n",flag);
	return length;
}

int pack3073(char *buf,short int seat)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,11);
	p+=4;
	send_int32_2buf(p,3073);
	p+=4;
	send_int16_2buf(p,seat);
	//printf("3073 seat = %d\n",seat);
	return 11;
}

int pack4073(char *buf,short int direction,short int *cards,short int num)
{
	int i,length = 11 + num*2;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,4073);
	p+=4;
	send_int16_2buf(p,direction);
	p+=2;
	for (i=0;i<num;i++)
	{
		send_int16_2buf(p,cards[i]);
		p+=2;
	}
	return length;
}

int pack2074(char *buf,short int flag,short int type)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,2074);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,type);

	return 13;
}

int pack3074(char *buf,short int flag,short int seat)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,3074);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,seat);

	return 13;
}

int pack4074(char *buf,int forkid,int desk)
{
	short int i;
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,25);
	p+=4;
	send_int32_2buf(p,4074);
	p+=4;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		send_int16_2buf(p,i);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].lack);
		p+=2;
	}
	
	return 25;
}

int pack2075(char *buf,int forkid,int desk,int seat,short int flag,short int card)
{
	int length=15;
	char *p;
	p=buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,2075);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].begin_time-hall->time_now);
	p+=2;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,card);
	p+=2;

	if (hall->fork[forkid].zjh[desk].seat[seat].flag_chi == 1)
	{
		send_int16_2buf(p,1);
		p+=2;
		length += 2;
	}
	if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1)
	{
		send_int16_2buf(p,2);
		p+=2;
		length += 2;
	}
	if (hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1)
	{
		send_int16_2buf(p,3);
		p+=2;
		length += 2;
	}
	if (hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
	{
		if (hall->fork[forkid].zjh[desk].seat[seat].card_in > 0 || hall->fork[forkid].zjh[desk].seat[seat].card_num == 14)
		{
			send_int16_2buf(p,5);
			p+=2;
			length += 2;
		}
		else
		{
			send_int16_2buf(p,4);
			p+=2;
			length += 2;
		}
	}
	
	send_int32_2buf(buf+1,length);
	return length;
}

int pack3076(char *buf,short int seat,short int type,short int card_seat,short int card,short int lastNum,short int handNum,short int win)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,23);
	p+=4;
	send_int32_2buf(p,3076);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,card_seat);
	p+=2;
	send_int16_2buf(p,card);
	p+=2;
	send_int16_2buf(p,lastNum);
	p+=2;
	send_int16_2buf(p,handNum);
	p+=2;
	send_int16_2buf(p,win);
	return 23;
}

int pack2077(char *buf,short int flag,short int card)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,2077);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,card);
	
	return 13;
}

int pack3077(char *buf,int forkid,int desk,int seat)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,15);
	p+=4;
	send_int32_2buf(p,3077);
	p+=4;
	send_int16_2buf(p,(short int)seat);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_out);
	p+=2;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[seat].card_num);
	
	return 15;
}

int pack4077(char *buf,short int type,short int seat,short int card,short int num)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,17);
	p+=4;
	send_int32_2buf(p,4077);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,type);
	p+=2;
	send_int16_2buf(p,card);
	p+=2;
	send_int16_2buf(p,num);
	
	return 17;
}

int pack2078(char *buf,short int seat,short int num,short int card)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,15);
	p+=4;
	send_int32_2buf(p,2078);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,num);
	p+=2;
	send_int16_2buf(p,card);
	
	return 15;
}	

int pack3079(char *buf,short int seat,short int time)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,3079);
	p+=4;
	send_int16_2buf(p,seat);
	p+=2;
	send_int16_2buf(p,time);
	
	return 13;
}

int pack3080(char *buf,int forkid,int desk)
{
	short int i,j,k,m,length=11;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3080);
	p+=4;
	send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat_max);
	p+=2;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		length += (58 + (hall->fork[forkid].zjh[desk].seat[i].num_peng+hall->fork[forkid].zjh[desk].seat[i].num_gang)*4 + hall->fork[forkid].zjh[desk].seat[i].card_num*2 + hall->fork[forkid].zjh[desk].seat[i].lose_num*26);
		//座位号
		send_int16_2buf(p,i);
		p+=2;
		//总输赢
		send_long64_2buf(p,hall->fork[forkid].zjh[desk].gold[i]);
		p+=8;
		//最新金币数
		send_long64_2buf(p,hall->user[hall->fork[forkid].zjh[desk].seat[i].user].money);
		p+=8;
		//总番数
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].times);
		p+=2;
		//状态结果
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].flag);
		p+=2;
		//胡序
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].ranking);
		p+=2;
		//胡牌方式
		if (hall->fork[forkid].zjh[desk].seat[i].win[0].flag == 1 || hall->fork[forkid].zjh[desk].seat[i].win[0].flag == 2)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].flag);
			p+=2;
		}
		else
		{
			send_int16_2buf(p,0);
			p+=2;
		}
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].td);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].jg);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].hd);
		p+=2;
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].gh);
		p+=2;

		for (j=0;j<7;j++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].card_type[j]);
			p+=2;
		}
		
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].geng);
		p+=2;
		//操作数量
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].num_peng+hall->fork[forkid].zjh[desk].seat[i].num_gang);
		p+=2;

		for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].num_peng;j++)
		{
			send_int16_2buf(p,5);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_peng[j]);
			p+=2;
		}
		for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].num_gang;j++)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].gang[j].flag);
			p+=2;
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].gang[j].card_gang);
			p+=2;
		}
		//手牌数量
		if (hall->fork[forkid].zjh[desk].seat[i].card_num != 14 && hall->fork[forkid].zjh[desk].seat[i].ranking > 0 && hall->fork[forkid].zjh[desk].seat[i].ranking < 4)
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_num+1);
			p+=2;
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
			{
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card[j]);
				p+=2;
			}
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].win[0].card_win);
			p+=2;
			length += 2;
		}	
		else
		{
			send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card_num);
			p+=2;
			for (j=0;j<hall->fork[forkid].zjh[desk].seat[i].card_num;j++)
			{
				send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].card[j]);
				p+=2;
			}
		}
		//点杠数量
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].point_gang);
		p+=2;
		//被胡数量
		send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[i].lose_num);
		p+=2;
		//printf("i=%d lose_num=%d\n",i,hall->fork[forkid].zjh[desk].seat[i].lose_num);
		for (j=0;j<hall->fork[forkid].zjh[desk].seat_max;j++)
		{
			if (j != i && hall->fork[forkid].zjh[desk].seat[j].ranking > 0)
			{
				//非自己,有胡牌
				for (k=0;k<3;k++)
				{
					if (hall->fork[forkid].zjh[desk].seat[j].win[0].seat[k] == i)
					{
						//胡牌方式
						//printf("i=%d,j=%d,seat[%d]=%d,gold=%lld\n",i,j,k,hall->fork[forkid].zjh[desk].seat[j].win[0].seat[k],hall->fork[forkid].zjh[desk].seat[j].win[0].gold[k]);
						if (hall->fork[forkid].zjh[desk].seat[j].win[0].flag == 1 || hall->fork[forkid].zjh[desk].seat[j].win[0].flag == 2)
						{
							send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].flag);
							p+=2;
						}
						else
						{
							send_int16_2buf(p,0);
							p+=2;
						}
						send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].td);
						p+=2;
						send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].jg);
						p+=2;
						send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].hd);
						p+=2;
						send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].gh);
						p+=2;
						for (m=0;m<7;m++)
						{
							send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].card_type[m]);
							p+=2;
						}
						send_int16_2buf(p,hall->fork[forkid].zjh[desk].seat[j].win[0].geng);
						p+=2;
						break;
					}
				}
			}
		}
		//printf("-------------------------\n");
	}
	//printf("3080 length = %d\n",length);
	send_int32_2buf(buf+1,length);
	return length;
}

int pack3081(char *buf,short int flag,short int num,long long int *gold,long long int *gold_new)
{
	int i,length=13;
	char *p;
	p = buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3081);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,num);
	p+=2;
	for (i=0;i<num;i++)
	{
		send_int16_2buf(p,(short)i);
		p+=2;
		send_long64_2buf(p,gold[i]);
		p+=8;
		send_long64_2buf(p,gold_new[i]);
		p+=8;
		length+=18;
	}

	send_int32_2buf(buf+1,length);
	return length;
}

int pack3082(char *buf,short int seat)
{
	char *p;
	p = buf;
	*p = '|';
	p++;
	send_int32_2buf(p,11);
	p+=4;
	send_int32_2buf(p,3082);
	p+=4;
	send_int16_2buf(p,seat);
	
	return 11;
}

int pack2083(char *buf,short int flag,long long int gold)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,19);
	p+=4;
	send_int32_2buf(p,2083);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,gold);
	return 19;
}


int pack2090(char *buf,short int flag,short int msg_id)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,13);
	p+=4;
	send_int32_2buf(p,2090);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int16_2buf(p,msg_id);
	return 13;
}


int pack3090(char *buf,int uid,short int msg_type,short int msg_len,char *msg)
{
	//printf("pack 3090 uid=%d,msg_type=%d,msg_leng=%d,msg=[%s]\n",uid,msg_type,msg_len,msg);
	int length = msg_len+17;
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3090);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_int16_2buf(p,msg_type);
	p+=2;
	send_int16_2buf(p,msg_len);
	p+=2;
	send_string_2buf(p,msg);
	return length;
}

int pack3091(char *buf,int index,int site)
{
	char *p;
	int length=9;
	short int name_len,url_len;
	p=buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,3091);
	p+=4;

	name_len = strlen(hall->hb[index].list[site].banker_name);
	url_len = strlen(hall->hb[index].list[site].banker_url);

	send_int32_2buf(p,hall->hb[index].list[site].banker_uid);
	p+=4;
	send_int32_2buf(p,hall->hb[index].list[site].hbid);
	p+=4;
	send_long64_2buf(p,hall->hb[index].list[site].banker_gold);
	p+=8;
	send_int16_2buf(p,hall->hb[index].list[site].mine);
	p+=2;
	send_int16_2buf(p,name_len);
	p+=2;
	send_string_2buf(p,hall->hb[index].list[site].banker_name);
	p+=name_len;
	send_int16_2buf(p,url_len);
	p+=2;
	send_string_2buf(p,hall->hb[index].list[site].banker_url);
	p+=url_len;
	
	length+=(name_len+url_len+22);
	send_int32_2buf(buf+1,length);
	return length;
}

int pack2092(char *buf,short int flag,long long int money,int hbid)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,23);
	p+=4;
	send_int32_2buf(p,2092);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,money);
	p+=8;
	send_int32_2buf(p,hbid);
	return 23;
}

int pack2093(char *buf,short int flag,long long int money,int tid)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,23);
	p+=4;
	send_int32_2buf(p,2093);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,money);
	p+=8;
	send_int32_2buf(p,tid);
	return 23;
}

int pack2094(char *buf,short int flag,long long int gold,long long int lose,long long int code,long long int gain,long long int money,int hbid,short int name_len,char *name)
{
	int length = 57+name_len;
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,2094);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_long64_2buf(p,gold);
	p+=8;
	send_long64_2buf(p,lose);
	p+=8;
	send_long64_2buf(p,code);
	p+=8;
	send_long64_2buf(p,gain);
	p+=8;
	send_long64_2buf(p,money);
	p+=8;
	send_int32_2buf(p,hbid);
	p+=4;
	send_int16_2buf(p,name_len);
	p+=2;
	send_string_2buf(p,name);
	return length;
}

int pack3094(char *buf,int hbid,int uid,short int name_len,char *name)
{
	int length = 19+name_len;
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,length);
	p+=4;
	send_int32_2buf(p,3094);
	p+=4;
	send_int32_2buf(p,hbid);
	p+=4;
	send_int32_2buf(p,uid);
	p+=4;
	send_int16_2buf(p,name_len);
	p+=2;
	send_string_2buf(p,name);
	return length;
}

int pack4094(char *buf,int hbid,short int time)
{
	char *p;
	p=buf;
	*p = '|';
	p++;
	send_int32_2buf(p,15);
	p+=4;
	send_int32_2buf(p,4094);
	p+=4;
	send_int32_2buf(p,hbid);
	p+=4;
	send_int16_2buf(p,time);
	return 15;
}

int packX097(int command,short int flag,char *buf,int index,int site)
{
	char *p;
	int i,length=37;
	short int name_len,url_len;
	name_len = strlen(hall->hb[index].list[site].banker_name);
	url_len = strlen(hall->hb[index].list[site].banker_url);
	length += (name_len+url_len);
	p=buf;
	*p = '|';
	p+=5;
	send_int32_2buf(p,command);
	p+=4;
	send_int16_2buf(p,flag);
	p+=2;
	send_int32_2buf(p,hall->hb[index].list[site].banker_uid);
	p+=4;
	send_long64_2buf(p,hall->hb[index].list[site].banker_gold);
	p+=8;
	send_int16_2buf(p,hall->hb[index].list[site].mine);
	p+=2;
	send_int16_2buf(p,name_len);
	p+=2;
	send_string_2buf(p,hall->hb[index].list[site].banker_name);
	p+=name_len;
	send_int16_2buf(p,url_len);
	p+=2;
	send_string_2buf(p,hall->hb[index].list[site].banker_url);
	p+=url_len;
	
	send_int32_2buf(p,hall->hb[index].list[site].begin_time+HB_ACTION_TIME);
	p+=4;
	send_int16_2buf(p,hall->hb[index].player);
	p+=2;
	send_int16_2buf(p,hall->hb[index].list[site].catch_num);
	p+=2;

	for (i=0;i<hall->hb[index].list[site].catch_num;i++)
	{
		name_len = strlen(hall->hb[index].list[site].user_catch[i].name);
		length+=(name_len+42);

		send_int32_2buf(p,hall->hb[index].list[site].user_catch[i].uid);
		p+=4;
		
		if (i==0)
		{
			if (hall->hb[index].list[site].catch_num < hall->hb[index].player)
			{
				send_long64_2buf(p,hall->hb[index].list[site].user_catch[i].gold - hall->hb[index].list[site].user_catch[i].gold%10);
				p+=8;
			}
			else
			{
				send_long64_2buf(p,hall->hb[index].list[site].user_catch[i].gold);
				p+=8;
			}
		}
		
		send_long64_2buf(p,hall->hb[index].list[site].user_catch[i].lose);
		p+=8;
		send_long64_2buf(p,hall->hb[index].list[site].user_catch[i].code);
		p+=8;
		send_long64_2buf(p,hall->hb[index].list[site].user_catch[i].gain);
		p+=8;
		send_int32_2buf(p,hall->hb[index].list[site].user_catch[i].catch_time);
		p+=4;
		send_int16_2buf(p,name_len);
		p+=2;
		send_string_2buf(p,hall->hb[index].list[site].user_catch[i].name);
		p+=name_len;
	}

	send_int32_2buf(buf+1,length);
	return length;
}