/**
 * myhiredis.c
 *
 *  Created on: 2017-10-15
 *      Author: yangxin
 */

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "myshm.h"
#include "mybase.h"
#include "myhiredis.h"

redisContext *c; 
/**
 * ConnectRedis 连接redis
 */
int ConnectRedis(const char *ip, int port, char *pwd,int nsec)
{
	if(ip == NULL || strlen(ip) == 0){
		printf("redis ip is error\n");
		exit(1);
	}
	if (redis_flag > 0)
	{
		return 0;
	}
	struct timeval timeout = { nsec, 0 }; // 1.5 seconds
	c = redisConnectWithTimeout(ip, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        return -1;
    }
	//login
	//printf("login start...\n");
	redisReply *reply = (redisReply*)redisCommand(c,"auth %s",pwd);
	if (reply == NULL)
	{
		printf("login -111...\n");
		redisFree(c);
		return -1;
	}
	if (strcmp(reply->str,"OK") != 0)
	{
		printf("login -333...\n");
		freeReplyObject(reply);
		//closeRedis();
		return -1;
	}
	freeReplyObject(reply);
	redis_flag = 1;
	return 0;
}

 void closeRedis()
 {
	redisFree(c);
	redis_flag = 0;
 }
//!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0)
//REDIS_REPLY_INTEGER
//REDIS_REPLY_STRING
//REDIS_REPLY_NIL

