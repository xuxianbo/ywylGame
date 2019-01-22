#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myshm.h"
#include "mymysql.h"
#include "mybase.h"

extern HALL *hall;
MYSQL mysql[MYSQL_MAX],*sock[MYSQL_MAX];
MYSQL_RES *res[MYSQL_MAX];
MYSQL_ROW row[MYSQL_MAX];

int connectDb(int s)
{
	  if (mysql_flag[s] > 0)
	  {
		  return 1;
	  }
	 // MYSQL *rsk;
	  //sock[s] = &mysql[s];
	  int reconnect = 1;
	  mysql_init(&mysql[s]);
	  mysql_options(&mysql[s], MYSQL_OPT_RECONNECT, &reconnect);
	  if (!(sock[s] = mysql_real_connect(&mysql[s],DBHOST,DBUSER,DBPASSWD,DBNAME,0,NULL,0)))
	  {
		//mysql_close(sock[s]);
		fprintf(stderr,"Couldn't connect to engine!\n%s\n\n",mysql_error(sock[s]));
		//perror("");
		return -1;
	  }
	  if(0 != mysql_query(sock[s],"set interactive_timeout = 86400*30*12")){
		mysql_close(sock[s]);
		printf(" ConnectDB Set Interactive failed.SQL Error:%s\n",mysql_error(sock[s]));
		return -1;
		}
	  mysql_flag[s] = 1;
	  return 1;
}

void CloseDb(int s)
{
	printf("Close DB%d!\n",s);
	mysql_close(sock[s]);
	mysql_flag[s] = 0;
	sock[s] = 0;
}

int CheckDBAlive(int s)
{
	if (sock[s] > 0)
	{
		if(mysql_ping(sock[s]) != 0){    //reconnect failed...Then try connect
		CloseDb(s);
		return connectDb(s);
		//printf("ConnectDB\n");
		}
	}
	else
	{
		return connectDb(s);
	}
	return 1;
}

int init_house_config()
{
	char sql[128];
	sprintf(sql,"select gameid,type,grade,num,gold_type,gold_min,bottom,people,seat_max,cards_num from config_house order by id asc");
	char *stop[32];
	int i,j,k,s=MYSQL_MAX-1,house_num;
	long long int bottom;
	short int seat_max;
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	i = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (i != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	i = 0;
	j = 0;
	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		house_num = atoi(row[s][3]);
		j = i+house_num;
		if (j > FORK_MAX)
		{
			j = FORK_MAX;
		}
		
		for (;i<j;i++)
		{
			hall->fork[i].gameid = (short)atoi(row[s][0]);
			hall->fork[i].type = (short)atoi(row[s][1]);
			hall->fork[i].grade = (short)atoi(row[s][2]);
			hall->fork[i].gold_type = (short)atoi(row[s][4]);
			hall->fork[i].gold_min = strtoll(row[s][5],stop,10);
			bottom = strtoll(row[s][6],stop,10);
			hall->fork[i].thread_max = atoi(row[s][7]);
			seat_max = atoi(row[s][8]);
			hall->fork[i].cards_num = atoi(row[s][9]);

			//printf("i=%d\n",i);
			if (hall->fork[i].grade != 100)
			{
				for (k=0;k<DESK_MAX;k++)
				{
					hall->fork[i].zjh[k].bottom = bottom;
					hall->fork[i].zjh[k].seat_max = seat_max;
				}
			}
			//printf("gameid=%d house_num=%d\n",hall->fork[i].gameid,house_num);
			switch (hall->fork[i].gameid)
			{
				case 0:
					hall->bjl[0].house_num = house_num;
					break;
				case 4:
					hall->bjl[1].house_num = house_num;
					break;
				case 10:
					hall->bjl[2].house_num = house_num;
					break;
				case 11:
					hall->bjl[3].house_num = house_num;
					break;
				case 12:
					hall->bjl[4].house_num = house_num;
					break;
			}

			if (hall->fork[i].gameid == 13)
			{
				hall->hb[hall->fork[i].type].house_num = house_num;
				hall->hb[hall->fork[i].type].bottom = bottom;
				hall->hb[hall->fork[i].type].player = seat_max;
				hall->hb[hall->fork[i].type].group_id = hall->fork[i].gameid*100+hall->fork[i].type;
			}
		}
	}
	hall->normal_num = j;
	mysql_free_result(res[s]);
	CloseDb(s);
	sem_post(&mysql_lock[s]);
	return 0;
}

