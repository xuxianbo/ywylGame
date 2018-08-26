#if ! defined(TOOLS_MYSHQL_H)  /* { */
#define       TOOLS_MYSHQL_H        1

#if defined(__cplusplus)
extern "C"{
#endif
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "/usr/include/mysql/mysql.h"
//#define DEBUG_MYHIREDIS_H_
#define MYSQL_MAX 2
#define DBHOST     "127.0.0.1"
#define DBUSER     "ywyl_app"
#define DBPASSWD   "ywyl_app20180709"
#define DBNAME     "ywyl"

sem_t mysql_lock[MYSQL_MAX];
int mysql_flag[MYSQL_MAX];

int connectDb(int s);
void CloseDb(int s);
int init_house_config();
//int init_robot_info();
int update_user_gold(int uid,long long int gold_mix,long long int money_mix,long long int house,long long int cake,long long int *gold_new,long long int *money_new);

int init_game_pool();
int update_game_pool(long long int pool_mix,long long int *pool_new);
int add_user_prize_top(int uid,short int gid,short int type,long long int bottom,long long int gold,short int check_type,short int *pai,int time);

int get_config_bjl(int index,int *win4,int *win3,int *win2,int *win1,int *win0);
int add_game_record(short int gid,short int type,short int grade,const long long int bottom,int *uid,const long long int *gold,const long long int *gold_old,const long long int *gold_new,char name[][USER_NAME_MAX],int time);
int add_game_bjl_reward(int index,int banker,long long int banker_gold,char *banker_name,short int *win,long long int *bet,int time);
int add_game_bjl_record(short int gid,int uid,int qh,long long int *bet,long long int *gain,long long int gain_total,const long long int gold_new,int time);
int update_game_bjl_reward(int index,int qh,int *uid,long long int *gain,char name[][USER_NAME_MAX]);
int update_game_bjl_banker(int index,int qh,long long int gain);
#if defined(__cplusplus)
}
#endif
#endif     /*DUBLINDPROC_H */ /* } */
