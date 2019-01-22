/*
 * myhiredis.h
 *
 *  Created on: 2014-5-15
 *      Author: yangxin
 */

#ifndef MYHIREDIS_H_
#define MYHIREDIS_H_

#if defined(__cplusplus)
extern "C"{
#endif

//#define DEBUG_MYHIREDIS_H_
#include <hiredis/hiredis.h>
#include<semaphore.h>
#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6000
#define REDIS_PWD  "123456"
#define EX_TIME 86400*7
#define EX_TIME_BZ_ZJH 3600
#define ZJH_BZ_MAX 10
#define EX_TIME_BZ_BJL 3600
#define BJL_BZ_MAX 1
/**
 * ConnectRedis 连接redis
 */
sem_t  redis_lock;
int redis_flag;

int ConnectRedis(const char *ip, int port,char *pwd, int nsec);
/**
 * setSessionByUid 通过uid设置session
 * @param uid 用户id
 * return 0 成功; -1 失败;
 */
int getUidByToken(char *token);
int getUserByUid(int uid,short int *group_num,int *group,short int *agent_num,int *agent);
int getUserByToken(char *token,long long int *gold,long long int *money,long long int *house,char *name,char *url,short int *sex,char *passwd,short int *group_num,int *group,short int *agent_num,int *agent);
//int updateUserGoldName(int uid,long long int gold,long long int money,long long int house,char *name,char *url,short int sex);
//int getGoldNameByUid(int uid,long long int *gold,long long int *money,long long int *house,char *name,char *url,short int *sex);
void closeRedis();
int getStatusByOrder(long long int order);

#if defined(__cplusplus)
}
#endif

#endif /* MYHIREDIS_H_ */
