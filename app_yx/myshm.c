#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>
#include "myshm.h"
#include "mybase.h"
/*
 *the numbers of children which will determine 
 *the limit of the value store in the share memory
*/
HALL *hall;
static int myshm_id = -1;
static key_t i_shm_key;
/**
 * the Id of semaphore
*/
void   set_shm_key(key_t i_key)
{
	myshm_id = -1;
	i_shm_key = i_key;
}

void init_myshm(int i_flag)
{
	int i,j,k,m,n;
	int ShmSize = sizeof(HALL);
	set_shm_key(SHM_KEY);
	//i_shm_key = ftok(".",1);
//	printf("%d,%d\n",i_shm_key,ShmSize);
	myshm_id = shmget(i_shm_key,ShmSize,0666|IPC_CREAT);
	if(myshm_id==-1)
	{
	//	printf("%s\n",strerror(errno));
		perror("crt shm err!\n");
		exit(-1);
	}
	if((hall = (HALL *) shmat(myshm_id,0,0))==(HALL *) -1)
	{
		perror("shm err:");
		exit(-1);
	}
	//printf("myshm flag = %d\n",i_flag);
	if(i_flag == 1)
	{
		hall->flag = 0;
		hall->time_now = time(NULL);
		hall->time_over = 0;
		hall->thread = 0;
		//hall->thread_total = 0;
		hall->normal_num = 0;
		//hall->game_num = 0;
		hall->site_msg = 0;
		sem_init(&(hall->aaa_lock),1,1);

		hall->pool_init = 0;
		hall->pool		= 0;
		/*
		for (i=0;i<ROBOT_NUM_MAX;i++)
		{
			memset(hall->robot_name[i],'\0',USER_NAME_MAX);
		}*/

		for (i=0;i<MSG_LIST_MAX;i++)
		{
			hall->aaa_msg[i].length = 0;
			hall->aaa_msg[i].id = 0;
			memset(hall->aaa_msg[i].msg,'\0',MSG_BUF_MAX);
		}
		//
		for(i=0;i<FORK_MAX;i++)
		{
			hall->fork[i].status = 0;
			hall->fork[i].child_pid = 0;
			hall->fork[i].thread = 0;
			hall->fork[i].thread_max = 0;
			hall->fork[i].gold_type = 0;
			hall->fork[i].gold_min = 0;
			hall->fork[i].gameid = 0;
			hall->fork[i].type = 0;
			hall->fork[i].grade = 0;
			hall->fork[i].cards_num = 0;
			hall->fork[i].fd[0] = -1;
			hall->fork[i].fd[1] = -1;

			for (j=0;j<DESK_MAX;j++)
			{
				for (k=0;k<CARD_MAX;k++)
				{
					hall->fork[i].zjh[j].cards[k] = 0;
				}
			
				hall->fork[i].zjh[j].flag = 0;
				hall->fork[i].zjh[j].people = 0;
				hall->fork[i].zjh[j].banker = -1;
				hall->fork[i].zjh[j].turn = -1;
				hall->fork[i].zjh[j].round = 1;
				hall->fork[i].zjh[j].player = 0;
				hall->fork[i].zjh[j].start_time = 0;
				hall->fork[i].zjh[j].in_total = 0;
				hall->fork[i].zjh[j].order_max = 0;
				hall->fork[i].zjh[j].card_num = 0;

				hall->fork[i].zjh[j].seat_dg = -1;
				/*
				hall->fork[i].zjh[j].card_type = 0;
				hall->fork[i].zjh[j].card_max = 0;
				hall->fork[i].zjh[j].power = -1;
				hall->fork[i].zjh[j].is_pay = 0;
				hall->fork[i].zjh[j].level[0] = 2;
				hall->fork[i].zjh[j].level[1] = 2;
				for (k=0;k<4;k++)
				{
					hall->fork[i].zjh[j].pay[k] = -1;
				}*/
				
				hall->fork[i].zjh[j].mj_zm = 2;
				hall->fork[i].zjh[j].mj_change = 1;
				hall->fork[i].zjh[j].mj_times = 4;
				hall->fork[i].zjh[j].mj_dg = 1;
				hall->fork[i].zjh[j].mj_yjjd = 0;
				hall->fork[i].zjh[j].mj_mqzz = 1;
				hall->fork[i].zjh[j].mj_td = 1;


				for (k=0;k<SEAT_MAX;k++)
				{
					hall->fork[i].zjh[j].uid[k] = -1;
					hall->fork[i].zjh[j].gold[k] = 0;
					hall->fork[i].zjh[j].gold_old[k] = 0;
					hall->fork[i].zjh[j].gold_new[k] = 0;
					memset(hall->fork[i].zjh[j].name[k],'\0',USER_NAME_MAX);

					hall->fork[i].zjh[j].seat[k].user = -1;
					hall->fork[i].zjh[j].seat[k].begin_time = 0;
					hall->fork[i].zjh[j].seat[k].in_all = 0;
					hall->fork[i].zjh[j].seat[k].status = 0;
					hall->fork[i].zjh[j].seat[k].card_num = 0;
					hall->fork[i].zjh[j].seat[k].ranking = 0;
					
					hall->fork[i].zjh[j].seat[k].ba_gang = 0;
					hall->fork[i].zjh[j].seat[k].point_gang = 0;
					hall->fork[i].zjh[j].seat[k].lose_num = 0;
					hall->fork[i].zjh[j].seat[k].flag_chi = 0;					
					hall->fork[i].zjh[j].seat[k].flag_peng = 0;				
					hall->fork[i].zjh[j].seat[k].flag_gang = 0;
					hall->fork[i].zjh[j].seat[k].flag_win = 0;	
					hall->fork[i].zjh[j].seat[k].lack = 0;
					hall->fork[i].zjh[j].seat[k].card_in = 0;
					hall->fork[i].zjh[j].seat[k].card_out = 0;
					hall->fork[i].zjh[j].seat[k].card_out_num = 0;
					hall->fork[i].zjh[j].seat[k].num_peng = 0;
					hall->fork[i].zjh[j].seat[k].num_gang = 0;
					hall->fork[i].zjh[j].seat[k].card_gang = 0;
					hall->fork[i].zjh[j].seat[k].win_times = -1;

					for (m=0;m<4;m++)
					{
						hall->fork[i].zjh[j].seat[k].card_change[m] = 0;
					}
					

					for (m=0;m<MJ_CPK_MAX;m++)
					{
						hall->fork[i].zjh[j].seat[k].card_peng[m] = 0;
					}

					for (m=0;m<MJ_CPK_MAX;m++)
					{
						hall->fork[i].zjh[j].seat[k].gang[m].flag = 0;
						hall->fork[i].zjh[j].seat[k].gang[m].card_gang = 0;
						hall->fork[i].zjh[j].seat[k].gang[m].seat[0] = -1;
						hall->fork[i].zjh[j].seat[k].gang[m].seat[1] = -1;
						hall->fork[i].zjh[j].seat[k].gang[m].seat[2] = -1;
						hall->fork[i].zjh[j].seat[k].gang[m].gold[0] = 0;
						hall->fork[i].zjh[j].seat[k].gang[m].gold[1] = 0;
						hall->fork[i].zjh[j].seat[k].gang[m].gold[2] = 0;
					}

					for (m=0;m<1;m++)
					{
						
						for (n=0;n<7;n++)
						{
							hall->fork[i].zjh[j].seat[k].win[m].card_type[n] = 0;
						}
						hall->fork[i].zjh[j].seat[k].win[m].geng = 0;
						hall->fork[i].zjh[j].seat[k].win[m].times = 0;
						hall->fork[i].zjh[j].seat[k].win[m].card_win = 0;
						hall->fork[i].zjh[j].seat[k].win[m].flag = 0;
						hall->fork[i].zjh[j].seat[k].win[m].td = 0;
						hall->fork[i].zjh[j].seat[k].win[m].jg = 0;
						hall->fork[i].zjh[j].seat[k].win[m].hd = 0;
						hall->fork[i].zjh[j].seat[k].win[m].gh = 0;
						hall->fork[i].zjh[j].seat[k].win[m].seat[0] = -1;
						hall->fork[i].zjh[j].seat[k].win[m].seat[1] = -1;
						hall->fork[i].zjh[j].seat[k].win[m].seat[2] = -1;
						hall->fork[i].zjh[j].seat[k].win[m].gold[0] = 0;
						hall->fork[i].zjh[j].seat[k].win[m].gold[1] = 0;
						hall->fork[i].zjh[j].seat[k].win[m].gold[2] = 0;
					}
					
					for (m=0;m<HAND_MAX;m++)
					{
						hall->fork[i].zjh[j].seat[k].card_out_list[m] = 0;
						hall->fork[i].zjh[j].seat[k].card[m] = 0;
						hall->fork[i].zjh[j].seat[k].card_copy[m] = 0;
					}
					sem_init(&(hall->fork[i].zjh[j].seat[k].lock),1,1);
					//
				}
				sem_init(&(hall->fork[i].zjh[j].lock),1,1);

				//私人场
				hall->fork[i].zjh[j].bottom = 0;
				hall->fork[i].zjh[j].round_max = 0;
				hall->fork[i].zjh[j].desk_status = 0;
				hall->fork[i].zjh[j].seat_max = 0;
				hall->fork[i].zjh[j].ip = 0;
				hall->fork[i].zjh[j].model = 1;
				hall->fork[i].zjh[j].kan = 1;
			}
		}
		//百家乐
		for (m=0;m<BJL_GAME_MAX;m++)
		{
			hall->bjl[m].banker_times = 0;
			hall->bjl[m].qh = 0;
			hall->bjl[m].banker_flag = 0;
			hall->bjl[m].house_num = 10;
			for (i=0;i<BJL_SEAT_MAX;i++)
			{
				hall->bjl[m].seat[i].user = -1;
				hall->bjl[m].seat[i].status = 0;
				sem_init(&(hall->bjl[m].seat[i].lock),1,1);
			}

			for (i=0;i<BJL_HAND_MAX;i++)
			{
				hall->bjl[m].banker_card[i] = 0;
			}

			for (i=0;i<BJL_BET_MAX;i++)
			{
				for (j=0;j<BJL_HAND_MAX;j++)
				{
					hall->bjl[m].card[i][j] = 0;
				}
			}
			
			hall->bjl[m].gold_total = 0;
			for (i=0;i<BJL_BET_MAX;i++)
			{
				hall->bjl[m].win[i] = 0;
				hall->bjl[m].gold[i] = 0;
				hall->bjl[m].robot_gold[i] = 0;
				if (i < 4)
				{
					hall->bjl[m].no_flag[i] = (rand()%(4-i))*10 + rand()%20 + 1;
				}
				else
				{
					hall->bjl[m].no_flag[i] = 0;
				}
			}

			for (i=0;i<BJL_BET_MAX;i++)
			{
				hall->bjl[m].gold_win[i] = 0;
			}
			
			for (i=0;i<BANKER_MAX;i++)
			{
				hall->bjl[m].banker_list[i]  = -1;
			}
			
			sem_init(&(hall->bjl[m].lock_banker),1,1);
			sem_init(&(hall->bjl[m].lock_bet),1,1);
			hall->bjl[m].list_num = 0;
			hall->bjl[m].banker = -1;
			hall->bjl[m].flag = 0;
			hall->bjl[m].start_time = 0;

			for (i=0;i<WINNER_MAX;i++)
			{
				hall->bjl[m].win_max[i] = 0;
				hall->bjl[m].win_uid[i] = -1;
				memset(hall->bjl[m].win_name[i],'\0',USER_NAME_MAX);
				memset(hall->bjl[m].win_url[i],'\0',USER_HEAD_URL);
			}
			sem_init(&(hall->bjl[m].lock_winner),1,1);
			
			for (i=0;i<BJL_L_MAX;i++)
			{
				for (j=0;j<BJL_BET_MAX;j++)
				{
					hall->bjl[m].list[i][j] = -1;
				}
			}

			for (i=0;i<BJL_HOUSE_MAX;i++)
			{
				hall->bjl[m].flag_seat[i] = 0;
				hall->bjl[m].flag_over[i] = 0;
				hall->bjl[m].flag_game[i] = 0;
				hall->bjl[m].flag_msg[i] = 0;
			}

			for (i=0;i<MSG_LIST_MAX;i++)
			{
				hall->bjl[m].bjl_msg[i].length = 0;
				memset(hall->bjl[m].bjl_msg[i].msg,'\0',128);
			}
			hall->bjl[m].msg_list_flag = -1;
			sem_init(&(hall->bjl[m].lock_msg),1,1);
		}
		//红包
	
		for (m=0;m<HB_MAX;m++)
		{
			hall->hb[m].group_id = 0;
			hall->hb[m].flag = 0;
			hall->hb[m].house_num = 0;
			hall->hb[m].bottom = 0;
			hall->hb[m].player = 0;
			hall->hb[m].msg_list_flag = -1;
			hall->hb[m].list_head = 0;
			sem_init(&(hall->hb[m].lock_msg),1,1);

			for (i=0;i<MSG_LIST_MAX;i++)
			{
				hall->hb[m].hb_msg[i].length = 0;
				memset(hall->hb[m].hb_msg[i].msg,'\0',128);
			}

			for (i=0;i<BJL_HOUSE_MAX;i++)
			{
				hall->hb[m].flag_msg[i] = 0;
				for (j=0;j<HB_LIST_MAX;j++)
				{
					hall->hb[m].flag_start[i][j] = 0;
					hall->hb[m].flag_over[i][j] = 0;
				}
			}

			for (i=0;i<HB_LIST_MAX;i++)
			{
				sem_init(&(hall->hb[m].list[i].lock),1,1);
				hall->hb[m].list[i].hbid = -1;
				hall->hb[m].list[i].user = -1;
				hall->hb[m].list[i].banker_uid = -1;
				memset(hall->hb[m].list[i].banker_name,'\0',USER_NAME_MAX);
				memset(hall->hb[m].list[i].banker_url,'\0',USER_HEAD_URL);
				hall->hb[m].list[i].banker_gold = 0;
				hall->hb[m].list[i].banker_win = 0;
				hall->hb[m].list[i].banker_gain = 0;
				hall->hb[m].list[i].banker_back = 0;
				hall->hb[m].list[i].banker_code = 0;
				hall->hb[m].list[i].mine = -1;
				hall->hb[m].list[i].mine_num = 0;
				hall->hb[m].list[i].status = 0;
				hall->hb[m].list[i].begin_time = 0;
				hall->hb[m].list[i].catch_num = 0;

				for (j=0;j<USER_CATCH_MAX;j++)
				{
					memset(hall->hb[m].list[i].user_catch[j].name,'\0',USER_NAME_MAX);
					hall->hb[m].list[i].user_catch[j].notice = 0;
					hall->hb[m].list[i].user_catch[j].uid = -1;
					hall->hb[m].list[i].user_catch[j].catch_time = 0;
					hall->hb[m].list[i].user_catch[j].gold = 0;
					hall->hb[m].list[i].user_catch[j].gain = 0;
					hall->hb[m].list[i].user_catch[j].lose = 0;
					hall->hb[m].list[i].user_catch[j].code = 0;
				}
			}
			
		}
		
		
		//
		hall->ip_num = 0;
		sem_init(&(hall->ip_lock),1,1);

		for(i=0;i<USER_MAX;i++)
		{
			init_user(i);
		}
		
	}
	else
	{
		//get myshm
		//printf("get myshm\n");
	}
}

