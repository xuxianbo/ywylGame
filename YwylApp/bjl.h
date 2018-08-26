int BJLCommand(int index,int user,int command,int forkid,char *buf);

int BJLTimeOut(int forkid,int index,int user,char *buf);
//荷官
void bjlDealer(int forkid);
//进场入桌
int bjlInDesk(int user,int forkid);
//获取游戏初始化信息
int bjlGameInfo(int forkid,int index,int user);
//离桌离场
int bjlOutDesk(int index,int user);
//坐下
int bjlSeatDown(int index,int user,short int seat);
//站起
void bjlStandUp(int index,int seat);
//上庄
int bjlWantBanker(int index,int user,char *token);
//下庄
int bjlGiveUpBanker(int index,int user);

int bjlBankerList(int index,int user);
//检查可显座位
//void bjlCheckSeat();
void bjlCheckBankerList(int index);
//定庄
void bjlSetBanker(int index);
//结算
int bjlBalance(int forkid,int index);
//下注
int bjlOrder(int index,int user,short int type,long long int gold,char *token);

int bjlDraw(int index);

int drawRecord(int index);

void createMyCards(int index,int type);

int getType(int index);

void bjlRecord(int index);

int bjlUserMsg(int index,int user,char *msg);
