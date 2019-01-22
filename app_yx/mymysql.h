#if ! defined(TOOLS_MYSHQL_H)  /* { */
#define       TOOLS_MYSHQL_H        1

#if defined(__cplusplus)
extern "C"{
#endif
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "/usr/local/mysql/include/mysql.h"
//#define DEBUG_MYHIREDIS_H_
#define MYSQL_MAX 3
#define DBHOST     "127.0.0.1"
#define DBUSER     "root"
#define DBPASSWD   "123456"
#define DBNAME     "yi_xin"

sem_t mysql_lock[MYSQL_MAX];
int mysql_flag[MYSQL_MAX];

int connectDb(int s);
void CloseDb(int s);
int CheckDBAlive(int s);
int init_house_config();
int get_all_group_status(int index);
int update_group_gain(int group_id,long long int group_win,long long int group_code,long long int gain_qun,long long int gain_sys);
//int init_robot_info();
int update_user_gold(int uid,long long int money_mix,long long int house,long long int cake,long long int *money_new,long long int hb_money,int hb_num,long long int send_money,int send_num);

//int init_game_pool();
//int update_game_pool(long long int pool_mix,long long int *pool_new);
int add_user_prize_top(int uid,short int gid,short int type,long long int bottom,long long int gold,short int check_type,short int *pai,int time);

//int get_config_bjl(int index,int *win4,int *win3,int *win2,int *win1,int *win0);
int add_game_hb_record(int index,int site,int time);
int add_user_agent_record(int group_id,int *uid,long long int *gain,int banker,char *banker_name,int create_time);
int update_user_gold_by_uid(long long int gain,int uid);
int add_game_record(short int gid,short int type,short int grade,const long long int bottom,int *uid,const long long int *gold,const long long int *gold_old,const long long int *gold_new,char name[][USER_NAME_MAX],int time);
int add_game_bjl_reward(int index,int banker,long long int banker_gold,char *banker_name,short int *banker_card,short int *win,long long int *bet,int time);
int add_game_bjl_record(short int gid,int uid,int qh,long long int *bet,long long int *gain,long long int gain_total,const long long int gold_new,int time);
int add_system_bjl_record(int index,long long int total,int qh,int time);
int update_game_bjl_reward(int index,int qh,int *uid,long long int *gain,char name[][USER_NAME_MAX]);
int update_game_bjl_banker(int index,int qh,long long int gain);
int get_user_gold_name(int uid,long long int *money,long long int *house,char *name,char *url,short int *sex,char *passwd);
int add_user_exchange(int uid,short int type,char *name,long long int money,int time);
int get_user_status(int uid);
int add_user_transfer(int uid,int uid2,long long int money,char *info,int create_time);
#if defined(__cplusplus)
}
#endif
#endif     /*DUBLINDPROC_H */ /* } */
