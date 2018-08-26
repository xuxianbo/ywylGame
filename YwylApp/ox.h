int OXCommand(int forkid,int user,int command,char *buf);

void OXTimeOut(int forkid,int user,char *buf);
//牛牛庄家设置
void setOXBanker(int forkid,int desk);

int OXIsTab(int forkid,int desk);

void setOXBet(int forkid,int desk);
//牛牛庄家结算
void OX_balance(int forkid,int desk,int seat,char *buf);
//牛牛开牌
int OX_PK(int forkid,int desk,int seat,char *buf);

int OX_bet(int forkid,int desk,int seat,long long int gold);