int get_all_group_status(int index)
{
	int ret,s=0;
	char sql[128];

	sprintf(sql,"select status from group_base where id=%d",hall->hb[index].group_id);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	ret = mysql_real_query(&mysql[s],sql,strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		//printf("ret = %d\n",ret);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -2;
	}

	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		hall->hb[index].flag = atoi(row[s][0]);
	}
	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return 0;
}

int update_group_gain(int group_id,long long int group_win,long long int group_code,long long int gain_qun,long long int gain_sys)
{
	int ret,s=0;
	char sql[256];
	//*gold_new = -1;
	//*money_new = -1;

	sprintf(sql,"update group_base set group_win=group_win+%lld,group_code=group_code+%lld,group_gain=group_gain+%lld,server_gain=server_gain+%lld where id=%d",\
	group_win,group_code,gain_qun,gain_sys,group_id);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	//printf("mysql sock= %d\n",*(int *)(&mysql[s]));
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s],sql,strlen(sql));
	if (ret != 0)
	{
		//update 失败
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_user_agent_record(int group_id,int *uid,long long int *gain,int banker,char *banker_name,int create_time)
{
	int ret,s=0;
	char sql[1024];
	sprintf(sql,"insert into user_agent_record (group_id,uid1,gain1,uid2,gain2,uid3,gain3,uid4,gain4,uid5,gain5,banker,banker_name,create_time) \
	values (%d,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,'%s',%d)",\
	group_id,banker,gain[0],uid[0],gain[1],uid[1],gain[2],uid[2],gain[3],uid[3],gain[4],banker,banker_name,create_time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int update_user_gold_by_uid(long long int gain,int uid)
{
	int ret,s=0;
	char sql[128];
	sprintf(sql,"update user_active set money=money+%lld where uid = %d",gain,uid);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}
/*
int init_robot_info()
{
	char sql[128];
	sprintf(sql,"select name from user_active where uid > 8999 and uid < 10000 order by uid asc");
	int i,s=2;
	
	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	i = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (i != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	i = 0;

	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		sprintf(hall->robot_name[i],row[s][0]);
		i++;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}*/
/*
int init_game_pool()
{
	char sql[64];
	char *stop[32];
	sprintf(sql,"select money from game_pool where id = 1");

	int ret,s=2;
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	//printf("mysql_store_result\n");
	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		hall->pool_init = strtoll(row[s][0],stop,10);
		hall->pool = hall->pool_init;
		break;
	}
	
	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return 0;
}

int update_game_pool(long long int pool_mix,long long int *pool_new)
{
	int ret,s=0;
	char sql[64];
	char *stop[32];


	sprintf(sql,"update game_pool set money=money+%lld where id = 1",pool_mix);

	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	if (pool_mix < 0)
	{
		ret = mysql_real_query(&mysql[s], sql, strlen(sql));
		if (ret != 0)
		{
			writeFile(LOG_MYSQL,sql);
			CloseDb(s);
			sem_post(&mysql_lock[s]);
			return -1;
		}
	}

	sprintf(sql,"select money from game_pool where id = 1");
	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -2;
	}
	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		*pool_new = strtoll(row[s][0],stop,10);
		break;
	}
	mysql_free_result(res[s]);

	sem_post(&mysql_lock[s]);
	return 1;
}*/

int update_user_gold(int uid,long long int money_mix,long long int house,long long int cake,long long int *money_new,long long int hb_money,int hb_num,long long int send_money,int send_num)
{
	/*if (gold_mix == 0 && cake == 0 && money_mix == 0)
	{
		return -3;
	}*/
	int ret,s=1;
	char sql[256];
	char *stop[32];
	//*gold_new = -1;
	//*money_new = -1;

	sprintf(sql,"update user_active set money=money+%lld,house=%lld,cake=cake+%lld,hb_money=hb_money+%lld,hb_num=hb_num+%d,send_hb_money=send_hb_money+%lld,send_hb_num=send_hb_num+%d where uid=%d"\
		,money_mix,house,cake,hb_money,hb_num,send_money,send_num,uid);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	//printf("mysql sock= %d\n",*(int *)(&mysql[s]));
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		//update 失败
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	sprintf(sql,"select money from user_active where uid=%d",uid);
	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -2;
	}

	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		*money_new = strtoll(row[s][0],stop,10);
		break;
	}

	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return 0;
}

int get_user_gold_name(int uid,long long int *money,long long int *house,char *name,char *url,short int *sex,char *passwd)
{
	int ret,s=1;
	char sql[128];
	char *stop[32];

	sprintf(sql,"select money,house,name,pic_head,sex,pay_passwd from user_active where uid = %d",uid);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -2;
	}

	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		*money = strtoll(row[s][0],stop,10);
		*house = strtoll(row[s][1],stop,10);
		sprintf(name,row[s][2]);
		sprintf(url,row[s][3]);
		*sex = atoi(row[s][4]);
		sprintf(passwd,row[s][5]);
		break;
	}
	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return 0;
}

