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
	  MYSQL *rsk;
	  sock[s] = &mysql[s];
	  int reconnect = 1;
	  mysql_init(sock[s]);
	  mysql_options(sock[s], MYSQL_OPT_RECONNECT, &reconnect);
	  if (!(rsk = mysql_real_connect(sock[s],DBHOST,DBUSER,DBPASSWD,DBNAME,0,NULL,0)))
	  {
		mysql_close(sock[s]);
		fprintf(stderr,"Couldn't connect to engine!\n%s\n\n",mysql_error(sock[s]));
		//perror("");
		return -1;
	  }
	  mysql_flag[s] = 1;
	  return 1;
}

void CloseDb(int s)
{
	mysql_close(sock[s]);
	mysql_flag[s] = 0;
}

int init_house_config()
{
	char sql[128];
	sprintf(sql,"select gameid,type,grade,num,gold_type,gold_min,bottom,people,seat_max,cards_num from config_house order by id asc");
	char *stop[32];
	int i,j,k,s=0;
	long long int bottom;
	short int seat_max;
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
	j = 0;
	res[s] = mysql_store_result(&mysql[s]);
	while ((row[s] = mysql_fetch_row(res[s])))
	{
		j = i+atoi(row[s][3]);
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
		}
	}
	hall->normal_num = j;
	
	sem_post(&mysql_lock[s]);
	return 0;
}
/*
int init_robot_info()
{
	char sql[128];
	sprintf(sql,"select name from user_active where uid > 8999 and uid < 10000 order by uid asc");
	int i,s=0;
	
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

int init_game_pool()
{
	char sql[128];
	char *stop[32];
	sprintf(sql,"select money from game_pool where id = 1");

	int ret,s=0;
	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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
	
	sem_post(&mysql_lock[s]);
	return 0;
}
int update_game_pool(long long int pool_mix,long long int *pool_new)
{
	int ret,s=0;
	char sql[128];
	char *stop[32];


	sprintf(sql,"update game_pool set money=money+%lld where id = 1",pool_mix);

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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

	sem_post(&mysql_lock[s]);
	return 1;
}

int update_user_gold(int uid,long long int gold_mix,long long int money_mix,long long int house,long long int cake,long long int *gold_new,long long int *money_new)
{
	/*if (gold_mix == 0 && cake == 0 && money_mix == 0)
	{
		return -3;
	}*/
	int ret,s=0;
	char sql[256];
	char *stop[32];
	//*gold_new = -1;
	//*money_new = -1;

	sprintf(sql,"update user_active set gold=gold+%lld,money=money+%lld,house=%lld,cake=cake+%lld where uid=%d"\
		,gold_mix,money_mix,house,cake,uid);

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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
	
	sprintf(sql,"select gold,money from user_active where uid=%d",uid);
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
		*gold_new = strtoll(row[s][0],stop,10);
		*money_new = strtoll(row[s][1],stop,10);
		break;
	}

	sem_post(&mysql_lock[s]);
	return 0;
}

int add_user_prize_top(int uid,short int gid,short int grade,long long int bottom,long long int gold,short int check_type,short int *pai,int time)
{
	int ret,s=0;
	char sql[1024];
	sprintf(sql,"insert into user_prize_top (uid,gid,grade,bottom,gold,check_type,pai_1,pai_2,pai_3,pai_4,pai_5,create_time) values (%d,%d,%d,%lld,%lld,%d,%d,%d,%d,%d,%d,%d)",\
	uid,gid,grade,bottom,gold,check_type,pai[0],pai[1],pai[2],pai[3],pai[4],time);

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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
	int ret,s=1;
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
		default:
			return -1;
	}

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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
	char sql[1024];
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
	
	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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
	int ret,s=1;
	char sql[128];
	switch (index)
	{
		case 0:
			sprintf(sql,"update game_bjl_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		case 1:
			sprintf(sql,"update game_ttz_reward set banker_gain=%lld where qh=%d",gain,qh);
			break;
		default:
			return -1;
	}

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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

	int ret,s=1;
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
		default:
			return -1;
	}

	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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

int add_game_bjl_reward(int index,int banker,long long int banker_gold,char *banker_name,short int *win,long long int *bet,int time)
{
	
	if (bet[0] == 0 && bet[1] == 0 && bet[2] == 0 && bet[3] == 0)
	{
		return 0;
	}
	int ret,s=1;
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
		default:
			return -1;
	}

	sem_wait(&mysql_lock[s]);

	if(connectDb(s) < 0)
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
		default:
			sprintf(sql,"select win_4,win_3,win_2,win_1,win_0 from config_bjl_draw");
			break;
	}
	int ret,s=1;
	sem_wait(&mysql_lock[s]);
	if(connectDb(s) < 0)
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

	sem_post(&mysql_lock[s]);
	return 0;
}
