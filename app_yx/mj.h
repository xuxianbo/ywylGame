int MJCommand(int forkid,int user,int command,char *buf);

void MJTimeOut(int forkid,int user,char *buf);

int MJPassOn(char *buf,int forkid,int desk);

void MJBalance(int forkid,int desk);

int MJIsTab(int forkid,int desk);

void setMJLack(int forkid,int desk);

short int getMJLack(short int *card,short int card_num);

int MJ_lack(char *buf,int forkid,int desk,int seat,short int parm);

int MJ_cardOut(char *buf,int forkid,int desk,int seat,short int card);

int MJ_action(char *buf,int forkid,int desk,int seat,short int type,short int card);

short int getMJCardOut(int forkid,int desk,int seat);

int getNumSameCard(short int card,int forkid,int desk,int seat);

void MJCheckMyself(char *buf,int forkid,int desk,int seat);
void MJCheckOther(char *buf,int forkid,int desk,int seat);

short int isMJPeng(int forkid,int desk,int seat);
short int isMJGang(int forkid,int desk,int seat);
short int isMJWin(int forkid,int desk,int seat);

void MJGetOneCard(char *buf,int forkid,int desk,int seat);

void MJSetOneCard(char *buf,int forkid,int desk,int seat);

int MJCheckAllSeat(int forkid,int desk);

int MJActionCheck(char *buf,int forkid,int desk);


short int is_jiang(int forkid,int desk,int seat);
short int is_me_qing(int forkid,int desk,int seat);
short int is_zhong_zhang(int forkid,int desk,int seat);
short int is_qing(int forkid,int desk,int seat);
short int is_yaojiu(int forkid,int desk,int seat);

void MJ_giveUp(char *buf,int forkid,int desk,int seat);
void MJ_getType(int forkid,int desk,int seat,short int *type,short int *geng,short int *times);

void MJCheckTing(int forkid,int desk);
void MJCheckTingZhu(int forkid,int desk);

void doMJChange(char *buf,int forkid,int desk,short int num);
void getMJChangeCards(int forkid,int desk,int seat,short int *card,short int num);
void setMJChangeCards(char *buf,int forkid,int desk,int seat,short int *card,short int num);