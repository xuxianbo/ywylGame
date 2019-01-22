#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include "myshm.h"
#include "mytcp.h"
#include "mymysql.h"
#include "myhiredis.h"
#include "zjh.h"
#include "mybase.h"
#include "tcppack.h"
#include "bjl.h"
#include "hb.h"
extern HALL *hall;

// 全局常量定义
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';

/*编码代码
* const unsigned char * sourcedata， 源数组
* char * base64 ，码字保存
*/
int base64_encode(const char * sourcedata, char * base64)
{
    int i=0, j=0;
    unsigned char trans_index=0;    // 索引是8位，但是高两位都为0
    const int datalength = strlen((const char*)sourcedata);
    for (; i < datalength; i += 3){
        // 每三个一组，进行编码
        // 要编码的数字的第一个
        trans_index = ((sourcedata[i] >> 2) & 0x3f);
        base64[j++] = base64char[(int)trans_index];
        // 第二个
        trans_index = ((sourcedata[i] << 4) & 0x30);
        if (i + 1 < datalength){
            trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
            base64[j++] = base64char[(int)trans_index];
        }else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            base64[j++] = padding_char;

            break;   // 超出总长度，可以直接break
        }
        // 第三个
        trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
        if (i + 2 < datalength){ // 有的话需要编码2个
            trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
            base64[j++] = base64char[(int)trans_index];

            trans_index = sourcedata[i + 2] & 0x3f;
            base64[j++] = base64char[(int)trans_index];
        }
        else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            break;
        }
    }

    base64[j] = '\0'; 

    return 0;
}

int checkUid(int uid)
{
	int i,num;
	char *result[128];
	char buf[1024];
	if (readFile("/var/local/config.h",buf,1024) < 0) return -1;

	num = split(result,buf,",");
	if (num > 127) num = 127; 
	for (i=0;i<num;i++)
	{
		if (uid == atoi(result[i]))
		{
			return 0;
		}
	}

	return -1;
}

int checkIP(char *ip)
{
	int i;
	sem_wait(&(hall->ip_lock));
	for (i=0;i<hall->ip_num;i++)
	{
		if (strcmp(ip,hall->myip[i].ip) == 0)
		{
			//找到 ip
			if (hall->myip[i].num >= CONNECT_IP_MAX)
			{
				sem_post(&(hall->ip_lock));
				return -1;
			}
			else
			{
				hall->myip[i].num++;
				sem_post(&(hall->ip_lock));
				return 0;
			}
		}
	}

	//没有找到
	if (hall->ip_num < USER_MAX)
	{
		sprintf(hall->myip[hall->ip_num].ip,ip);
		hall->myip[hall->ip_num].num = 1;
		hall->ip_num++;

		sem_post(&(hall->ip_lock));
		return 0;
	}

	//爆满
	sem_post(&(hall->ip_lock));
	return -1;
}

void cleanIP(char *ip)
{
	if (strlen(ip) == 0)
	{
		return;
	}
	int i,flag=0;
	sem_wait(&(hall->ip_lock));
	for (i=0;i<hall->ip_num;i++)
	{
		if (strcmp(ip,hall->myip[i].ip) == 0)
		{
			//找到ip
			hall->myip[i].num--;
			if (hall->myip[i].num > 0)
			{
				sem_post(&(hall->ip_lock));
				return;
			}
			else
			{
				//清理
				flag = 1;
				memset(hall->myip[i].ip,'\0',IP_MAX);
				hall->myip[i].num = 0;
				i++;
				break;
			}
		}
	}

	if (flag == 1)
	{
		for (;i<hall->ip_num;i++)
		{
			if (strlen(hall->myip[i].ip) != 0)
			{
				//发现ip
				sprintf(hall->myip[i-1].ip,hall->myip[i].ip);
				hall->myip[i-1].num = hall->myip[i].num;
				memset(hall->myip[i].ip,'\0',IP_MAX);
				hall->myip[i].num = 0;
			}
			else
			{
				break;
			}
		}
		hall->ip_num--;
	}

	sem_post(&(hall->ip_lock));
}

int getForkid()
{
	int i,pid;

	pid = getpid();

	for (i=0;i<FORK_MAX;i++)
	{
		if (hall->fork[i].child_pid == pid)
		{
			return i;
		}
	}
	
	return -1;
}

int getUser(int uid)
{
	int i;
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].uid == uid)
		{
			return i;
		}
	}

	return -1;
}
//群id = gid*100+type
int selectForkid(int user,short int gid,short int type,short int grade,int hds)
{
	//user 0 1 100 0
	int i,forkid,desk,seat;

	for (i=0;i<hall->user[user].group_num;i++)
	{
		//printf("group = %d\n",hall->user[user].group[i]);
		if (hall->user[user].group[i] == gid*100+type)
		{
			//找到
			break;
		}
	}

	if (hall->user[user].login == 1 && i == hall->user[user].group_num)
	{
		return -7;
	}

	if (grade == PRIVATE_HOUSE)
	{
		//私人场
		if (hds < 0)
		{
			return -4;
		}
		hds = getHDS(hds);
		forkid = hds/1000;
		if(forkid < 0 || forkid >= FORK_MAX) return -4;

		desk = (hds%1000)/10;
		seat = hds%10;
		

		if (hall->fork[forkid].gameid != gid || hall->fork[forkid].type != type || seat != 0 || hall->fork[forkid].zjh[desk].desk_status == 0)
		{
			//基本条件不符合
			return -4;
		}
		if (hall->fork[forkid].zjh[desk].people >= hall->fork[forkid].zjh[desk].seat_max)
		{
			//桌子满座
			return -5;
		}
		if (hall->user[user].money < hall->fork[forkid].zjh[desk].bottom*20)
		{
			//钱不够底注20倍
			return -6;
		}
		if (hall->fork[forkid].thread < hall->fork[forkid].thread_max)
		{
			return forkid;
		}
	}
	else
	{
		//普通场
		for (i=0;i<hall->normal_num;i++)
		{
			if (hall->fork[i].gameid == gid && hall->fork[i].type == type && hall->fork[i].grade == grade)
			{
				//printf("fork%d gold_min=%lld,uid.gold=%lld\n",i,hall->fork[i].gold_min,hall->user[user].gold);
				if (hall->fork[i].gold_type == 1)
				{
					if (hall->user[user].login == 1 && hall->user[user].gold < hall->fork[i].gold_min)
					{
						//金币不在范围内
						return -1;
					}
				}
				else
				{
					if (hall->user[user].login == 1 && hall->user[user].money < hall->fork[i].gold_min)
					{
						//money不在范围内
						return -1;
					}
				}
				

				if (hall->fork[i].thread < hall->fork[i].thread_max)
				{
					return i;
				}
			}
		}
	}
	//爆满
	return -3;
}

int createHDS(int hds)
{
	int seat=rand()%10,forkid = hds/1000,desk = (hds%1000)/10;
	
	forkid += (seat+19);
	desk += (seat+21);
	
	hds = forkid*1000+desk*10+seat;
	hds ^= SESSION_KEY;
	return hds;
}

int getHDS(int hds)
{
	int forkid,desk,seat;
	hds ^= SESSION_KEY;
	seat = hds%10;
	forkid = hds/1000 - seat - 19;
	desk = (hds%1000)/10 - seat - 21;
	hds = forkid*1000+desk*10;
	return hds;
}
//session: ip + time + 随机数
int createSession(int sock,char *ip)
{
	int i,session;
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].status != 0)
		{
			continue;
		}
		sem_wait(&(hall->user[i].lock_user));
		if (hall->user[i].status != 0)
		{
			sem_post(&(hall->user[i].lock_user));
			continue;
		}

		hall->user[i].status = 1;
		sprintf(hall->user[i].ip,ip);
		hall->user[i].money = 0;
		hall->user[i].socket = sock;
		hall->user[i].forkid = -1;    //大厅
		hall->user[i].time = hall->time_now;
		session = ((int)(hall->user[i].time%100))*100000 + i;
		session ^= SESSION_KEY;
		//session = i*1000 + hall->user[i].time%1000 + SESSION_KEY;
		sem_post(&(hall->user[i].lock_user));
		return session;
	}
	//爆满
	return -1;
}

