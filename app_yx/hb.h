int HBCommand(int index,int user,int command,int forkid,char *buf);

int HBTimeOut(int forkid,int index,int user,char *buf);
//荷官
void HBDealer(int forkid);
//进场入桌
int HBInDesk(char *buf,int user,int forkid);
//离桌离场
int HBOutDesk(char *buf,int index,int user);

void HBCleanUser(int index,int user);
//发红包
int HBSendPacket(char *buf,int index,int user,long long int gold,short int type,char *passwd);
//抢红包
int HBCatchPacket(char *buf,int index,int user,int hbid);

int HBPacketInfo(char *buf,int user,int index,int hbid);

void checkList(int index);

//结算
void HBBalance(int index,int site);
//聊天
int HBUserMsg(char *buf,int index,int user);