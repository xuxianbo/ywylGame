#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define  CONNECT_IP_MAX	10				//每个IP连接数上限
#define  CONNECT_CHECK "5d9195f260a4e8dd2e903770bfe128ec"
#define  SESSION_KEY 578913
#define  TIME_OUT 150					//TIME_OUT*0.2秒
#define  HALL_TIME_OUT 60				//大厅超时上限HALL_TIME_OUT*0.2秒	
#define  AAAMSG_MIN	2000				//大奖通知下限
#define  PRIVATE_HOUSE 99				//私人场等级
#define	 MSG_FEE 0						//互动表情收费
#define  ORDER_TIME	300					//订单有效时长
#define  BOTTOM_PRIVATE	2000			//私人场最大底注
#define  BUF_MAX	8192				//buf最大内存

//炸金花
#define  COUNTDOWN 5				//倒计时秒
#define  TIME_ACTION 60				//操作时间上限

//百家乐拼三张
#define TIME_WAIT	5			//游戏等待时间
#define TIME_BET	10			//游戏下注时间
#define TIME_OVER	15			//游戏结算+结果展示时间
#define USER_BET_MAX 100000		//用户单局下注最大值
#define UP_BANKER	11000		//上庄最小值
#define DOWN_BANKER 5000		//强制下庄

#define GOLD_BANKER 11000		//系统庄家上庄金币数量	
#define BJL_CARD_MAX 52			//一副牌数量
#define FORK_BANKER	9			//庄家房间号

//推筒子
#define TTZ_CARD_MAX 40			//推筒子牌数量
#define TTZ_BET_MAX 3			//投注区域数

//牛牛
#define  OX_CARD_MAX 52			//牛牛一副牌数量
#define  OX_BANKER_TIME	  10	//牛牛抢庄时间
#define  OX_BET_TIME	  10	//牛牛上注时间
#define  OX_COMPUTE_TIME  9		//牛牛计算时间

//五子棋
#define  WZQ_BET_MAX	9999999	//五子棋学费上限
#define  WZQ_TIME_OUT   600		//五子棋超时上限

//麻将
#define  MJ_LACK_TIME 10		//定缺换牌时间
#define  MJ_ACTION_TIME 15		//操作时间
//#define  MJ_TIMES_MAX 3			//满番
#define  MJ_FREEZE_TIME 45		//冻结时间

#define LOG_MYSQL "/root/ywyl_sql.log"
#define LOG_PATH "/root/ywyl_app.log"
#define LOGIN_PATH "/root/ywyl_login.log"
#define WEB_ROOT "/home/wwwroot/api.88008088.com/public/resource/admin/user.online"
#define WEB_ROOT_BJL "/home/wwwroot/api.88008088.com/public/resource/admin/bjl.user"
#define WEB_ROOT_TTZ "/home/wwwroot/api.88008088.com/public/resource/admin/ttz.user"

int checkIP(char *ip);

int checkUid(int uid);

void cleanIP(char *ip);

int base64_encode(const char * sourcedata, char * base64);

int createHDS(int hds);

int getHDS(int hds);

int getForkid();

int selectForkid(int user,short int gid,short int type,short int grade,int hds);

int createSession(int sock,char *ip);

int checkSession(int forkid,int session,int uid,int sock);

int getUserBySession(int session);

void cleanSession(int user);

void getUserBySocket(int forkid,int sock,int *user);

int split(char **arr, char *str, const char *del);

int readFile(char *path,char *buf,int size);

int writeFile(char *path,char *buf);

int writeToFile(char *path,char *buf);

int getDate(time_t *time);

char  *formatTime(char *str_time);

double getTimeNow(char *str_time);

void randomNum(char *buf,int forkid);


void cleanUser(int user);

void AllSeatSend(int forkid,int desk,char *buf,int length);

void OtherSeatSend(int forkid,int desk,int seat,char *buf,int length);

void AllBjlSend(int forkid,char *buf,int length);

int UserSend(int user,char *buf,int length);

int User_msg(int user,char *msg);

int getUserGoldFromRedis(int user,int type);

int updateUserGold(int user);

int getSSCOrder(int user);

void insertAAAMsg(int uid,char *name,short int gid,long long int gold_win,int check_type);

int myRandom();

void flushUser(int user);

int ZJH_ready(int user);

int createZJHDesk(int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int round);

int createWZQDesk(int user,short int gid,short int type,short int grade);

int createOXDesk(int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int model,short int kan);

int createMJDesk(int user,short int gid,short int type,short int grade,short int flag,long long int bottom,short int ip,short int seat_max,short int mj_change,short int mj_zm,short int mj_times,short int mj_dg,short int mj_yjjd,short int mj_mqzz,short int mj_td);

int ZJH_inDesk(int user,int forkid,char *token,int hds);

int gameInfo(int user);

int deskOut(int user);

int User_stock(int user,short int type,long long int gold);

int User_order(int user,long long int order);

void User_order_check(int user);