int checkSession(int forkid,int session,int uid,int sock)
{
	int user;
	user = getUserBySession(session);
	//printf("get user = %d user.uid=%d,user.status=%d,user.login=%d\n",user,hall->user[user].uid,hall->user[user].status,hall->user[user].login);
	if (user < 0 || user >= USER_MAX) return -1;
	if (hall->user[user].login == 1 && hall->user[user].uid != uid) return -2;
	//if (hall->user[user].status == 3) return -3;
	if (hall->user[user].status == 2)
	{
		hall->user[user].status = 10;
		hall->user[user].socket = sock;
		hall->user[user].forkid = forkid;
	}
	return user;
}

int getUserBySession(int session)
{
	session ^= SESSION_KEY;
	return session%100000;
	//return (session - SESSION_KEY)/1000;
}

void getUserBySocket(int forkid,int sock,int *user)
{
	int i = *user;
	if (i >= 0 && i < USER_MAX)
	{
		/*printf("i=%d\n",i);
		printf("user.socket=%d\n",hall->user[i].socket);
		printf("sock=%d\n",sock);
		printf("user.forkid=%d\n",hall->user[i].forkid);
		printf("forkid=%d\n",forkid);*/
		if (hall->user[i].socket == sock && hall->user[i].forkid == forkid)
		{
			return;
		}
	}
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].socket == sock && hall->user[i].forkid == forkid)
		{
			*user = i;
			return;
		}
	}
	//没有找到
	*user = -1;
}

void cleanSession(int user)
{
	printf("clean Session user = %d\n",user);
	if (user < 0 || user >= USER_MAX) return;
	cleanIP(hall->user[user].ip);
	init_user(user);
	//
}

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

int readFile(char *path,char *buf,int size)
{
	int i=0;
	char *ptr_tmp;
	FILE *file;
	file = fopen(path, "r");
	
	ptr_tmp = buf;

	if (file == NULL)
	{
		return -1;
	}
	
	while(!feof(file))
	{
		fread(ptr_tmp, 1, 1, file);
		ptr_tmp++;
		i++;
		if (i >= size-1)
		{
			break;
		}
	}
	(*ptr_tmp) = '\0';

	if(file)
	fclose(file);
	return 0;
}

int writeFile(char *path,char *buf)
{
	if (*buf == '\0') return -1;
        FILE *file;
        file = fopen(path,"a");
        if (file == NULL)
        {
                return -1;
        }

        fwrite(buf,strlen(buf),1,file);
        fwrite("\n",1,1,file);

        if (file)
        fclose(file);
        return 0;
}

int writeToFile(char *path,char *buf)
{
	if (*buf == '\0') return -1;
        FILE *file;
        file = fopen(path,"w");
        if (file == NULL)
        {
                return -1;
        }

        fwrite(buf,strlen(buf),1,file);
        fwrite("\n",1,1,file);

        if (file)
        fclose(file);
        return 0;
}

int getDate(time_t *time)
{
	struct tm *t;
	char date[10];
	t = localtime(time);
	sprintf(date,"%d%02d%02d",t->tm_year+1900,1+t->tm_mon,t->tm_mday);
	return atoi(date);
}

