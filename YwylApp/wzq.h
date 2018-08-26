void wzqDealer(int forkid);

int WZQCommand(int forkid,int user,int command,char *buf);

void WZQTimeOut(int forkid,int user,char *buf);
//五子棋结算
int WZQ_balance(int user);
//设置学费
int WZQ_bet(int user,long long int gold);