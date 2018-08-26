#if !defined(MYSHMH)
#define MYSHMH 1

#include <sys/types.h>
#include <semaphore.h>

#define SHM_KEY 201864
#define FORK_MAX 152				//房间数量上限
#define RECV_BUF_MAX 2048			//消息缓存最大值

#define USER_MAX 10000				//玩家数量上限
#define USER_NAME_MAX 65			//用户名长度上限
#define USER_HEAD_URL 160			//用户头像url长度上限
#define IP_MAX 32					//用户ip地址长度

#define DESK_MAX 60					//每个房间桌子总数上限
#define SEAT_MAX 9					//每张桌子座位总数上限
#define HAND_MAX 28					//手牌最大张数上限
#define CARD_MAX 108				//公牌的数量上限

#define BJL_HOUSE_MAX 10			//百家乐房间上限
#define BJL_HAND_MAX 5				//百家乐手牌上限
#define BANKER_MAX 20				//百家乐上庄队列上限
#define BJL_BET_MAX	4				//百家乐投注区域数量上限
#define BJL_L_MAX 20				//百家乐历史开奖记录
#define BJL_SEAT_MAX 8				//百家乐可视桌位数
#define MSG_LIST_MAX 10				//百家乐同时聊天上限
#define WINNER_MAX	5				//百家乐大赢家列表上限
#define ORDER_MAX 10				//订单并发上限

#define MJ_CPK_MAX	5				//麻将吃碰杠次数上限

//#define ROBOT_NUM_MAX 1000			//机器人数上限

/**
 *    Thke IPC key of the share memory
 */

//#define  MYSHM_DEBUG 1	

