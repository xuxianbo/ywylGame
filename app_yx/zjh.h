int ZJHNormalCommand(int forkid,int user,int command,char *buf);

void ZJHNormalTimeOut(int forkid,int user,char *buf);

void passOn(char *buf,int forkid,int desk,int seat);

int ZJH_check(char *buf,int forkid,int desk,int seat);

int isGameOver(char *buf,int forkid,int desk,int seat);

int ZJH_PK(char *buf,int forkid,int desk,int seat,int pk_seat);

int ZJH_bet(char *buf,int forkid,int desk,int seat,long long gold);

int giveUp(char *buf,int forkid,int desk,int seat);

//void ZJH_robot(int forkid,int desk);

//void ZJH_robot_play_game(int forkid,int desk);

void zjhBalance(char *buf,int forkid,int desk);