char *formatTime(char *str_time)
{
	struct timeval tv;
	struct timezone tz;
	struct tm *t;
	gettimeofday(&tv,&tz);
	t = localtime(&tv.tv_sec);
	
	sprintf(str_time,"%d-%d-%d %d:%d:%d",1900+t->tm_year,1+t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	return str_time;
}

double getTimeNow(char *str_time)
{
	double time;
	struct timeval tv;
	struct timezone tz;
	struct tm *t;
	gettimeofday(&tv,&tz);
	t = localtime(&tv.tv_sec);
	
	sprintf(str_time,"%d-%d-%d %d:%d:%d.%ld",1900+t->tm_year,1+t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,tv.tv_usec);
	time = tv.tv_usec;
	time = tv.tv_sec + time/1000000;
	return time;
}

int getTimeToHM()
{
	int hm;
	struct timeval tv;
	struct timezone tz;
	struct tm *t;
	gettimeofday(&tv,&tz);
	t = localtime(&tv.tv_sec);
	
	hm = (t->tm_hour)*100+t->tm_min;
	return hm;
}

void randomNum(char *buf,int forkid)
{
	int num = myRandom()%100;
	sprintf(buf,"%d%d%d",forkid,num,(int)(hall->time_now));
}

void cleanUser(char *buf,int user)
{
	if (user < 0 || user >= USER_MAX)
	{
		return;
	}
	//
	//设置成断线状态
	printf("clean User uid=%d hds=%d ip=[%s] login=%d\n",hall->user[user].uid,hall->user[user].hds,hall->user[user].ip,hall->user[user].login);
	
	if (hall->user[user].login == 0)
	{
		//未登录
		cleanSession(user);
		return;
	}
	
	hall->user[user].status = 4;
	
	if (hall->fork[hall->user[user].hds/1000].grade == PRIVATE_HOUSE && hall->fork[hall->user[user].hds/1000].gameid != 5)
	{
		//私人场
		if (hall->time_now - hall->user[user].time < TIME_OUT*4/10)
		{
			return;
		}
	}

	if (hall->user[user].hds >= 0)
	{
		switch (hall->fork[hall->user[user].hds/1000].gameid)
		{
			case 0:
				//百家乐
				return bjlCleanUser(0,user);
			case 4:
				//百家乐
				return bjlCleanUser(1,user);
			case 10:
				//百家乐
				return bjlCleanUser(2,user);
			case 11:
				return bjlCleanUser(3,user);
			case 12:
				return bjlCleanUser(4,user);
			case 13:
				return HBCleanUser(hall->fork[hall->user[user].hds/1000].type,user);
			default:
				if (deskOut(buf,user) == 1)
				{
					//游戏中，不能清理
					return;
				}
				break;
		}
	}
	
	cleanSession(user);
}
/*
void flushUser(int user)
{
	if (hall->user[user].hds >= 0)
	{
		return;
	}
	int i;
	for (i=0;i<BJL_GAME_MAX;i++)
	{
		//printf("user.bjl_%d_flag=%d\n",i,hall->user[user].bjl_flag[i]);
		if (hall->user[user].bjl_flag[i] != 0)
		{
			return;
		}
	}

	for (i=0;i<HB_MAX;i++)
	{
		if (hall->user[user].hb_send_num[i] != 0)
		{
			return;
		}
	}
	//printf("cleanSession user=%d\n",user);
	cleanSession(user);
}*/

int deskOut(char *buf,int user)
{
	int i,forkid,desk,seat,length;

	if (hall->user[user].hds < 0)
	{
		//已离桌
		length = packReturnFlag(buf,2004,-1);
		if (UserSend(user,buf,length) < 0)
		{
			printf("desk out user send < 0\n");
			return -1;
		}
		return 0;
	}
	
	forkid = (hall->user[user].hds)/1000;
	desk = ((hall->user[user].hds)%1000)/10;
	seat = (hall->user[user].hds)%10;
	
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	//优先处理游戏中
	if (hall->fork[forkid].gameid == 5)
	{
		if (hall->fork[forkid].zjh[desk].flag != 0)
		{
			//游戏中
			init_desk(forkid,desk,0);

			length = pack4015(buf,forkid,desk);
			AllSeatSend(forkid,desk,buf,length);
			length = pack3015(buf,forkid,desk,-1);
			AllSeatSend(forkid,desk,buf,length);
		}

		if (seat == 0)
		{
			if (hall->user[user].status == 4)
			{
				//断线暂不处理
				hall->fork[forkid].zjh[desk].seat[seat].status = 1;
				hall->fork[forkid].zjh[desk].start_time = hall->time_now;

				if (hall->fork[forkid].zjh[desk].seat[1].status > 0)
				{
					//对面座位有人 通知状态改变
					length = pack3015(buf,forkid,desk,seat);
					UserSend(hall->fork[forkid].zjh[desk].seat[1].user,buf,length);	
				}
				sem_post(&(hall->fork[forkid].zjh[desk].lock));
				return 1;
			}
			//房主离开，解散房间
			if (hall->fork[forkid].zjh[desk].seat[1].user >= 0)
			{
				//有人
				length = packPublicSeat(buf,3004,1);
				AllSeatSend(forkid,desk,buf,length);
				__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].people),1);
				hall->user[hall->fork[forkid].zjh[desk].seat[1].user].hds = -1;
				hall->fork[forkid].zjh[desk].seat[1].user = -1;
				hall->fork[forkid].zjh[desk].seat[1].status = 0;
			}

			length = packPublicSeat(buf,3004,(short)seat);
			AllSeatSend(forkid,desk,buf,length);
			__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].people),1);
			hall->user[user].hds = -1;
			hall->fork[forkid].zjh[desk].seat[seat].status = 0;
			hall->fork[forkid].zjh[desk].seat[seat].user = -1;

			init_desk(forkid,desk,1);
			
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 0;
		}
		//printf("deskOut fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
		//离桌成功
		//离桌广播
		length = packPublicSeat(buf,3004,(short)seat);
		AllSeatSend(forkid,desk,buf,length);

		__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].people),1);
		hall->user[user].hds = -1;
		hall->fork[forkid].zjh[desk].seat[seat].status = 0;
		hall->fork[forkid].zjh[desk].seat[seat].user = -1;
		
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		return 0;
	}
	else if (hall->fork[forkid].gameid == 1)
	{
		//炸金花
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (i != seat && hall->fork[forkid].zjh[desk].seat[i].user >= 0 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 10)
			{
				
				if (hall->fork[forkid].zjh[desk].seat[seat].status > 2 && hall->fork[forkid].zjh[desk].seat[seat].status < 5)
				{
					//游戏中 暂不处理
					printf("游戏中，暂不处理\n");
					length = pack3015(buf,forkid,desk,seat);
					AllSeatSend(forkid,desk,buf,length);
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return 1;
				}
				else
				{
					//正常离桌
					length = packPublicSeat(buf,3004,(short)seat);
					AllSeatSend(forkid,desk,buf,length);
					
					__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].people),1);
					hall->user[user].hds = -1;
					hall->fork[forkid].zjh[desk].seat[seat].status = 0;
					hall->fork[forkid].zjh[desk].seat[seat].user = -1;
					
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					return 0;
				}
			}
		}

		//其余全部已断钱
		if (hall->fork[forkid].zjh[desk].flag != 0)
		{
			//游戏直接比牌结算
			zjhBalance(buf,forkid,desk);
		}

		//清人
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].user >= 0 && (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10)))
			{
				//此人已断线
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].hds = -1;
				cleanSession(hall->fork[forkid].zjh[desk].seat[i].user);
				hall->fork[forkid].zjh[desk].seat[i].user = -1;
				hall->fork[forkid].zjh[desk].seat[i].status = 0;
				
			}
		}
		
		length = packPublicSeat(buf,3004,(short)seat);
		UserSend(user,buf,length);
		//清桌
		hall->user[user].hds = -1;
		hall->fork[forkid].zjh[desk].seat[seat].user = -1;
		hall->fork[forkid].zjh[desk].seat[seat].status = 0;
		init_desk(forkid,desk,1);
		sem_post(&(hall->fork[forkid].zjh[desk].lock));
		return 0;
		
	}
	else
	{
		//牛牛及其它
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (i != seat && hall->fork[forkid].zjh[desk].seat[i].user >= 0 && hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 10)
			{
				
				if (hall->fork[forkid].zjh[desk].seat[seat].status > 2)
				{
					//游戏中 暂不处理
					if (hall->fork[forkid].gameid == 6)
					{
						//麻将
						length = pack3070(buf,forkid,desk,seat);
						AllSeatSend(forkid,desk,buf,length);
					}
					else
					{
						length = pack3015(buf,forkid,desk,seat);
						AllSeatSend(forkid,desk,buf,length);
					}
					
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					printf("NO deskOut fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
					return 1;
				}
				else
				{
					//正常离桌
					length = packPublicSeat(buf,3004,(short)seat);
					AllSeatSend(forkid,desk,buf,length);

					__sync_fetch_and_sub(&(hall->fork[forkid].zjh[desk].people),1);
					hall->user[user].hds = -1;
					hall->fork[forkid].zjh[desk].seat[seat].status = 0;
					hall->fork[forkid].zjh[desk].seat[seat].user = -1;
					
					sem_post(&(hall->fork[forkid].zjh[desk].lock));
					printf("YES deskOut fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
					return 0;
				}
			}
		}

		if (hall->fork[forkid].zjh[desk].flag == 100)
		{
			sem_post(&(hall->fork[forkid].zjh[desk].lock));
			return 1;
		}

		//清人
		for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
		{
			if (hall->fork[forkid].zjh[desk].seat[i].user >= 0 && (hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 4 || \
				(hall->user[hall->fork[forkid].zjh[desk].seat[i].user].status == 2 && hall->time_now - hall->user[hall->fork[forkid].zjh[desk].seat[i].user].time > 10)))
			{
				//此人已断线
				hall->user[hall->fork[forkid].zjh[desk].seat[i].user].hds = -1;
				cleanSession(hall->fork[forkid].zjh[desk].seat[i].user);
				hall->fork[forkid].zjh[desk].seat[i].user = -1;
				hall->fork[forkid].zjh[desk].seat[i].status = 0;
			}
		}
		length = packPublicSeat(buf,3004,(short)seat);
		UserSend(user,buf,length);

		printf("Clean deskOut fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
		//清桌
		hall->user[user].hds = -1;
		hall->fork[forkid].zjh[desk].seat[seat].user = -1;
		hall->fork[forkid].zjh[desk].seat[seat].status = 0;
		init_desk(forkid,desk,1);
		sem_post(&(hall->fork[forkid].zjh[desk].lock));

		return 0;
	}
	
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	return 0;
}

//桌内广播
void AllSeatSend(int forkid,int desk,char *buf,int length)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status > 0)
		{
			//广播
			UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
		}
	}
}

void OtherSeatSend(int forkid,int desk,int seat,char *buf,int length)
{
	int i;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (i != seat && hall->fork[forkid].zjh[desk].seat[i].status > 0)
		{
			//广播
			UserSend(hall->fork[forkid].zjh[desk].seat[i].user,buf,length);
		}
	}
}

void AllBjlSend(int forkid,char *buf,int length)
{
	if (hall->fork[forkid].thread <= 0)
	{
		return;
	}
	int i;
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].hds >= 0 && hall->user[i].hds/1000 == forkid)
		{
			UserSend(i,buf,length);
		}
	}
}

int UserSend(int user,char *buf,int length)
{
	if (hall->user[user].status == 10)
	{
		return App_Send(hall->user[user].socket,buf,length);
	}

	return 0;
}

int User_msg(int user,char *buf)
{
	int forkid,desk,seat,length;
	
	if (hall->user[user].hds < 0)
	{
		//已离桌
		return -1;
	}
	
	forkid = (hall->user[user].hds)/1000;
	desk = ((hall->user[user].hds)%1000)/10;
	seat = (hall->user[user].hds)%10;
	/*
	short int type;
	recv_int16_from(buf+17,&type);
	if (type == 5)
	{
		if (hall->user[user].money < MSG_FEE)
		{
			return 0;
		}
		
		__sync_fetch_and_sub(&(hall->user[user].money),MSG_FEE);
		//hall->user[user].msg_gold_update = 1;
	}
	*/
	length = pack3014(buf,buf+17,(short)seat);
	AllSeatSend(forkid,desk,buf,length);

	return 0;
}