int getUidByToken(char *token)
{
	int uid;
	redisReply *reply;
	sem_wait(&redis_lock);
	//连接redis
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}
	//获取uid
	reply = (redisReply*)redisCommand(c,"get %s",token);
	if (reply == NULL)
	{
		//redis error
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	if (reply->type != REDIS_REPLY_STRING)
	{
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	uid = atoi(reply->str);
	freeReplyObject(reply);
	//closeRedis();
	sem_post(&redis_lock);
	return uid;
}

int getUserByUid(int uid,short int *group_num,int *group,short int *agent_num,int *agent)
{
	int i,j,k1=0,k2=0,p=0;
	char *result[32];
	redisReply *reply;
	//printf("token = [%s]\n",token);
	sem_wait(&redis_lock);
	//连接redis
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}
	//获取uid
	//user_agent_%d(uid)
	//获取上级代理关系
	reply = (redisReply*)redisCommand(c,"get user_agent_%d",uid);
	if (reply == NULL)
	{
		printf("get user_agent redis error!\n");
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	if (reply->type != REDIS_REPLY_STRING)
	{
		printf("get user_agent reply error!\n");
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	//printf("reply->str=[%s]\n",reply->str);
	i = split(result,reply->str,",");
	if (i < 1)
	{
		//printf("i = %d\n",i);
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	p=0;
	*group_num = atoi(result[p]);
	p++;
	if (*group_num < 0 || *group_num > GROUP_MAX)
	{
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	for (i=0;i<*group_num;i++)
	{
		group[k1] = atoi(result[p]);
		p++;
		k1++;
		*agent_num = atoi(result[p]);
		p++;
		if (*agent_num < 0 || *agent_num > AGENT_MAX)
		{
			freeReplyObject(reply);
			//closeRedis();
			sem_post(&redis_lock);
			return -1;
		}
		for (j=0;j<*agent_num;j++)
		{
			agent[k2] = atoi(result[p]);
			p++;
			k2++;
		}
	}
	//
	freeReplyObject(reply);
	//closeRedis();
	sem_post(&redis_lock);
	return 0;
}

int getUserByToken(char *token,long long int *gold,long long int *money,long long int *house,char *name,char *url,short int *sex,char *passwd,short int *group_num,int *group,short int *agent_num,int *agent)
{
	int i,j,k1=0,k2=0,p=0,uid=-1;
	char *result[32];
	char *stop[32];
	redisReply *reply;
	//printf("token = [%s]\n",token);
	sem_wait(&redis_lock);
	//连接redis
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}
	//获取uid
	reply = (redisReply*)redisCommand(c,"get %s",token);
	if (reply == NULL)
	{
		//redis error
		printf("get uid redis error!\n");
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	if (reply->type != REDIS_REPLY_STRING)
	{
		freeReplyObject(reply);
		//closeRedis();
		printf("get uid reply error! [%s]\n",token);
		sem_post(&redis_lock);
		return -1;
	}

	uid = atoi(reply->str);
	freeReplyObject(reply);
	//user_agent_%d(uid)
	//获取金币，名称
	reply = (redisReply*)redisCommand(c,"get gold_name_%d",uid);
	if (reply == NULL)
	{
		printf("get gold_name redis error!\n");
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	if (reply->type != REDIS_REPLY_STRING)
	{
		printf("get gold_name reply error!\n");
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	//printf("reply->str=[%s]\n",reply->str);
	i = split(result,reply->str,",");
	if (i < 7)
	{
		//printf("i = %d\n",i);
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	*gold = strtoll(result[0],stop,10);
	*money = strtoll(result[1],stop,10);
	*house = strtoll(result[2],stop,10);
	sprintf(name,result[3]);
	sprintf(url,result[4]);
	*sex = atoi(result[5]);
	sprintf(passwd,result[6]);
	//
	freeReplyObject(reply);
	//获取上级代理关系
	reply = (redisReply*)redisCommand(c,"get user_agent_%d",uid);
	if (reply == NULL)
	{
		printf("get user_agent redis error!\n");
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	if (reply->type != REDIS_REPLY_STRING)
	{
		printf("get user_agent reply error!\n");
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	//printf("reply->str=[%s]\n",reply->str);
	i = split(result,reply->str,",");
	if (i < 1)
	{
		//printf("i = %d\n",i);
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	p=0;
	*group_num = atoi(result[p]);
	p++;
	if (*group_num < 0 || *group_num > GROUP_MAX)
	{
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	for (i=0;i<*group_num;i++)
	{
		group[k1] = atoi(result[p]);
		p++;
		k1++;
		*agent_num = atoi(result[p]);
		p++;
		if (*agent_num < 0 || *agent_num > AGENT_MAX)
		{
			freeReplyObject(reply);
			//closeRedis();
			sem_post(&redis_lock);
			return -1;
		}
		for (j=0;j<*agent_num;j++)
		{
			agent[k2] = atoi(result[p]);
			p++;
			k2++;
		}
	}
	//
	freeReplyObject(reply);
	//closeRedis();
	sem_post(&redis_lock);
	return uid;
}

/*int getGoldNameByUid(int uid,long long int *gold,long long int *money,long long int *house,char *name,char *url,short int *sex)
{
	int i;
	char *result[8];
	char *stop[32];
	redisReply *reply;
	sem_wait(&redis_lock);
	//连接redis
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}

	reply = (redisReply*)redisCommand(c,"get gold_name_%d",uid);
	if (reply == NULL)
	{
		printf("get gold_name redis error!\n");
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	if (reply->type != REDIS_REPLY_STRING)
	{
		printf("get gold_name reply error!\n");
		freeReplyObject(reply);
		sem_post(&redis_lock);
		return -1;
	}
	//printf("reply->str=[%s]\n",reply->str);
	i = split(result,reply->str,",");
	if (i != 6)
	{
		freeReplyObject(reply);
		sem_post(&redis_lock);
		return -1;
	}
	*gold = strtoll(result[0],stop,10);
	*money = strtoll(result[1],stop,10);
	*house = strtoll(result[2],stop,10);
	sprintf(name,result[3]);
	sprintf(url,result[4]);
	*sex = atoi(result[5]);
	//
	freeReplyObject(reply);
	sem_post(&redis_lock);
	return 0;
}*/
/*
int updateUserGoldName(int uid,long long int gold,long long int money,long long int house,char *name,char *url,short int sex)
{
	redisReply *reply;
	sem_wait(&redis_lock);
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}

	reply = (redisReply*)redisCommand(c,"set gold_name_%d %lld,%lld,%lld,%s,%s,%d ex %d",uid,gold,money,house,name,url,sex,EX_TIME);

	if (reply == NULL)
	{
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}

	freeReplyObject(reply);
	//closeRedis();
	sem_post(&redis_lock);
	return 0;
}*/

int getStatusByOrder(long long int order)
{
	int flag;
	redisReply *reply;
	sem_wait(&redis_lock);
	//连接redis
	if (ConnectRedis(REDIS_HOST,REDIS_PORT,REDIS_PWD, 2) < 0)
	{
		printf("redis connect->login fail\n");
		sem_post(&redis_lock);
		return -1;
	}
	//获取uid
	reply = (redisReply*)redisCommand(c,"get order_%lld",order);
	if (reply == NULL)
	{
		//redis error
		closeRedis();
		sem_post(&redis_lock);
		return -1;
	}
	if (reply->type != REDIS_REPLY_STRING)
	{
		freeReplyObject(reply);
		//closeRedis();
		sem_post(&redis_lock);
		return 0;
	}
	flag = atoi(reply->str);
	freeReplyObject(reply);
	//closeRedis();
	sem_post(&redis_lock);
	return flag;
}
#ifdef DEBUG_MYHIREDIS_H_ 
#include <time.h>

int split(char **arr, char *str, const char *del) 
{   
	char *s = strtok(str, del);   
	int i=0;
	while(s != NULL) 
	{   
		*arr++ = s;   
		s = strtok(NULL, del);   
		i++;
	}   
	return i;
}

int main(int argc,char* pargv[])
{
	int user;
	char name[64];

	// char tbname[1280];
	// sprintf(tbname,"name sex  birth bak ");
	initUser("yang",12);
	return 0;
	// putUid2DBSet("dbn",27136873);
	// hgetAllbyUid(7589);

	/****** 存 1K ~ 50K  性能测式************
	int size = 10;
	if (argc >=2)
		size  = atoi(pargv[1]);

	char *buff = malloc(size*1024); //10240 /102400 /51200
	srand(time(NULL));
	int count =10000; 
	struct timeval begin;
	struct timeval end;
	struct timezone tz;
	gettimeofday(&begin, &tz);
	int i = 0;
	for (i = 0; i < count; ++i)
	{
		int index = rand()%size*1024;
		char value = rand()%128;
		int k_index = rand()%32;
		char key[32];
		buff[index]=value;
		key[k_index]=value;
		const char * argv[10];
		size_t argvlen[10];
		int nargs = 3;
		argv[0] = "set";
		argvlen[0] = 3;
		
		argv[1] = key;
		argvlen[1] = sizeof(key);

		argv[2] = buff;
		argvlen[2] = size*1024;

		char *rtn;
		int rlen = 1024;
		rtn = ExeRedisCmd_ex(nargs,argv,argvlen,&rlen);
		// rtn[rlen] = '\0';
		// printf("%s %d \n",rtn,rlen );
		if(rtn)
			free(rtn);
		
	}
	if (buff)
		free(buff);
	gettimeofday(&end, &tz);
		long sec =  end.tv_sec - begin.tv_sec;
		long usec = end.tv_usec - begin.tv_usec;
		sec*=1000000;
		sec+=usec;
		float s = (float)sec/1000000;
		float num = count/s; 
	// printf("1秒钟可以写 %d 次\n",count );
	printf("写入大小%dk  总时间 %fs 总次数%d 1秒执行 %f次\n",size,s,count,num);
	*/


	// char * rstr;
	// int rlen;
	// int r= ExeRedisCmd("get","10001",strlen("10001"),NULL,0,&rstr,&rlen);
	// printf("r = %d len = %d str = %s\n",r,rlen,rstr);
	// free(rstr);

	// r= ExeRedisCmd("set","10002",strlen("10002"),"I am yzc",strlen("I am yzc"),&rstr,&rlen);
	// 	printf("r = %d len = %d str = %s\n",r,rlen,rstr);
	// return 0;


	// redisReply *reply;

	// reply = redisCommand(c,"SPOP set01");
	// printf("reply->type = %d reply->integer = %d reply->len = %d reply->str = %s\n",reply->type,reply->integer,reply->len,reply->str);
	// free(reply);
	// return 0;



	// int uid = getuidfromusername(3,"yzc");
	// printf("uid = %d\n",uid);

	// setuidbyusername(4,"yang",7589);

	// char kname[32];
	// char svalue[256];
	// sprintf(kname,"yangzhicheng");
	// sprintf(svalue,"yzc 1975 08 09 2018");

	// setdatabykey(strlen(kname),kname,strlen(svalue),svalue);

	// char *vptr;
	// int vlen;
	// getdatabykey(strlen(kname),kname,&vlen,&vptr);
	// printf("Get VPTR len = %d v = %s\n",vlen,vptr);
	// free(vptr);

	// return 1;
	// bf_node_t * wt = getdatafromredis(1,2713687,tbname);
	// char *p = wt->dptr;
	// printf("%c len = %d  cmd = %d uid = %d tbnm = %d \n",p[0],*((int *)(p+9)),*((int *)(p+13)),*((int *)(p+17)),*((int *)(p+21)));

	// PUTPHEAD(tbname,0,0,0,0);
	// PUTCOMMAND(tbname, OP_SAVDATA);
	// //包头+uid+表个数+四字母表名+表长度+内容+四字母表名+表长度+内容
	// char *tt = tbname + 17;
	// *((int *)tt) = 189666;tt+=4;
	// *((int *)tt) = 4;tt+=4;//表个数
	// sprintf(tt,"tb01");tt+=4;//四字母表名
	// *((int *)tt) = 4;tt+=4;//表长度
	// sprintf(tt,"yang");tt+=4;//内容
	// sprintf(tt,"tb02");tt+=4;//四字母表名
	// *((int *)tt) = 8;tt+=4;//表长度
	// sprintf(tt,"zhicheng");tt+=8;//内容
	// sprintf(tt,"tb03");tt+=4;//四字母表名
	// *((int *)tt) = 4;tt+=4;//表长度
	// sprintf(tt,"male");tt+=4;//内容
	// sprintf(tt,"tb04");tt+=4;//四字母表名
	// *((int *)tt) = 8;tt+=4;//表长度
	// sprintf(tt,"19750809");tt+=8;//内容

	// int i = savedata2redis(tbname);
	// printf("i = %d\n",i);


	return 0;
}
#endif
