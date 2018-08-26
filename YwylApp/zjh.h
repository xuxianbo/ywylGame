int ZJHNormalCommand(int forkid,int user,int command,char *buf);

void ZJHNormalTimeOut(int forkid,int user,char *buf);

void passOn(int forkid,int desk,int seat);

int ZJH_check(int forkid,int desk,int seat);

int isGameOver(int forkid,int desk,int seat);

int ZJH_PK(int forkid,int desk,int seat,int pk_seat);

int ZJH_bet(int forkid,int desk,int seat,long long gold);

int giveUp(int forkid,int desk,int seat);

//void ZJH_robot(int forkid,int desk);

//void ZJH_robot_play_game(int forkid,int desk);

void zjhBalance(int forkid,int desk);