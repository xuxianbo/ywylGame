int BJLCommand(int index,int user,int command,int forkid,char *buf);

int BJLTimeOut(int forkid,int index,int user,char *buf);
//荷官
void bjlDealer(int forkid);
//进场入桌
int bjlInDesk(char *buf,int user,int forkid);
//获取游戏初始化信息
int bjlGameInfo(char *buf,int forkid,int index,int user);
//离桌离场
int bjlOutDesk(char *buf,int index,int user);
//清离出桌
void bjlCleanUser(int index,int user);
//坐下
int bjlSeatDown(char *buf,int index,int user,short int seat);
//站起
void bjlStandUp(int index,int seat);
//上庄
int bjlWantBanker(char *buf,int index,int user);
//下庄
int bjlGiveUpBanker(char *buf,int index,int user);

int bjlBankerList(char *buf,int index,int user);
//检查可显座位
//void bjlCheckSeat();
void bjlCheckBankerList(int index);
//定庄
void bjlSetBanker(char *buf,int index);
//结算
int bjlBalance(char *buf,int forkid,int index);
//下注
int bjlOrder(char *buf,int index,int user,short int type,long long int gold);

int bjlDraw(int index);

int drawRecord(int index);

void createMyCards(int index,int type);

int getType(int index);

void bjlRecord(int index);

int bjlUserMsg(char *buf,int index,int user);

int getBJLTimes(int index,int site);

void robotOrder(int index);