int updateUserGold(int user,long long int hb_money,int hb_num,long long int send_money,int send_num)
{
	//printf("update user gold user=%d money=%lld,money_init=%lld,stock=%lld\n",user,hall->user[user].money,hall->user[user].money_init,hall->user[user].stock);
	if (user < 0 || user >= USER_MAX)
	{
		return 0;
	}
	int ret=-1;
	long long int money_new=-1,money_mix=0,cake_mix=0;
	//sem_wait(&(hall->user[user].lock_user));
	//同步金币
	while (1)
	{
		money_mix = hall->user[user].money - hall->user[user].money_init;
		cake_mix = hall->user[user].cake;
		ret = update_user_gold(hall->user[user].uid,money_mix,hall->user[user].stock,cake_mix,&money_new,hb_money,hb_num,send_money,send_num);
		if (ret == -1)
		{
			sleep(1);
		}
		else
		{
			break;
		}
	}
	
	if (ret == -2)
	{
		//更新成功，获取失败
		printf("更新成功，获取失败\n");
		//sem_wait(&(hall->user[user].lock_user));
		__sync_fetch_and_sub(&(hall->user[user].cake),cake_mix);
		__sync_fetch_and_add(&(hall->user[user].money_init),money_mix);
		//sem_post(&(hall->user[user].lock_user));
	}
	else 
	{
		//正常
		//printf("ok!\n");
		//sem_wait(&(hall->user[user].lock_user));
		__sync_fetch_and_sub(&(hall->user[user].cake),cake_mix);
		__sync_fetch_and_add(&(hall->user[user].money_init),money_mix);
		
		if (money_new != hall->user[user].money_init)
		{
			money_mix = money_new - hall->user[user].money_init;
			__sync_fetch_and_add(&(hall->user[user].money_init),money_mix);
			__sync_fetch_and_add(&(hall->user[user].money),money_mix);
		}
		//sem_post(&(hall->user[user].lock_user));
	}
	
	//sem_post(&(hall->user[user].lock_user));
	return 0;
}

void insertAAAMsg(int uid,char *name,short int gid,long long int gold_win,int check_type)
{
	//printf("insert aaa msg uid=%d,gold_win=%lld\n",uid,gold_win);
	if (uid < 0 || gold_win < AAAMSG_MIN)
	{
		return;
	}
	int i;

	sem_wait(&(hall->aaa_lock));
	//
	i = hall->site_msg;
	if (i-1 < 0)
	{
		hall->aaa_msg[i].id = hall->aaa_msg[MSG_LIST_MAX-1].id+1;
	}
	else
	{
		hall->aaa_msg[i].id = hall->aaa_msg[i-1].id+1;
	}
	hall->aaa_msg[i].length = pack3027(hall->aaa_msg[i].msg,uid,name,gid,(short)check_type,gold_win);
	//
	hall->site_msg++;
	if (hall->site_msg >= MSG_LIST_MAX)
	{
		hall->site_msg = 0;
	}
	sem_post(&(hall->aaa_lock));
}

int myRandom()
{
	int randomx;
	int randoms=open("/dev/urandom",O_RDONLY);
	read(randoms,&randomx,sizeof(int));
	close(randoms);
	if (randomx == 0)
	{
		randomx = rand();
	}
	randomx = abs(randomx);
	return randomx;
}

int ZJH_ready(char *buf,int user)
{
	int i,forkid,desk,seat,sum_ready,sum_uready,length;

	if ((hall->flag != 1 && hall->time_now > hall->time_over - CLOSE_TIME + 300) || hall->user[user].hds < 0)
	{
		//已离桌 或关服中
		length = packReturnFlag(buf,2005,-3);
		return UserSend(user,buf,length);
	}
	forkid = (hall->user[user].hds)/1000;
	desk = ((hall->user[user].hds)%1000)/10;
	seat = (hall->user[user].hds)%10;

	if (hall->fork[forkid].zjh[desk].flag != 0 || hall->fork[forkid].zjh[desk].seat[seat].status > 1)
	{
		//已准备或游戏已开始
		//printf("ZJH_ready game flag or status not ok\n");
		//length = packReturnFlag(buf,2005,-4);
		//return UserSend(user,buf,length);
		return 0;
	}
	
	if (hall->fork[forkid].gold_type == 1)
	{
		/*
		if (hall->user[user].gold < hall->fork[forkid].gold_min)
		{
			//金币低于房间要求
			length = packReturnFlag(buf,2005,-1);
			return UserSend(user,buf,length);
		}*/

		if (hall->user[user].gold < hall->fork[forkid].zjh[desk].bottom*10)
		{
			//钱不够底注10倍
			//printf("ZJH_ready gold < bottom,user.gold=%lld,bottom=%lld\n",hall->user[user].gold,hall->fork[forkid].zjh[desk].bottom);
			length = packReturnFlag(buf,2005,-2);
			return UserSend(user,buf,length);
		}
	}
	else
	{
		/*
		if (hall->user[user].money < hall->fork[forkid].gold_min)
		{
			//金币低于房间要求
			length = packReturnFlag(buf,2005,-1);
			return UserSend(user,buf,length);
		}*/

		if (hall->user[user].money < hall->fork[forkid].zjh[desk].bottom*10)
		{
			//钱不够底注10倍
			//printf("ZJH_ready gold < bottom,user.gold=%lld,bottom=%lld\n",hall->user[user].gold,hall->fork[forkid].zjh[desk].bottom);
			length = packReturnFlag(buf,2005,-2);
			return UserSend(user,buf,length);
		}
	}
	
	hall->fork[forkid].zjh[desk].seat[seat].status = 2;
	//未进入倒计时
	sum_ready = 0;
	sum_uready = 0;
	for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
	{
		if (hall->fork[forkid].zjh[desk].seat[i].status == 2)
		{
			sum_ready++;
		}
		else if (hall->fork[forkid].zjh[desk].seat[i].status > 0)
		{
			sum_uready++;
		}
	}
	//printf("fork%d desk%d game ready = %d,unready=%d\n",forkid,desk,sum_ready,sum_uready);
	if (hall->fork[forkid].gameid < 3)
	{
		//炸金花，牛牛
		if (sum_uready == 0)
		{
			//立即开始游戏
			//printf("fork%d Game start now!!!\n",forkid);
			hall->fork[forkid].zjh[desk].start_time = hall->time_now;
		}
		else if (sum_ready >= 2 && hall->fork[forkid].zjh[desk].start_time == 0)
		{
			//printf("fork%d Game start ... \n",forkid);
			//
			hall->fork[forkid].zjh[desk].start_time = hall->time_now+COUNTDOWN;
		}
	}
	else
	{
		//其它
		if (sum_ready == hall->fork[forkid].zjh[desk].seat_max)
		{
			//全部已准备 游戏开始
			hall->fork[forkid].zjh[desk].start_time = hall->time_now;
		}
	}
	
	//准备广播
	length = packPublicSeat(buf,3005,(short)seat);
	AllSeatSend(forkid,desk,buf,length);
	return 0;
}

int createZJHDesk(char *buf,int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int round)
{
	int hds=-1,i,j,k,length;
	//printf("uid %d money=%lld createDesk,gid=%d,type=%d,grade=%d,flag=%d,bottom=%lld,ip=%d,seat_max=%d,round=%d\n",hall->user[user].uid,hall->user[user].money,gid,type,grade,flag,bottom,ip,seat_max,round);
	if (bottom <= 0 || (round != 0 && round != 3) || hall->user[user].money < bottom*20 || grade != PRIVATE_HOUSE || (flag != 1 && flag != 2) || (ip != 0 && ip != 1) || seat_max < 2 || seat_max >= SEAT_MAX)
	{
		//printf("参数错误!\n");
		length = pack2006(buf,gid,type,grade,-2);
		UserSend(user,buf,length);
		return 0;
	}
	
	if (hall->user[user].hds >= 0)
	{
		length = pack2006(buf,gid,type,grade,-3);
		UserSend(user,buf,length);
		return 0;
	}

	for (i=0;i<hall->normal_num;i++)
	{
		if (hall->fork[i].gameid == gid && hall->fork[i].type == type && hall->fork[i].grade == grade)
		{
			if (bottom > BOTTOM_PRIVATE)
			{
				length = pack2006(buf,gid,type,grade,-2);
				UserSend(user,buf,length);
				return -1;
			}
			k = rand()%DESK_MAX;
			for (j=0;j<DESK_MAX;j++)
			{
				if (hall->fork[i].zjh[k].desk_status != 0)
				{
					k++;
					if (k >= DESK_MAX)
					{
						k = 0;
					}
					continue;
				}
				sem_wait(&(hall->fork[i].zjh[k].lock));
				if (hall->fork[i].zjh[k].desk_status == 0)
				{
					hall->fork[i].zjh[k].people = 1;
					hall->fork[i].zjh[k].desk_status = flag;
					hall->fork[i].zjh[k].ip = ip;
					hall->fork[i].zjh[k].seat_max = seat_max;
					hall->fork[i].zjh[k].bottom = bottom;
					hall->fork[i].zjh[k].round_max = round;
					hall->fork[i].zjh[k].round = 1;
					hall->fork[i].zjh[k].order_max = bottom;
					hall->fork[i].zjh[k].in_total = 0;
					hall->fork[i].zjh[k].seat[0].status = 1;
					hall->fork[i].zjh[k].seat[0].user = user;
					hds = i*1000+k*10;
					hall->user[user].hds = hds;
					sem_post(&(hall->fork[i].zjh[k].lock));
					length = pack2006(buf,gid,type,grade,createHDS(hds));
					return UserSend(user,buf,length);
				}
				sem_post(&(hall->fork[i].zjh[k].lock));
				k++;
				if (k >= DESK_MAX)
				{
					k = 0;
				}
			}
		}
	}
	//
	length = pack2006(buf,gid,type,grade,hds);
	return UserSend(user,buf,length);
}