void init_desk(int forkid,int desk,int flag)
{
	int k,m,n;
	//重置一局游戏
	//printf("fork=%d desk=%d 重置一局游戏\n",forkid,desk);

	for (k=0;k<SEAT_MAX;k++)
	{
		//if (hall->fork[forkid].zjh[desk].seat[k].status > 0)
		//{
			//有人
			hall->fork[forkid].zjh[desk].seat[k].card_num = 0;
			hall->fork[forkid].zjh[desk].seat[k].begin_time = 0;
			hall->fork[forkid].zjh[desk].seat[k].in_all = 0;
			if (hall->fork[forkid].zjh[desk].seat[k].status > 0)
			{
				hall->fork[forkid].zjh[desk].seat[k].status = 1;
			}
			else
			{
				hall->fork[forkid].zjh[desk].seat[k].status = 0;
			}
			
			hall->fork[forkid].zjh[desk].seat[k].ranking = 0;
			hall->fork[forkid].zjh[desk].seat[k].ba_gang = 0;
			hall->fork[forkid].zjh[desk].seat[k].point_gang = 0;
			hall->fork[forkid].zjh[desk].seat[k].lose_num = 0;
			hall->fork[forkid].zjh[desk].seat[k].flag_chi = 0;					
			hall->fork[forkid].zjh[desk].seat[k].flag_peng = 0;				
			hall->fork[forkid].zjh[desk].seat[k].flag_gang = 0;
			hall->fork[forkid].zjh[desk].seat[k].flag_win = 0;	
			hall->fork[forkid].zjh[desk].seat[k].lack = 0;
			hall->fork[forkid].zjh[desk].seat[k].card_in = 0;
			hall->fork[forkid].zjh[desk].seat[k].card_out = 0;
			hall->fork[forkid].zjh[desk].seat[k].card_out_num = 0;
			hall->fork[forkid].zjh[desk].seat[k].num_peng = 0;
			hall->fork[forkid].zjh[desk].seat[k].num_gang = 0;
			hall->fork[forkid].zjh[desk].seat[k].card_gang = 0;
			hall->fork[forkid].zjh[desk].seat[k].win_times = -1;

			for (m=0;m<4;m++)
			{
				hall->fork[forkid].zjh[desk].seat[k].card_change[m] = 0;
			}

			for (m=0;m<MJ_CPK_MAX;m++)
			{
				hall->fork[forkid].zjh[desk].seat[k].card_peng[m] = 0;
			}

			for (m=0;m<MJ_CPK_MAX;m++)
			{
				hall->fork[forkid].zjh[desk].seat[k].gang[m].flag = 0;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].card_gang = 0;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].seat[0] = -1;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].seat[1] = -1;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].seat[2] = -1;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].gold[0] = 0;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].gold[1] = 0;
				hall->fork[forkid].zjh[desk].seat[k].gang[m].gold[2] = 0;
			}

			for (m=0;m<1;m++)
			{
				
				for (n=0;n<7;n++)
				{
					hall->fork[forkid].zjh[desk].seat[k].win[m].card_type[n] = 0;
				}
				hall->fork[forkid].zjh[desk].seat[k].win[m].geng = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].times = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].card_win = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].flag = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].td = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].jg = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].hd = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].gh = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].seat[0] = -1;
				hall->fork[forkid].zjh[desk].seat[k].win[m].seat[1] = -1;
				hall->fork[forkid].zjh[desk].seat[k].win[m].seat[2] = -1;
				hall->fork[forkid].zjh[desk].seat[k].win[m].gold[0] = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].gold[1] = 0;
				hall->fork[forkid].zjh[desk].seat[k].win[m].gold[2] = 0;
			}
			
			for (m=0;m<HAND_MAX;m++)
			{
				hall->fork[forkid].zjh[desk].seat[k].card_out_list[m] = 0;
				hall->fork[forkid].zjh[desk].seat[k].card[m] = 0;
				//hall->fork[forkid].zjh[desk].seat[k].card_copy[m] = 0;
			}
		//}
	}

	hall->fork[forkid].zjh[desk].order_max = hall->fork[forkid].zjh[desk].bottom;
	hall->fork[forkid].zjh[desk].start_time = 0;
	hall->fork[forkid].zjh[desk].card_num = 0;
	//hall->fork[forkid].zjh[desk].card_max = 0;
	hall->fork[forkid].zjh[desk].flag = 0;
	hall->fork[forkid].zjh[desk].round = 1;
	hall->fork[forkid].zjh[desk].in_total = 0;		
	hall->fork[forkid].zjh[desk].player = 0;
	hall->fork[forkid].zjh[desk].seat_dg = -1;

	if (flag)
	{
		//重置桌子
		hall->fork[forkid].zjh[desk].people = 0;
		hall->fork[forkid].zjh[desk].desk_status = 0;
		hall->fork[forkid].zjh[desk].banker = -1;
		hall->fork[forkid].zjh[desk].turn = -1;
		hall->fork[forkid].zjh[desk].start_time = 0;
		hall->fork[forkid].zjh[desk].order_max = 0;

		for (k=0;k<SEAT_MAX;k++)
		{
			hall->fork[forkid].zjh[desk].uid[k] = -1;
			hall->fork[forkid].zjh[desk].gold[k] = 0;
			hall->fork[forkid].zjh[desk].gold_old[k] = 0;
			hall->fork[forkid].zjh[desk].gold_new[k] = 0;
			hall->fork[forkid].zjh[desk].seat[k].begin_time = 0;
			hall->fork[forkid].zjh[desk].seat[k].in_all = 0;
			hall->fork[forkid].zjh[desk].seat[k].status = 0;
			hall->fork[forkid].zjh[desk].seat[k].card_num = 0;
			for (m=0;m<HAND_MAX;m++)
			{
				hall->fork[forkid].zjh[desk].seat[k].card[m] = 0;
			}
		}
	}

	printf("重置桌子完成 init forkid->%d,desk->%d flag=%d\n",forkid,desk,flag);
}