int get_user_status(int uid)
{
	int ret,s=0;
	char sql[64];

	sprintf(sql,"select status from user_base where uid = %d",uid);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	
	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -2;
	}

	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		ret = atoi(row[s][0]);
		break;
	}
	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return ret;
}

int add_user_prize_top(int uid,short int gid,short int grade,long long int bottom,long long int gold,short int check_type,short int *pai,int time)
{
	int ret,s=0;
	char sql[1024];
	sprintf(sql,"insert into user_prize_top (uid,gid,grade,bottom,gold,check_type,pai_1,pai_2,pai_3,pai_4,pai_5,create_time) values (%d,%d,%d,%lld,%lld,%d,%d,%d,%d,%d,%d,%d)",\
	uid,gid,grade,bottom,gold,check_type,pai[0],pai[1],pai[2],pai[3],pai[4],time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_game_hb_record(int index,int site,int time)
{
	int ret,s=0;
	char sql[4096];
	
	sprintf(sql,"insert into game_hb_record (group_id,banker,banker_name,banker_gold,banker_back,banker_win,banker_code,banker_gain,banker_mine,banker_mine_num,begin_time,\
	uid1,name1,time1,gold1,lose1,code1,gain1,\
	uid2,name2,time2,gold2,lose2,code2,gain2,\
	uid3,name3,time3,gold3,lose3,code3,gain3,\
	uid4,name4,time4,gold4,lose4,code4,gain4,\
	uid5,name5,time5,gold5,lose5,code5,gain5,\
	uid6,name6,time6,gold6,lose6,code6,gain6,\
	uid7,name7,time7,gold7,lose7,code7,gain7,\
	uid8,name8,time8,gold8,lose8,code8,gain8,\
	uid9,name9,time9,gold9,lose9,code9,gain9,\
	uid10,name10,time10,gold10,lose10,code10,gain10,\
	create_time) values \
	(%d,%d,'%s',%lld,%lld,%lld,%lld,%lld,%d,%d,%d,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d,'%s',%d,%lld,%lld,%lld,%lld,\
	%d)",\
	hall->hb[index].group_id,hall->hb[index].list[site].banker_uid,hall->hb[index].list[site].banker_name,hall->hb[index].list[site].banker_gold,\
	hall->hb[index].list[site].banker_back,hall->hb[index].list[site].banker_win,hall->hb[index].list[site].banker_code,\
	hall->hb[index].list[site].banker_gain,hall->hb[index].list[site].mine,hall->hb[index].list[site].mine_num,(int)hall->hb[index].list[site].begin_time,\
	hall->hb[index].list[site].user_catch[0].uid,hall->hb[index].list[site].user_catch[0].name,hall->hb[index].list[site].user_catch[0].catch_time,\
	hall->hb[index].list[site].user_catch[0].gold,hall->hb[index].list[site].user_catch[0].lose,hall->hb[index].list[site].user_catch[0].code,hall->hb[index].list[site].user_catch[0].gain,\
	hall->hb[index].list[site].user_catch[1].uid,hall->hb[index].list[site].user_catch[1].name,hall->hb[index].list[site].user_catch[1].catch_time,\
	hall->hb[index].list[site].user_catch[1].gold,hall->hb[index].list[site].user_catch[1].lose,hall->hb[index].list[site].user_catch[1].code,hall->hb[index].list[site].user_catch[1].gain,\
	hall->hb[index].list[site].user_catch[2].uid,hall->hb[index].list[site].user_catch[2].name,hall->hb[index].list[site].user_catch[2].catch_time,\
	hall->hb[index].list[site].user_catch[2].gold,hall->hb[index].list[site].user_catch[2].lose,hall->hb[index].list[site].user_catch[2].code,hall->hb[index].list[site].user_catch[2].gain,\
	hall->hb[index].list[site].user_catch[3].uid,hall->hb[index].list[site].user_catch[3].name,hall->hb[index].list[site].user_catch[3].catch_time,\
	hall->hb[index].list[site].user_catch[3].gold,hall->hb[index].list[site].user_catch[3].lose,hall->hb[index].list[site].user_catch[3].code,hall->hb[index].list[site].user_catch[3].gain,\
	hall->hb[index].list[site].user_catch[4].uid,hall->hb[index].list[site].user_catch[4].name,hall->hb[index].list[site].user_catch[4].catch_time,\
	hall->hb[index].list[site].user_catch[4].gold,hall->hb[index].list[site].user_catch[4].lose,hall->hb[index].list[site].user_catch[4].code,hall->hb[index].list[site].user_catch[4].gain,\
	hall->hb[index].list[site].user_catch[5].uid,hall->hb[index].list[site].user_catch[5].name,hall->hb[index].list[site].user_catch[5].catch_time,\
	hall->hb[index].list[site].user_catch[5].gold,hall->hb[index].list[site].user_catch[5].lose,hall->hb[index].list[site].user_catch[5].code,hall->hb[index].list[site].user_catch[5].gain,\
	hall->hb[index].list[site].user_catch[6].uid,hall->hb[index].list[site].user_catch[6].name,hall->hb[index].list[site].user_catch[6].catch_time,\
	hall->hb[index].list[site].user_catch[6].gold,hall->hb[index].list[site].user_catch[6].lose,hall->hb[index].list[site].user_catch[6].code,hall->hb[index].list[site].user_catch[6].gain,\
	hall->hb[index].list[site].user_catch[7].uid,hall->hb[index].list[site].user_catch[7].name,hall->hb[index].list[site].user_catch[7].catch_time,\
	hall->hb[index].list[site].user_catch[7].gold,hall->hb[index].list[site].user_catch[7].lose,hall->hb[index].list[site].user_catch[7].code,hall->hb[index].list[site].user_catch[7].gain,\
	hall->hb[index].list[site].user_catch[8].uid,hall->hb[index].list[site].user_catch[8].name,hall->hb[index].list[site].user_catch[8].catch_time,\
	hall->hb[index].list[site].user_catch[8].gold,hall->hb[index].list[site].user_catch[8].lose,hall->hb[index].list[site].user_catch[8].code,hall->hb[index].list[site].user_catch[8].gain,\
	hall->hb[index].list[site].user_catch[9].uid,hall->hb[index].list[site].user_catch[9].name,hall->hb[index].list[site].user_catch[9].catch_time,\
	hall->hb[index].list[site].user_catch[9].gold,hall->hb[index].list[site].user_catch[0].lose,hall->hb[index].list[site].user_catch[0].code,hall->hb[index].list[site].user_catch[9].gain,\
	time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_game_bjl_record(short int gid,int uid,int qh,long long int *bet,long long int *gain,long long int gain_total,const long long int gold_new,int time)
{
	if (uid < 0)
	{
		return 0;
	}
	int ret,s=0;
	char sql[1024];
	switch (gid)
	{
		case 0:
			sprintf(sql,"insert into game_bjl_record (uid,qh,bet1,bet2,bet3,bet4,gain1,gain2,gain3,gain4,gain_total,gold_new,create_time) values (%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%d)",\
			uid,qh,bet[0],bet[1],bet[2],bet[3],gain[0],gain[1],gain[2],gain[3],gain_total,gold_new,time);
			break;
		case 1:
			sprintf(sql,"insert into game_ttz_record (uid,qh,bet1,bet2,bet3,gain1,gain2,gain3,gain_total,gold_new,create_time) values (%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%d)",\
			uid,qh,bet[0],bet[1],bet[2],gain[0],gain[1],gain[2],gain_total,gold_new,time);
			break;
		case 2:
			sprintf(sql,"insert into game_lhd_record (uid,qh,bet1,bet2,bet3,gain1,gain2,gain3,gain_total,gold_new,create_time) values (%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%d)",\
			uid,qh,bet[0],bet[1],bet[2],gain[0],gain[1],gain[2],gain_total,gold_new,time);
			break;
		case 3:
			sprintf(sql,"insert into game_bzw_record (uid,qh,bet1,bet2,gain1,gain2,gain_total,gold_new,create_time) values (%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%d)",\
			uid,qh,bet[0],bet[1],gain[0],gain[1],gain_total,gold_new,time);
			break;
		case 4:
			sprintf(sql,"insert into game_bmw_record (uid,qh,bet1,bet2,bet3,bet4,bet5,bet6,bet7,bet8,gain1,gain2,gain3,gain4,gain5,gain6,gain7,gain8,gain_total,gold_new,create_time) values (%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%d)",\
			uid,qh,bet[0],bet[1],bet[2],bet[3],bet[4],bet[5],bet[6],bet[7],gain[0],gain[1],gain[2],gain[3],gain[4],gain[5],gain[6],gain[7],gain_total,gold_new,time);
			break;
		default:
			return -1;
	}
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_game_record(short int gid,short int type,short int grade,const long long int bottom,int *uid,const long long int *gold,const long long int *gold_old,const long long int *gold_new,char name[][USER_NAME_MAX],int time)
{
	int ret,s=0;
	char sql[2048];
	switch (gid)
	{
		case 1:
			sprintf(sql,"insert into game_zjh_record (type,grade,bottom,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid1_gold_old,uid1_gold_new,uid2_gold,uid2_gold_old,uid2_gold_new,uid3_gold,uid3_gold_old,uid3_gold_new,uid4_gold,uid4_gold_old,uid4_gold_new,uid5_gold,uid5_gold_old,uid5_gold_new,uid6_gold,uid6_gold_old,uid6_gold_new,uid7_gold,uid7_gold_old,uid7_gold_new,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time) values (%d,%d,%lld,%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,'%s','%s','%s','%s','%s','%s','%s',%d)",\
			type,grade,bottom,uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],gold[0],gold_old[0],gold_new[0],gold[1],gold_old[1],gold_new[1],gold[2],gold_old[2],gold_new[2],gold[3],gold_old[3],gold_new[3],gold[4],gold_old[4],gold_new[4],gold[5],gold_old[5],gold_new[5],gold[6],gold_old[6],gold_new[6],name[0],name[1],name[2],name[3],name[4],name[5],name[6],time);
			break;
		case 2:
			sprintf(sql,"insert into game_ox_record (type,grade,bottom,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid1_gold_old,uid1_gold_new,uid2_gold,uid2_gold_old,uid2_gold_new,uid3_gold,uid3_gold_old,uid3_gold_new,uid4_gold,uid4_gold_old,uid4_gold_new,uid5_gold,uid5_gold_old,uid5_gold_new,uid6_gold,uid6_gold_old,uid6_gold_new,uid7_gold,uid7_gold_old,uid7_gold_new,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time) values (%d,%d,%lld,%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,'%s','%s','%s','%s','%s','%s','%s',%d)",\
			type,grade,bottom,uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],gold[0],gold_old[0],gold_new[0],gold[1],gold_old[1],gold_new[1],gold[2],gold_old[2],gold_new[2],gold[3],gold_old[3],gold_new[3],gold[4],gold_old[4],gold_new[4],gold[5],gold_old[5],gold_new[5],gold[6],gold_old[6],gold_new[6],name[0],name[1],name[2],name[3],name[4],name[5],name[6],time);
			break;
		case 5:
			if (gold[0] == 0)
			{
				return 0;
			}
			sprintf(sql,"insert into game_wzq_record (type,grade,uid1,uid2,uid1_gold,uid1_gold_old,uid1_gold_new,uid2_gold,uid2_gold_old,uid2_gold_new,uid1_name,uid2_name,create_time) values (%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,'%s','%s',%d)",\
			type,grade,uid[0],uid[1],gold[0],gold_old[0],gold_new[0],gold[1],gold_old[1],gold_new[1],name[0],name[1],time);
			break;
		case 6:
			sprintf(sql,"insert into game_mj_record (type,grade,bottom,uid1,uid2,uid3,uid4,uid1_gold,uid1_gold_old,uid1_gold_new,uid2_gold,uid2_gold_old,uid2_gold_new,uid3_gold,uid3_gold_old,uid3_gold_new,uid4_gold,uid4_gold_old,uid4_gold_new,uid1_name,uid2_name,uid3_name,uid4_name,create_time) values (%d,%d,%lld,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,'%s','%s','%s','%s',%d)",\
			type,grade,bottom,uid[0],uid[1],uid[2],uid[3],gold[0],gold_old[0],gold_new[0],gold[1],gold_old[1],gold_new[1],gold[2],gold_old[2],gold_new[2],gold[3],gold_old[3],gold_new[3],name[0],name[1],name[2],name[3],time);
			break;
		default:
			return 0;
	}
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int update_game_bjl_banker(int index,int qh,long long int gain)
{
	int ret,s=0;
	char sql[128];
	switch (index)
	{
		case 0:
			sprintf(sql,"update game_bjl_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		case 1:
			sprintf(sql,"update game_ttz_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		case 2:
			sprintf(sql,"update game_lhd_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		case 3:
			sprintf(sql,"update game_bzw_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		case 4:
			sprintf(sql,"update game_bmw_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		default:
			return -1;
	}
	//printf("sql=[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int update_game_bjl_reward(int index,int qh,int *uid,long long int *gain,char name[][USER_NAME_MAX])
{
	if (uid[0] < 0)
	{
		return 0;
	}

	int ret,s=0;
	char sql[1024];
	switch (index)
	{
		case 0:
			sprintf(sql,"update game_bjl_reward set uid1=%d,uid2=%d,uid3=%d,uid4=%d,uid5=%d,gain1=%lld,gain2=%lld,gain3=%lld,gain4=%lld,gain5=%lld,name1='%s',name2='%s',name3='%s',name4='%s',name5='%s' where qh=%d",\
			uid[0],uid[1],uid[2],uid[3],uid[4],gain[0],gain[1],gain[2],gain[3],gain[4],name[0],name[1],name[2],name[3],name[4],qh);
			break;
		case 1:
			sprintf(sql,"update game_ttz_reward set uid1=%d,uid2=%d,uid3=%d,uid4=%d,uid5=%d,gain1=%lld,gain2=%lld,gain3=%lld,gain4=%lld,gain5=%lld,name1='%s',name2='%s',name3='%s',name4='%s',name5='%s' where qh=%d",\
			uid[0],uid[1],uid[2],uid[3],uid[4],gain[0],gain[1],gain[2],gain[3],gain[4],name[0],name[1],name[2],name[3],name[4],qh);
			break;
		case 2:
			sprintf(sql,"update game_lhd_reward set uid1=%d,uid2=%d,uid3=%d,uid4=%d,uid5=%d,gain1=%lld,gain2=%lld,gain3=%lld,gain4=%lld,gain5=%lld,name1='%s',name2='%s',name3='%s',name4='%s',name5='%s' where qh=%d",\
			uid[0],uid[1],uid[2],uid[3],uid[4],gain[0],gain[1],gain[2],gain[3],gain[4],name[0],name[1],name[2],name[3],name[4],qh);
			break;
		case 3:
			sprintf(sql,"update game_bzw_reward set uid1=%d,uid2=%d,uid3=%d,uid4=%d,uid5=%d,gain1=%lld,gain2=%lld,gain3=%lld,gain4=%lld,gain5=%lld,name1='%s',name2='%s',name3='%s',name4='%s',name5='%s' where qh=%d",\
			uid[0],uid[1],uid[2],uid[3],uid[4],gain[0],gain[1],gain[2],gain[3],gain[4],name[0],name[1],name[2],name[3],name[4],qh);
			break;
		case 4:
			sprintf(sql,"update game_bmw_reward set uid1=%d,uid2=%d,uid3=%d,uid4=%d,uid5=%d,gain1=%lld,gain2=%lld,gain3=%lld,gain4=%lld,gain5=%lld,name1='%s',name2='%s',name3='%s',name4='%s',name5='%s' where qh=%d",\
			uid[0],uid[1],uid[2],uid[3],uid[4],gain[0],gain[1],gain[2],gain[3],gain[4],name[0],name[1],name[2],name[3],name[4],qh);
			break;
		default:
			return -1;
	}
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_game_bjl_reward(int index,int banker,long long int banker_gold,char *banker_name,short int *banker_card,short int *win,long long int *bet,int time)
{
	
	if (bet[0] == 0 && bet[1] == 0 && bet[2] == 0 && bet[3] == 0 && bet[4] == 0 && bet[5] == 0 && bet[6] == 0 && bet[7] == 0)
	{
		return 0;
	}
	int ret,s=0;
	char sql[1024];
	

	switch (index)
	{
		case 0:
			sprintf(sql,"insert into game_bjl_reward (banker,banker_gold,banker_name,a_gold,b_gold,c_gold,d_gold,a_win,b_win,c_win,d_win,create_time) values (%d,%lld,'%s',%lld,%lld,%lld,%lld,%d,%d,%d,%d,%d)",\
	banker,banker_gold,banker_name,bet[0],bet[1],bet[2],bet[3],win[0],win[1],win[2],win[3],time);
			break;
		case 1:
			sprintf(sql,"insert into game_ttz_reward (banker,banker_gold,banker_name,a_gold,b_gold,c_gold,a_win,b_win,c_win,create_time) values (%d,%lld,'%s',%lld,%lld,%lld,%d,%d,%d,%d)",\
	banker,banker_gold,banker_name,bet[0],bet[1],bet[2],win[0],win[1],win[2],time);
			break;
		case 2:
			sprintf(sql,"insert into game_lhd_reward (banker,banker_gold,banker_name,a_gold,b_gold,c_gold,a_win,b_win,c_win,create_time) values (%d,%lld,'%s',%lld,%lld,%lld,%d,%d,%d,%d)",\
	banker,banker_gold,banker_name,bet[0],bet[1],bet[2],win[0],win[1],win[2],time);
			break;
		case 3:
			sprintf(sql,"insert into game_bzw_reward (banker,banker_gold,banker_name,pai_1,pai_2,pai_3,a_gold,b_gold,a_win,b_win,create_time) values (%d,%lld,'%s',%d,%d,%d,%lld,%lld,%d,%d,%d)",\
	banker,banker_gold,banker_name,banker_card[0],banker_card[1],banker_card[2],bet[0],bet[1],win[0],win[1],time);
			break;
		case 4:
			sprintf(sql,"insert into game_bmw_reward (banker,banker_gold,banker_name,a_gold,b_gold,c_gold,d_gold,e_gold,f_gold,g_gold,h_gold,a_win,b_win,c_win,d_win,e_win,f_win,g_win,h_win,create_time) values (%d,%lld,'%s',%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%d,%d,%d,%d,%d,%d,%d,%d,%d)",\
	banker,banker_gold,banker_name,bet[0],bet[1],bet[2],bet[3],bet[4],bet[5],bet[6],bet[7],win[0],win[1],win[2],win[3],win[4],win[5],win[6],win[7],time);
			break;
		default:
			return -1;
	}
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);

	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s],sql,(int)strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	ret = mysql_insert_id(&mysql[s]);
	sem_post(&mysql_lock[s]);
	return ret;
}

int add_system_bjl_record(int index,long long int total,int qh,int time)
{
	if (total == 0)
	{
		return 0;
	}
	int ret,s=0;
	char sql[128];
	
	sprintf(sql,"insert into system_robot_record (bjl_index,qh,total,create_time) values (%d,%d,%lld,%d)",index,qh,total,time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);

	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s],sql,(int)strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	ret = mysql_insert_id(&mysql[s]);
	sem_post(&mysql_lock[s]);
	return ret;
}

int add_user_exchange(int uid,short int type,char *name,long long int money,int time)
{
	int ret,s=0;
	char sql[1024];
	sprintf(sql,"insert into user_duihuan (uid,type,name,money,create_time) values (%d,%d,'%s',%lld,%d)",\
	uid,type,name,money,time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_user_transfer(int uid,int uid2,long long int money,char *info,int create_time)
{
	int ret,s=0;
	char sql[1024];
	sprintf(sql,"insert into user_transfer (uid,player_uid,num,transfer_desc,create_time) \
	values (%d,%d,%lld,'%s',%d)",uid,uid2,money,info,create_time);
	//printf("[%s]\n",sql);
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	ret = mysql_insert_id(&mysql[s]);
	sem_post(&mysql_lock[s]);
	return ret;
}


/*
int get_config_bjl(int index,int *win4,int *win3,int *win2,int *win1,int *win0)
{
	//printf("get_config_bjl\n");
	char sql[128];
	switch (index)
	{
		case 0:
			sprintf(sql,"select win_4,win_3,win_2,win_1,win_0 from config_bjl_draw");
			break;
		case 1:
			sprintf(sql,"select win_4,win_3,win_2,win_1,win_0 from config_ttz_draw");
			break;
		case 2:
			sprintf(sql,"select win_4,win_3,win_2,win_1,win_0 from config_lhd_draw");
			break;
		default:
			sprintf(sql,"select win_4,win_3,win_2,win_1,win_0 from config_bjl_draw");
			break;
	}
	int ret,s=0;
	sem_wait(&mysql_lock[s]);
	if(CheckDBAlive(s) < 0)
	{
		writeFile(LOG_MYSQL,sql);
		sem_post(&mysql_lock[s]);
		return -1;
	}

	ret = mysql_real_query(&mysql[s], sql, strlen(sql));
	if (ret != 0)
	{
		writeFile(LOG_MYSQL,sql);
		CloseDb(s);
		sem_post(&mysql_lock[s]);
		return -1;
	}
	//printf("mysql_store_result\n");
	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		*win4 = atoi(row[s][0]);
		*win3 = atoi(row[s][1]);
		*win2 = atoi(row[s][2]);
		*win1 = atoi(row[s][3]);
		*win0 = atoi(row[s][4]);
	}

	mysql_free_result(res[s]);
	sem_post(&mysql_lock[s]);
	return 0;
}
*/