int createWZQDesk(char *buf,int user,short int gid,short int type,short int grade)
{
	int hds=-1,i,j,k,length;
	//printf("uid %d createWZQDesk,gid=%d,type=%d,grade=%d\n",hall->user[user].uid,gid,type,grade);
	
	if (hall->user[user].hds >= 0)
	{
		length = pack2006(buf,gid,type,grade,-3);
		UserSend(user,buf,length);
		return 0;
	}

	for (i=0;i<hall->normal_num;i++)
	{
		if (hall->fork[i].gameid == gid && hall->fork[i].type == type && hall->fork[i].grade == grade)
		{
			k = rand()%DESK_MAX;
			for (j=0;j<DESK_MAX;j++)
			{
				if (hall->fork[i].zjh[k].desk_status != 0)
				{
					k++;
					if (k >= DESK_MAX)
					{
						k = 0;
					}
					continue;
				}
				sem_wait(&(hall->fork[i].zjh[k].lock));
				if (hall->fork[i].zjh[k].desk_status == 0)
				{
					hall->fork[i].zjh[k].people = 1;
					hall->fork[i].zjh[k].desk_status = 2;
					hall->fork[i].zjh[k].seat_max = 2;
					hall->fork[i].zjh[k].bottom = 0;
					hall->fork[i].zjh[k].in_total = 0;
					hall->fork[i].zjh[k].seat[0].status = 1;
					hall->fork[i].zjh[k].seat[0].user = user;
					hds = i*1000+k*10;
					hall->user[user].hds = hds;
					sem_post(&(hall->fork[i].zjh[k].lock));
					length = pack2006(buf,gid,type,grade,createHDS(hds));
					return UserSend(user,buf,length);
				}
				sem_post(&(hall->fork[i].zjh[k].lock));

				k++;
				if (k >= DESK_MAX)
				{
					k = 0;
				}
			}
		}
	}
	//
	length = pack2006(buf,gid,type,grade,hds);
	return UserSend(user,buf,length);
}

int createOXDesk(char *buf,int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int model,short int kan)
{
	int hds=-1,i,j,k,length;
	//printf("uid %d money=%lld createDesk,gid=%d,type=%d,grade=%d,flag=%d,bottom=%lld,ip=%d,seat_max=%d,model=%d,kan=%d\n",hall->user[user].uid,hall->user[user].money,gid,type,grade,flag,bottom,ip,seat_max,model,kan);
	if (bottom <= 0 || hall->user[user].money < bottom*20 || grade != PRIVATE_HOUSE || (flag != 1 && flag != 2) || (ip != 0 && ip != 1) || seat_max < 2 || seat_max >= SEAT_MAX || (model != 0 && model != 1) || (kan != 0 && kan != 1))
	{
		length = pack2006(buf,gid,type,grade,-2);
		UserSend(user,buf,length);
		return 0;
	}
	
	if (hall->user[user].hds >= 0)
	{
		length = pack2006(buf,gid,type,grade,-3);
		UserSend(user,buf,length);
		return 0;
	}

	for (i=0;i<hall->normal_num;i++)
	{
		if (hall->fork[i].gameid == gid && hall->fork[i].type == type && hall->fork[i].grade == grade)
		{
			if (bottom > BOTTOM_PRIVATE)
			{
				length = pack2006(buf,gid,type,grade,-2);
				UserSend(user,buf,length);
				return -1;
			}
			k = rand()%DESK_MAX;
			for (j=0;j<DESK_MAX;j++)
			{
				if (hall->fork[i].zjh[k].desk_status != 0)
				{
					k++;
					if (k >= DESK_MAX)
					{
						k = 0;
					}
					continue;
				}
				sem_wait(&(hall->fork[i].zjh[k].lock));
				if (hall->fork[i].zjh[k].desk_status == 0)
				{
					hall->fork[i].zjh[k].people = 1;
					hall->fork[i].zjh[k].desk_status = flag;
					hall->fork[i].zjh[k].ip = ip;
					hall->fork[i].zjh[k].seat_max = seat_max;
					hall->fork[i].zjh[k].bottom = bottom;
					hall->fork[i].zjh[k].model = model;
					hall->fork[i].zjh[k].kan = kan;
					hall->fork[i].zjh[k].seat[0].status = 1;
					hall->fork[i].zjh[k].seat[0].user = user;
					hds = i*1000+k*10;
					hall->user[user].hds = hds;
					sem_post(&(hall->fork[i].zjh[k].lock));
					length = pack2006(buf,gid,type,grade,createHDS(hds));
					return UserSend(user,buf,length);
				}
				sem_post(&(hall->fork[i].zjh[k].lock));
				k++;
				if (k >= DESK_MAX)
				{
					k = 0;
				}
			}
		}
	}
	//
	length = pack2006(buf,gid,type,grade,hds);
	return UserSend(user,buf,length);
}

int createMJDesk(char *buf,int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int mj_change,short int mj_zm,short int mj_times,short int mj_dg,short int mj_yjjd,short int mj_mqzz,short int mj_td)
{
	int hds=-1,i,j,k,length;
	//printf("uid %d money=%lld createDesk,gid=%d,type=%d,grade=%d,flag=%d,bottom=%lld,ip=%d,seat_max=%d,round=%d\n",hall->user[user].uid,hall->user[user].money,gid,type,grade,flag,bottom,ip,seat_max,round);
	if (bottom <= 0 || hall->user[user].money < bottom*20 || grade != PRIVATE_HOUSE || (flag != 1 && flag != 2) || seat_max < 2 || seat_max >= SEAT_MAX || mj_times < 2 || mj_times > 12)
	{
		//printf("参数错误!\n");
		length = pack2006(buf,gid,type,grade,-2);
		UserSend(user,buf,length);
		return 0;
	}
	
	if (hall->user[user].hds >= 0)
	{
		length = pack2006(buf,gid,type,grade,-3);
		UserSend(user,buf,length);
		return 0;
	}

	for (i=0;i<hall->normal_num;i++)
	{
		if (hall->fork[i].gameid == gid && hall->fork[i].type == type && hall->fork[i].grade == grade)
		{
			if (bottom > BOTTOM_PRIVATE)
			{
				length = pack2006(buf,gid,type,grade,-2);
				UserSend(user,buf,length);
				return -1;
			}
			k = rand()%DESK_MAX;
			for (j=0;j<DESK_MAX;j++)
			{
				if (hall->fork[i].zjh[k].desk_status != 0)
				{
					k++;
					if (k >= DESK_MAX)
					{
						k = 0;
					}
					continue;
				}
				sem_wait(&(hall->fork[i].zjh[k].lock));
				if (hall->fork[i].zjh[k].desk_status == 0)
				{
					hall->fork[i].zjh[k].people = 1;
					hall->fork[i].zjh[k].desk_status = flag;
					hall->fork[i].zjh[k].ip = ip;
					hall->fork[i].zjh[k].seat_max = seat_max;
					hall->fork[i].zjh[k].bottom = bottom;
					hall->fork[i].zjh[k].banker = -1;

					hall->fork[i].zjh[k].mj_change = mj_change;
					hall->fork[i].zjh[k].mj_zm = mj_zm;
					hall->fork[i].zjh[k].mj_times = mj_times;
					hall->fork[i].zjh[k].mj_dg = mj_dg;
					hall->fork[i].zjh[k].mj_yjjd = mj_yjjd;
					hall->fork[i].zjh[k].mj_mqzz = mj_mqzz;
					hall->fork[i].zjh[k].mj_td = mj_td;

					hall->fork[i].zjh[k].seat[0].status = 1;
					hall->fork[i].zjh[k].seat[0].user = user;
					hds = i*1000+k*10;
					hall->user[user].hds = hds;
					sem_post(&(hall->fork[i].zjh[k].lock));
					length = pack2006(buf,gid,type,grade,createHDS(hds));
					return UserSend(user,buf,length);
				}
				sem_post(&(hall->fork[i].zjh[k].lock));
				k++;
				if (k >= DESK_MAX)
				{
					k = 0;
				}
			}
		}
	}
	//
	length = pack2006(buf,gid,type,grade,hds);
	return UserSend(user,buf,length);
}