void init_user(int i)
{
	int j,m;
	memset(hall->myip[i].ip,'\0',IP_MAX);
	memset(hall->user[i].passwd,'\0',PASSWD_MAX);
	hall->myip[i].num = 0;
	//user
	hall->user[i].status = 0;
	hall->user[i].login = 0;
	hall->user[i].sex = 0;
	hall->user[i].uid = -1;
	hall->user[i].hds = -1;
	memset(hall->user[i].name,'\0',USER_NAME_MAX);
	memset(hall->user[i].url,'\0',USER_HEAD_URL);
	memset(hall->user[i].ip,'\0',IP_MAX);
	hall->user[i].gold_init = 0;
	hall->user[i].gold = 0;
	hall->user[i].money_init = 0;
	hall->user[i].money = 0;
	hall->user[i].stock = 0;
	hall->user[i].cake = 0;
	hall->user[i].socket = -1;
	hall->user[i].forkid = -1;
	hall->user[i].time = 0;
	hall->user[i].msg_gold_update = 0;
	sem_init(&(hall->user[i].lock_user),1,1);
	//百家乐数据

	for (m=0;m<BJL_GAME_MAX;m++)
	{
		hall->user[i].banker_flag[m] = 0;
		hall->user[i].bjl_flag[m] = 0;
		hall->user[i].bjl_balance[m] = 0;
		for (j=0;j<BJL_BET_MAX;j++)
		{
			hall->user[i].bet[m][j] = 0;
		}
		for (j=0;j<BJL_BET_MAX;j++)
		{
			hall->user[i].gold_win[m][j] = 0;
		}
	}
	//
	//充值订单
	for (m=0;m<ORDER_MAX;m++)
	{
		hall->user[i].order[m].order = 0;
		hall->user[i].order[m].create_time = 0;
		hall->user[i].order[m].update_time = 0;
	}
	hall->user[i].order_num = 0;
	sem_init(&(hall->user[i].lock_order),1,1);
	//红包
	for (m=0;m<HB_MAX;m++)
	{
		hall->user[i].hb_send_num[m] = 0;
	}
	
	hall->user[i].agent_num = 0;
	for (m=0;m<AGENT_MAX;m++)
	{
		hall->user[i].agent[m] = -1;
	}
	
	hall->user[i].group_num = 0;
	for (m=0;m<GROUP_MAX;m++)
	{
		hall->user[i].group[m] = -1;
	}

	//
	sem_init(&(hall->user[i].lock_user_msg),1,1);
	hall->user[i].user_msg_num = 0;
	for (m=0;m<MSG_LIST_MAX;m++)
	{
		hall->user[i].user_msg[m].length = 0;
		memset(hall->user[i].user_msg[m].msg,'\0',MSG_BUF_MAX);
	}
}

void drop_myshm()
{
	shmdt(hall);
}