#if defined(__cplusplus)
extern "C" {
#endif

struct MJ_GANG
{									
	short int flag;			//0初始化 1暗杠，2巴杠，3过手杠，4点杠
	short int card_gang;	//牌值 初始化0
	short int seat[3];		//被杠座位号 -1 初始化
	long long int gold[3];	//输赢金币数
};
typedef struct MJ_GANG MJ_GANG;

struct MJ_WIN
{								
	short int card_type[7];	//0初始化,胡的7种牌型

	short int geng;			//0初始化，根数量
	short int times;		//0初始化，番数
	short int card_win;		//0初始化 胡的牌值
	short int flag;			//0初始化 1点炮,2自摸,3未胡,4无叫,5查大叫,6花猪
	short int td;			//0初始化 1天胡,2地胡
	short int jg;			//0初始化 1金勾钓 
	short int hd;			//0初始化 1海底
	short int gh;			//0初始化 1杠上花，2抢杠胡，3接杠上炮
	short int seat[3];		//输家座位号 -1初始化
	long long int gold[3];	//输赢金币数
};
typedef struct MJ_WIN MJ_WIN;

//炸金花
struct SEAT
{									
	int user;						//USER_id 
	long long int in_all;			//此座位当前总下注数//抢庄倍数或牛牛倍数(0未抢不抢,1-4倍)
	time_t begin_time;				//操作时间缀
	short int status;				//座位状态 0无人，1（游戏未开始）未准备，2（游戏未开始）已准备，3（游戏已开始）未看牌，4（游戏已开始）已看牌，5（游戏已开始）已弃牌
									//牛牛 0无人，1（游戏未开始）未准备，2（游戏未开始）已准备，3（游戏已开始）抢庄未选择，4（游戏已开始）抢庄已选择,未下注，5（游戏已开始）已下注,未开牌 6 已开牌
									//掼蛋 0无人，1（游戏未开始）未准备，2（游戏未开始）已准备，3（游戏已开始）游戏中
									//麻将 0无人，1未准备，2已准备，3游戏中，4已认输
	short int card[HAND_MAX];		//手牌
	short int card_copy[HAND_MAX];	//乱序手牌
	short int card_num;				//剩余手牌数量
	short int ranking;				//游戏排名 0未进行排名，1-3排名 4 认输
									//麻将
	short int card_change[4];		//要交换的牌
	short int win_times;			//当前可胡牌番数
	short int point_gang;			//点杠次数
	short int lose_num;				//被胡次数
	short int ba_gang;				//巴杠状态 0 初始化 1请求巴杠，2巴杠被抢杠胡
	short int flag_chi;				//吃 0不可 1可吃 过置回0 3同意
	short int flag_peng;			//碰 0不可 1可碰 过置回0 3同意
	short int flag_gang;			//杠 0不可 1可杠 过置回0 3同意
	short int flag_win;				//胡 0不可 1可胡 过置回0 3同意
	short int card_gang;			//杠牌
	short int lack;					//定缺花色 初始化0 1条,2筒,3万
	short int card_in;				//当前进牌 初始化0
	short int card_out;				//当前出牌 初始化0
	short int card_out_list[HAND_MAX];	//历史出牌
	short int card_out_num;				//历史出牌张数

	short int card_peng[MJ_CPK_MAX];		//已碰牌 0初始化 
	MJ_GANG gang[MJ_CPK_MAX];				//已杠牌
	MJ_WIN win[1];							//已胡牌
	short int num_peng;						//碰次数
	short int num_gang;						//杠次数

	sem_t lock;						//锁
};
typedef struct SEAT SEAT;

struct ZJH
{	
	SEAT seat[SEAT_MAX];			//座位
	short int cards[CARD_MAX];		//桌牌
	
	short int flag;					//游戏状态,0游戏未开始，1游戏进行中/牛牛抢庄中/掼蛋上贡阶段，2牛牛闲家押注倍数中/掼蛋还贡阶段,3掼蛋出牌阶段，
									//麻将 0游戏未开始，1游戏第一次换牌阶段，2游戏第二次换牌阶段，3游戏定缺阶段，4游戏行牌阶段 
	short int people;				//入座人数
	short int banker;				//庄家座位号
	short int turn;					//当前轮流座位号
	short int round;				//当前总轮数
	short int player;				//进行游戏人数
									//掼蛋
	//short int power;				//当前牌权方
	//short int is_pay;				//是否进贡,0不需要，1需要，2抗贡
	//short int pay[4];				//1，2，倒2，倒1游的贡牌 -1不需要进贡还贡，0未进贡，1，未还贡，N>1已进/还贡的牌
	//short int level[2];			//双方等级
	//short int card_type;			//桌牌的牌型
	//short int card_max;			//桌牌值最大值
	short int card_num;				//剩于牌数量
	
	time_t	start_time;				//游戏开始时间,0初始化,N时间缀
	long long int in_total;			//此桌当前下注总数 //牛牛定庄倍数
	long long int order_max;		//当前未看牌下注最大值
	sem_t lock;						//桌锁


									//私人场专用
	long long int bottom;			//底注
	short int seat_max;				//座位数量
	short int desk_status;			//桌子状态 0未启用，1可见，2不可见
	short int ip;					//是否允许同IP进入，0允许，1不允许

	short int round_max;			//炸金花不可开牌最大轮数
	short int model;				//牛牛模型 0普通 1激情
	short int kan;					//牛牛是否坎顺斗 0否 1是
									
									//麻将配置
	short int seat_dg;				//点杠座位号
	short int mj_change;			//0初始化，1一次换三张
	short int mj_zm;				//1自摸加底，2自摸加番
	short int mj_times;				//最大番数
	short int mj_dg;				//点杠上花 1自摸，2点炮
	short int mj_yjjd;				//幺九将对 0否 1是 
	short int mj_mqzz;				//门清中张 0否 1是
	short int mj_td;				//天地和 0否 1是
										//保存数据
	int uid[SEAT_MAX];					//用户id
	char name[SEAT_MAX][USER_NAME_MAX]; //用户名称
	long long int gold[SEAT_MAX];		//下注金额
	long long int gold_old[SEAT_MAX];	//旧金币数
	long long int gold_new[SEAT_MAX];	//新金币数
};
typedef struct ZJH ZJH;

struct BJL_SEAT
{									
	int user;						//USER_id 
	short int status;				//此座状态 0初始化无人，1有人
	sem_t lock;						//锁
};
typedef struct BJL_SEAT BJL_SEAT;

struct BJL_MSG
{									
	char msg[RECV_BUF_MAX];			//消息内容
	short int length;				//消息长度
};
typedef struct BJL_MSG BJL_MSG;

struct BJL
{	
	BJL_SEAT  seat[BJL_SEAT_MAX];			//座位
	time_t	start_time;						//游戏阶段时间,0初始化,N时间缀
	short int flag;							//游戏状态 0初始化游戏等待，1游戏开始可下注，2，游戏封盘开奖中，3，游戏结算中,4，游戏结束结果展示
	short int banker_card[BJL_HAND_MAX];	//庄家手牌
	long long int gold_win[BJL_BET_MAX];	//庄家的各方赢金数
	int	banker;								//当前庄家user
	short int banker_flag;					//庄家是否结算 0 没有 1已结算
	short int banker_times;					//玩家上庄局数
	short int list_num;						//上庄队列人数
	int banker_list[BANKER_MAX];			//上庄队列
	sem_t lock_banker;						//上庄队列锁
	int qh;									//最新期号

	short int card[BJL_BET_MAX][BJL_HAND_MAX];	//投注区域手牌
	short int win[BJL_BET_MAX];				//各方胜负 0初始化，1庄胜，2庄负
	long long int gold_total;				//总注
	long long int gold[BJL_BET_MAX];		//各方下注
	sem_t lock_bet;
	
	char win_name[WINNER_MAX][USER_NAME_MAX];		//大赢家名称列表
	char win_url[WINNER_MAX][USER_HEAD_URL];		//大赢家头像URL列表
	long long int win_max[WINNER_MAX];				//大赢家所赢金币列表
	int	win_uid[WINNER_MAX];						//大赢家uid列表
	sem_t lock_winner;								//大赢家锁

	sem_t lock_msg;							//聊天锁
	short int msg_list_flag;				//聊天列表状态,-1无数据，0有数据可写入，1不可写入
	BJL_MSG bjl_msg[MSG_LIST_MAX];			//百家乐聊天列表

	short int list[BJL_L_MAX][BJL_BET_MAX];	//历史开奖记录 0初始化，1庄胜，2庄负
	
	short int flag_msg[BJL_HOUSE_MAX];		//聊天通知
	short int flag_seat[BJL_HOUSE_MAX];		//座位信息是否变化
	short int flag_over[BJL_HOUSE_MAX];		//游戏是否结束
	short int flag_game[BJL_HOUSE_MAX];		//游戏通知到阶段
};
typedef struct BJL BJL;

struct FORK
{					
	ZJH	 zjh[DESK_MAX];				//桌子
	short int status;				//0:进程未启动，1:进程已启动
	int child_pid;					//进程系统id
	int fd[2];						//管道组
	short int thread;				//单进程中的线程数量

	int thread_max;					//线程上限
	short int gold_type;			//币种 1金币 ,2现金
	long long int gold_min;			//房间金币下限
	short int gameid;				//游戏id
	short int type;					//房间类型
	short int grade;				//房间等级
	short int cards_num;			//牌最大张数
};
typedef struct FORK FORK;	

struct ORDER
{									
	long long int order;
	time_t create_time;
	time_t update_time;
};
typedef struct ORDER ORDER;

//在线用户列表
struct USER
{				
	short int status;				//0：初始化，1：已分配session，2：正在切换房间，3：须踢下线，4:已断线，10：正常
	short int login;				//是否登录,0未登录，1已登录
	short int sex;					//性别，0男1女

	int uid;						//0:初始化，>0:已登录uid
	int hds;						//入桌位置(百家乐入桌位置 0~7位置可视 其余均为10)
	char name[USER_NAME_MAX];		//用户名称
	char url[USER_HEAD_URL];		//用户头像url
	char ip[IP_MAX];				//用户ip
	long long int gold_init;		//玩家初始金币数
	long long int gold;				//玩家当前金币数
	long long int money_init;		//玩家初始化现金数
	long long int money;			//当前现金数
	long long int stock;			//仓库现金数
	long long int cake;				//贡献蛋糕量
	int forkid;						//当前进程号
	int socket;						//当前socket
	short int msg_gold_update;		//金币更新通知 0 不需要通知 1 需要通知
	unsigned int msg_id;			//已通知到消息号
	time_t time;					//最后活跃时间
	sem_t lock_user;				//用户锁

	//bjl
	short int banker_flag[2];				//庄家状态，0初始化，1上庄等待中，2我要下庄，10上庄中
	short int bjl_flag[2];					//百家乐是否下注 0 没有或已结算，1 已下注  
	short int bjl_balance[2];				//百家乐是否结算通知 0 不需要通知 1 需要通知
	long long int bet[2][BJL_BET_MAX];		//各方下注总额
	long long int gold_win[2][BJL_BET_MAX];	//各方赢金数

	//充值
	ORDER order[ORDER_MAX];				//充值订单	
	short int order_num;				//订单数量
	sem_t lock_order;					
};
typedef struct USER USER;

struct AAA_MSG
{									
	char msg[128];					//消息内容
	short int length;				//消息长度
	unsigned int id;				//消息号
};
typedef struct AAA_MSG AAA_MSG;

struct MYIP
{									
	char ip[IP_MAX];				//ip地址
	short int num;					//连接数量
};
typedef struct MYIP MYIP;
//游戏大厅
struct HALL
{					
	FORK fork[FORK_MAX];			//进程房间
	USER user[USER_MAX];			//大厅用户列表	
	MYIP myip[USER_MAX];			//ip列表
	short int ip_num;				//当前连接ip总数
	sem_t ip_lock;					//ip锁

	BJL  bjl[2];					//百家乐桌子

	long long int pool_init;		//彩池初始值
	long long int pool;				//彩池当前值

	short int flag;					//系统状态 0:关闭,1:开启,2:关闭倒计时
	short int normal_num;			//房间总数
	short int thread;				//大厅线程数量
	time_t time_now;				//全系统当前时间
	time_t time_over;				//关服时间

	sem_t aaa_lock;					//大奖通知列表锁
	AAA_MSG aaa_msg[MSG_LIST_MAX];	//大奖通知列表
	short int site_msg;				//消息游标

	//char robot_name[ROBOT_NUM_MAX][USER_NAME_MAX];//机器人名称
};
typedef struct HALL HALL;			//Hall大厅结构

void set_shm_key(key_t i_key);
/**
 * init_myshm 初始化共享内存数据
 * @param i_flag 初始化标识  1. 要对共享内存内的数据进行复位设置，非1 则只初始化，以便获得共享内存句柄
 *
 * @retuen  无返回值
 */
void init_myshm(int i_flag);

void init_desk(int forkid,int desk,int flag);

void drop_myshm();
#if defined(__cplusplus)
}
#endif
#endif