int ZJH_inDesk(int user,int forkid,char *buf,int hds)
{
	int uid,i,j,desk=-1,seat=-1,length;
	short int flag,wdesk=-1;
	char mytoken[36];
	snprintf(mytoken,33,"%s",buf+17);
	//printf("ZJH_inDesk uid=%d user.hds=%d hds=%d token=[%s]\n",hall->user[user].uid,hall->user[user].hds,hds,mytoken);
	if (hall->user[user].hds < 0)
	{
		//初次入桌
		uid = getUidByToken(mytoken);
		if (uid < 0 || uid != hall->user[user].uid)
		{
			//验证失败
			length = pack2003(buf,-1,forkid,desk,(short)seat);
			UserSend(user,buf,length);
			return 0;
		}
		if (hall->fork[forkid].grade == PRIVATE_HOUSE)
		{
			//私人场
			if (hds < 0)
			{
				length = pack2003(buf,-1,forkid,desk,(short)seat);
				UserSend(user,buf,length);
				return 0;
			}
			hds = getHDS(hds);
			if (hds/1000 != forkid)
			{
				length = pack2003(buf,-1,forkid,desk,(short)seat);
				UserSend(user,buf,length);
				return 0;
			}
			
			wdesk = (hds%1000)/10;
			//printf("入桌 start uid=%d hds=%d wdesk=%d\n",hall->user[user].uid,hds,wdesk);
			if (hall->fork[forkid].grade == PRIVATE_HOUSE && (wdesk < 0 || wdesk > DESK_MAX || hds%10 != 0))
			{
				//桌号错误
				length = pack2003(buf,-1,forkid,desk,(short)seat);
				UserSend(user,buf,length);
				return 0;
			}
			
			if (hall->fork[forkid].zjh[wdesk].desk_status == 0)
			{
				//桌子未启用
				length = pack2003(buf,-2,forkid,desk,(short)seat);
				UserSend(user,buf,length);
				return 0;
			}

			if (hall->user[user].money < hall->fork[forkid].zjh[wdesk].bottom*20)
			{
				//金币小于底注
				length = pack2003(buf,-5,forkid,desk,(short)seat);
				UserSend(user,buf,length);
				return 0;
			}
			//自定义桌子
			if (hall->fork[forkid].zjh[wdesk].desk_status != 0 && hall->fork[forkid].zjh[wdesk].people < hall->fork[forkid].zjh[wdesk].seat_max)
			{
				sem_wait(&(hall->fork[forkid].zjh[wdesk].lock));
				if (hall->fork[forkid].zjh[wdesk].desk_status != 0 && hall->fork[forkid].zjh[wdesk].flag != 100)
				{
					for (i=0;i<hall->fork[forkid].zjh[wdesk].seat_max;i++)
					{
						if (hall->fork[forkid].zjh[wdesk].seat[i].status == 0)
						{
							__sync_fetch_and_add(&(hall->fork[forkid].zjh[wdesk].people),1);
							hall->fork[forkid].zjh[wdesk].seat[i].status = 1;
							hall->fork[forkid].zjh[wdesk].seat[i].user = user;
							
							hall->user[user].hds = forkid*1000+wdesk*10+i;
							break;
						}
					}
				}
				sem_post(&(hall->fork[forkid].zjh[wdesk].lock));
			}
		}
		else
		{
			//顺序入桌
			flag = 0;
			for (i=0;i<DESK_MAX;i++)
			{
				if (hall->fork[forkid].zjh[i].people == hall->fork[forkid].zjh[i].seat_max)
				{
					continue;
				}
				sem_wait(&(hall->fork[forkid].zjh[i].lock));
				if (hall->fork[forkid].zjh[i].people < hall->fork[forkid].zjh[i].seat_max && hall->fork[forkid].zjh[i].flag != 100)
				{
					j = hall->fork[forkid].zjh[i].seat_max;
					//同IP不能进入
					/*for (j=0;j<hall->fork[forkid].zjh[i].seat_max;j++)
					{
						if (hall->fork[forkid].zjh[i].seat[j].status != 0)
						{
							//有人 检查IP
							if (strcmp(hall->user[hall->fork[forkid].zjh[i].seat[j].user].ip,hall->user[user].ip) == 0)
							{
								//IP相同
								break;
							}
						}
					}*/
					//
					if (j == hall->fork[forkid].zjh[i].seat_max)
					{
						for (j=0;j<hall->fork[forkid].zjh[i].seat_max;j++)
						{
							if (hall->fork[forkid].zjh[i].seat[j].status == 0)
							{
								__sync_fetch_and_add(&(hall->fork[forkid].zjh[i].people),1);
								hall->fork[forkid].zjh[i].seat[j].status = 1;
								hall->fork[forkid].zjh[i].seat[j].user = user;
								hall->user[user].hds = forkid*1000+i*10+j;
								flag = 1;
								break;
							}
						}
					}
					
				}
				sem_post(&(hall->fork[forkid].zjh[i].lock));

				if (flag == 1)
				{
					break;
				}
			}
		}
		//选座完成
		
		if (hall->user[user].hds < 0)
		{
			//满座
			length = pack2003(buf,0,forkid,desk,(short)seat);
			UserSend(user,buf,length);
			return 0;
		}
	//
	}
	
	desk = ((hall->user[user].hds)%1000)/10;
	seat = (hall->user[user].hds)%10;
	if (forkid != hall->user[user].hds/1000 || seat >= hall->fork[forkid].zjh[desk].seat_max)
	{
		return -1;
	}
	printf("入桌 OK ! fork%d desk%d seat%d uid=%d\n",forkid,desk,seat,hall->user[user].uid);
	//入桌应答
	length = pack2003(buf,1,forkid,desk,(short)seat);
	UserSend(user,buf,length);
	
	if (hall->fork[forkid].gameid == 6)
	{
		length = pack3070(buf,forkid,desk,seat);
		AllSeatSend(forkid,desk,buf,length);
	}
	else
	{
		length = pack3015(buf,forkid,desk,seat);
		AllSeatSend(forkid,desk,buf,length);
	}
	
	return 0;
}

int gameInfo(char *buf,int user)
{
	int i,length,ret = 0,forkid,desk,seat;

	if (hall->user[user].hds < 0)
	{
		//已离桌
		return 0;
	}

	forkid = (hall->user[user].hds)/1000;
	desk = ((hall->user[user].hds)%1000)/10;
	seat = (hall->user[user].hds)%10;
	
	sem_wait(&(hall->fork[forkid].zjh[desk].lock));
	if (hall->fork[forkid].gameid == 6)
	{
		//全部座位信息
		length = pack3070(buf,forkid,desk,-1);
		if (UserSend(user,buf,length) < 0)
		{
			ret = -1;
		}
		//看牌
		length = packLookCards(buf,forkid,desk,seat,-1);
		if (UserSend(user,buf,length) < 0)
		{
			ret = -1;
		}
		//游戏信息
		length = pack4070(buf,forkid,desk);
		if (UserSend(user,buf,length) < 0)
		{
			ret = -1;
		}
	}
	else
	{
		//全部座位信息
		length = pack3015(buf,forkid,desk,-1);
		if (UserSend(user,buf,length) < 0)
		{
			ret = -1;
		}

		//游戏信息
		length = pack4015(buf,forkid,desk);
		if (UserSend(user,buf,length) < 0)
		{
			ret = -1;
		}
	}
	
	//此座位信息广播
	if (hall->fork[forkid].gameid == 1)
	{
		//炸金花
		length = pack4006(buf,forkid,desk,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now,0);
		UserSend(user,buf,length);

		if (hall->fork[forkid].zjh[desk].seat[seat].status > 3)
		{
			length = packLookCards(buf,forkid,desk,seat,3);
			UserSend(user,buf,length);
		}
	}
	else if (hall->fork[forkid].gameid == 2)
	{
		switch (hall->fork[forkid].zjh[desk].flag)
		{
			case 1:
			case 2:
				if (hall->fork[forkid].type == 1)
				{
					length = packLookCards(buf,forkid,desk,seat,4);
					UserSend(user,buf,length);
				}
				break;
			case 3:
				//牛牛游戏结算中
				length = packLookCards(buf,forkid,desk,seat,5);
				UserSend(user,buf,length);
				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].status == 6)
					{
						length = pack3040(buf,forkid,desk,i);
						if (UserSend(user,buf,length) < 0)
						{
							return -1;
						}
					}
				}
				break;
			default:
				break;
		}
	}
	else if (hall->fork[forkid].gameid == 6)
	{
		//麻将
		switch (hall->fork[forkid].zjh[desk].flag)
		{
			case 1:

				if (hall->fork[forkid].zjh[desk].seat[seat].card_change[0] > 0)
				{
					//换三张
					length = pack2073(buf,1,hall->fork[forkid].zjh[desk].seat[seat].card_change,3);
					UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
				}

				for (i=0;i<hall->fork[forkid].zjh[desk].seat_max;i++)
				{
					if (hall->fork[forkid].zjh[desk].seat[i].card_change[0] > 0)
					{
						length = pack3073(buf,(short)i);
						UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
					}
				}
				break;
			case 3:

				if (hall->fork[forkid].zjh[desk].seat[seat].lack != 0)
				{
					length = pack2074(buf,1,hall->fork[forkid].zjh[desk].seat[seat].lack);
					UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
				}
				break;
			case 4:
				//游戏中
				length = pack3079(buf,hall->fork[forkid].zjh[desk].turn,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].begin_time - hall->time_now);
				UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);

				if (seat != hall->fork[forkid].zjh[desk].turn)
				{
					//没有轮到自己
					length = pack2078(buf,hall->fork[forkid].zjh[desk].turn,hall->fork[forkid].zjh[desk].card_num,0);
					UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);

					if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
					{
						//有操作
						length = pack2075(buf,forkid,desk,seat,1,hall->fork[forkid].zjh[desk].seat[hall->fork[forkid].zjh[desk].turn].card_out);
						UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
					}

				}
				else
				{
					//轮到自己
					//摸牌返回
					if (hall->fork[forkid].zjh[desk].seat[seat].card_in > 0)
					{
						length = pack2078(buf,(short int)seat,hall->fork[forkid].zjh[desk].card_num,hall->fork[forkid].zjh[desk].seat[seat].card_in);
						UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
					}
					

					if (hall->fork[forkid].zjh[desk].seat[seat].flag_peng == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_gang == 1 || hall->fork[forkid].zjh[desk].seat[seat].flag_win == 1)
					{
						//有操作
						length = pack2075(buf,forkid,desk,seat,0,hall->fork[forkid].zjh[desk].seat[seat].card_in);
						UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
					}

				}
				break;
			case 9:
				if (hall->fork[forkid].zjh[desk].seat[seat].ranking == 4)
				{
					length = pack3082(buf,seat);
					UserSend(hall->fork[forkid].zjh[desk].seat[seat].user,buf,length);
				}
				break;
			default:
				break;
		}
	}
	//printf("int desk reply ret = %d\n",ret);
	sem_post(&(hall->fork[forkid].zjh[desk].lock));
	return ret;
}

int User_stock(char *buf,int user,short int type,long long int gold)
{
	int i,length,forkid,desk,seat;

	if (hall->flag != 1 && hall->time_now > hall->time_over - CLOSE_TIME + 300)
	{
		length = pack2060(buf,type,-1,gold);
		return UserSend(user,buf,length);
	}
//	printf("stock.gold = %lld\n",gold);
	switch (type)
	{
		case 0:
			//存款
			//sem_wait(&(hall->user[user].lock_user));
			if (hall->user[user].money < gold || gold <= 0 || hall->user[user].hds >= 0)
			{
				//sem_post(&(hall->user[user].lock_user));
				length = pack2060(buf,type,0,gold);
				return UserSend(user,buf,length);
			}
			__sync_fetch_and_sub(&(hall->user[user].money),gold);
			__sync_fetch_and_add(&(hall->user[user].stock),gold);
			hall->user[user].msg_gold_update = 1;
			//sem_post(&(hall->user[user].lock_user));
			break;
		case 1:
			//取款
			//sem_wait(&(hall->user[user].lock_user));
			if (hall->user[user].stock < gold || gold <= 0)
			{
				//sem_post(&(hall->user[user].lock_user));
				length = pack2060(buf,type,0,gold);
				return UserSend(user,buf,length);
			}

			if (hall->user[user].hds >= 0)
			{
				forkid = hall->user[user].hds/1000;
				desk = ((hall->user[user].hds)%1000)/10;
				seat = hall->user[user].hds%10;

				if (hall->fork[forkid].gameid == 2 && hall->fork[forkid].zjh[desk].seat[seat].status > 1)
				{
					//sem_post(&(hall->user[user].lock_user));
					length = pack2060(buf,type,0,gold);
					return UserSend(user,buf,length);
				}
				else if (hall->fork[forkid].gameid == 6 && hall->user[user].money != 0 && hall->fork[forkid].zjh[desk].seat[seat].status > 1)
				{
					//sem_post(&(hall->user[user].lock_user));
					length = pack2060(buf,type,0,gold);
					return UserSend(user,buf,length);
				}
			}
			__sync_fetch_and_add(&(hall->user[user].money),gold);
			__sync_fetch_and_sub(&(hall->user[user].stock),gold);
			//sem_post(&(hall->user[user].lock_user));
			if (hall->user[user].hds >= 0 && hall->fork[hall->user[user].hds/1000].gameid == 0)
			{
				//3020
				if (hall->user[user].hds%10 < BJL_SEAT_MAX || hall->user[user].banker_flag[0] == 10)
				{
					for (i=0;i<BJL_HOUSE_MAX;i++)
					{
						hall->bjl[0].flag_seat[i] = 1;
					}
				}
				
			}
			else if (hall->user[user].hds >= 0 && hall->fork[hall->user[user].hds/1000].gameid == 4)
			{
				//3020
				if (hall->user[user].hds%10 < BJL_SEAT_MAX || hall->user[user].banker_flag[1] == 10)
				{
					for (i=0;i<BJL_HOUSE_MAX;i++)
					{
						hall->bjl[1].flag_seat[i] = 1;
					}
				}
			}
			else if (hall->user[user].hds >= 0 && hall->fork[hall->user[user].hds/1000].gameid == 10)
			{
				//3020
				if (hall->user[user].hds%10 < BJL_SEAT_MAX || hall->user[user].banker_flag[2] == 10)
				{
					for (i=0;i<BJL_HOUSE_MAX;i++)
					{
						hall->bjl[2].flag_seat[i] = 1;
					}
				}
			}
			else
			{
				//3016
				hall->user[user].msg_gold_update = 1;
			}
			break;
		default:
			return 0;
	}
	updateUserGold(user,0,0,0,0);
	length = pack2060(buf,type,1,gold);
	return UserSend(user,buf,length);
}

int User_order(char *buf,int user,long long int order)
{
	//printf("user_order = %lld\n",order);
	if (order <= 0)
	{
		return 0;
	}

	int i,length;
	sem_wait(&(hall->user[user].lock_order));
	
	if (hall->user[user].order_num > 1)
	{	
		for (i=hall->user[user].order_num-1;i>0;i--)
		{
		hall->user[user].order[i].order = hall->user[user].order[i-1].order;
		hall->user[user].order[i].create_time = hall->user[user].order[i-1].create_time;
		hall->user[user].order[i].update_time = hall->user[user].order[i-1].update_time;
		}
	}

	hall->user[user].order[0].order = order;
	hall->user[user].order[0].create_time = hall->time_now;
	hall->user[user].order[0].update_time = hall->user[user].order[0].create_time;
	hall->user[user].order_num++;
	if (hall->user[user].order_num > ORDER_MAX)
	{
		hall->user[user].order_num = ORDER_MAX;
	}

	sem_post(&(hall->user[user].lock_order));

	length = packReturnFlag(buf,2030,1);
	return UserSend(user,buf,length);
}

int getUserGoldFromRedis(char *buf,int user,int type)
{
	int length;
	long long int gold,money,stock;
	//printf("get User Gold From Redis type = %d\n",type);
	if (type==1 && get_user_gold_name(hall->user[user].uid,&money,&stock,hall->user[user].name,hall->user[user].url,&(hall->user[user].sex),hall->user[user].passwd) >= 0)
	{
		//printf("gold_init=%lld,gold=%lld\n",hall->user[user].gold_init,gold);
		//sem_wait(&(hall->user[user].lock_user));
		if (money != hall->user[user].money_init)
		{
			gold = money - hall->user[user].money_init;
			__sync_fetch_and_add(&(hall->user[user].money_init),gold);
			__sync_fetch_and_add(&(hall->user[user].money),gold);
		}
		//sem_post(&(hall->user[user].lock_user));
	}
	//金币更新通知
	length = pack3016(buf,user);
	
	if (hall->user[user].hds >= 0)
	{
		if (hall->fork[hall->user[user].hds/1000].grade != 100)
		{
			AllSeatSend(hall->user[user].hds/1000,(hall->user[user].hds%1000)/10,buf,length);
			return 0;
		}
	}
	
	return UserSend(user,buf,length);
}

void User_order_check(char *buf,int user)
{
	//printf("User_order_check order_num=%d\n",hall->user[user].order_num);
	if (hall->user[user].order_num <= 0)
	{
		return;
	}
	int i,j=0,flag,c_time,length;
	
	sem_wait(&(hall->user[user].lock_order));
	for (i=0;i<ORDER_MAX;i++)
	{
		if (hall->user[user].order[i].order > 0)
		{
			//有订单
			j++;
			flag = -2;
			c_time = hall->time_now - hall->user[user].order[i].create_time;
			if (c_time > ORDER_TIME)
			{
				//超时 清理
			}
			else if (c_time < 60)
			{
				//每秒更新
				if (hall->time_now - hall->user[user].order[i].update_time > 1)
				{
					hall->user[user].order[i].update_time = hall->time_now;
					flag = getStatusByOrder(hall->user[user].order[i].order);
				}
			}
			else if (c_time < 120)
			{
				//每5秒更新
				if (hall->time_now - hall->user[user].order[i].update_time > 5)
				{
					hall->user[user].order[i].update_time = hall->time_now;
					flag = getStatusByOrder(hall->user[user].order[i].order);
				}
			}
			else
			{
				//每25秒更新
				if (hall->time_now - hall->user[user].order[i].update_time > 25)
				{
					hall->user[user].order[i].update_time = hall->time_now;
					flag = getStatusByOrder(hall->user[user].order[i].order);
				}
			}
			
			switch (flag)
			{
				case 1:
					//已创建
					break;
				case 2:
					//充值成功
					getUserGoldFromRedis(buf,user,1);
					length = pack3030(buf);
					UserSend(user,buf,length);
					hall->user[user].order[i].order = 0;
					hall->user[user].order_num--;
					break;
				case 0:
					//未找到
					hall->user[user].order[i].order = 0;
					hall->user[user].order_num--;
					break;
				case -1:
					//redis error
					break;
				default:
					//未定义 跳过
					break;
			}
		}
		if (j >= hall->user[user].order_num)
		{
			break;
		}
	}
	//printf("########User_order_check order_num=%d\n",hall->user[user].order_num);
	sem_post(&(hall->user[user].lock_order));
}

int User_exchange(char *buf,int user,short int type,long long int gold,char *passwd)
{
	int length;

	if (hall->flag != 1 && hall->time_now > hall->time_over - CLOSE_TIME + 300)
	{
		//服务器停服中
		length = pack2083(buf,-1,gold);
		return UserSend(user,buf,length);
	}
	/*
	if (hall->user[user].hds >= 0)
	{
		//必须回大厅兑换
		length = pack2083(buf,-2,gold);
		return UserSend(user,buf,length);
	}*/

	if (gold <= 0)
	{
		length = pack2083(buf,-3,gold);
		return UserSend(user,buf,length);
	}

	if (strcmp(passwd,hall->user[user].passwd) != 0)
	{
		//密码错误
		length = pack2083(buf,-4,gold);
		return UserSend(user,buf,length);
	}

	//sem_wait(&(hall->user[user].lock_user));
	
	if (hall->user[user].money < gold)
	{
		//余额为足
		//sem_post(&(hall->user[user].lock_user));
		length = pack2083(buf,-3,gold);
		return UserSend(user,buf,length);
	}
	
	__sync_fetch_and_sub(&(hall->user[user].money),gold);
	while (add_user_exchange(hall->user[user].uid,type,hall->user[user].name,gold,hall->time_now) < 0)
	{
		sleep(1);
	}

	//sem_post(&(hall->user[user].lock_user));

	updateUserGold(user,0,0,0,0);
	length = pack2083(buf,1,gold);
	return UserSend(user,buf,length);
}

int User_chat(char *buf,int user,int uid,short int msg_id,short int msg_type,short int msg_len,char *msg)
{
	int i,j,length;
	if (msg_len < 0 || msg_len > 2000 || uid < 0 || hall->user[user].uid == uid)
	{
		return -1;
	}
	
	for (i=0;i<USER_MAX;i++)
	{
		if (hall->user[i].uid == uid)
		{
			//找到在线	
			if (hall->user[i].user_msg_num >= MSG_LIST_MAX)
			{
				length = pack2090(buf,-1,msg_id);
				return UserSend(user,buf,length);
			}

			sem_wait(&(hall->user[i].lock_user_msg));
			if (hall->user[i].user_msg_num >= MSG_LIST_MAX)
			{
				sem_post(&(hall->user[i].lock_user_msg));
				length = pack2090(buf,-1,msg_id);
				return UserSend(user,buf,length);
			}
			
			for (j=0;j<MSG_LIST_MAX;j++)
			{
				if (hall->user[i].user_msg[j].length == 0)
				{
					//可以发送
					hall->user[i].user_msg[j].length = pack3090(hall->user[i].user_msg[j].msg,hall->user[user].uid,msg_type,msg_len,msg);
					hall->user[i].user_msg_num++;
					sem_post(&(hall->user[i].lock_user_msg));
					length = pack2090(buf,1,msg_id);
					return UserSend(user,buf,length);
				}
			}

			sem_post(&(hall->user[i].lock_user_msg));
			length = pack2090(buf,-1,msg_id);
			return UserSend(user,buf,length);
		}
	}
	
	length = pack2090(buf,0,msg_id);
	return UserSend(user,buf,length);
}

int User_trade(char *buf,int user,int uid,long long int money,char *info,char *passwd)
{
	int length,tid=-1;

	if (money <= 0)
	{
		return -1;
	}

	if (strcmp(passwd,hall->user[user].passwd) != 0)
	{
		//密码错误
		length = pack2093(buf,-1,hall->user[user].money,tid);
		return UserSend(user,buf,length);
	}

	if (hall->user[user].money < money)
	{
		//钱不足
		length = pack2093(buf,-2,hall->user[user].money,tid);
		return UserSend(user,buf,length);
	}

	__sync_fetch_and_sub(&(hall->user[user].money),money);
	updateUserGold(user,0,0,0,0);

	while ((tid = add_user_transfer(hall->user[user].uid,uid,money,info,(int)hall->time_now)) < 0)
	{
		sleep(1);
	}

	length = pack2093(buf,1,hall->user[user].money,tid);
	return UserSend(user,buf,length);
}