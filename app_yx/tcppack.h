int pack555(char *buf);

int pack666(char *buf,short int flag);

int pack777(char *buf);

int pack888(char *buf,int ret);

int pack998(char *buf);

int pack999(char *buf);

int pack1000(char *buf,int session);

int pack2001(char *buf,short int flag,short int ret,short int gid,short int type,short int grade,int hds,int session);

int pack2002(char *buf,short int flag,short int gid,short int type,short int grade);

int pack2003(char *buf,short int flag,int forkid,int desk,short int seat);

int pack3015(char *buf,int forkid,int desk,int seat);

int pack4015(char *buf,int forkid,int desk);

//int pack3055(char *buf,int forkid,int desk,int seat);

//int pack4055(char *buf,int forkid,int desk);
//2004 2026
int packReturnFlag(char *buf,int command,short int flag);
//2022 2023
int packReturnFlagGold(char *buf,int command,short int flag,long long int gold);
//3004 3005 3007 3010
int packPublicSeat(char *buf,int command,short int seat);

int pack2006(char *buf,short int gid,short int type,short int grade,int flag);

int pack3006(char *buf,int forkid,int desk,int time);

int pack4006(char *buf,int forkid,int desk,short int time,short int flag);
//看牌
int packLookCards(char *buf,int forkid,int desk,int seat,int num);

int pack3008(char *buf,int forkid,int desk,int seat,long long int gold);

int pack3009(char *buf,short int seat,short int win,short int lose);

int pack3012(char *buf,int forkid,int desk,short int win,short int lose);

int pack3013(char *buf,int forkid,int desk,short int win,long long int gold_win);

//int pack4013(char *buf,short int seat_max,short int flag);

int pack3014(char *buf,char *msg,short int seat);

int pack3016(char *buf,int user);

int pack3017(char *buf,int forkid,int desk,short int win,long long int gold_win);

int pack2018(char *buf,short int gid,short int type);

int pack2019(char *buf,short int flag,short int gid,short int type,short int grade,int hds);
//bjl
int pack3020(int forkid,int index,char *buf,int bet_max,int look_max,long long int gold_banker);

int pack4020(int forkid,int index,char *buf,int bet_max);

int pack5020(int forkid,int index,char *buf,int bet_max);

int pack6020(int forkid,int index,char *buf,int bet_max);

int pack7020(int forkid,int index,char *buf,int user,int bet_max);

int pack2021(int index,char *buf,int user,short int type,short int flag,int bet_max);

int pack4021(int forkid,int index,char *buf,int user,int bet_max,int look_max,long long int gold_banker);

int pack3025(int index,char *buf);

int pack3026(char *buf,char *msg,int user);
//

int pack3027(char *buf,int uid,char *name,short int gid,short int check_type,long long int gold_win);

int pack3028(char *buf,char *msg,int user);

int pack3029(int index,char *buf,int bet_max);

int pack3030(char *buf);

int pack3040(char *buf,int forkid,int desk,int seat);

int pack3041(char *buf,long long int *balance,int forkid,int desk);

void createIP(char *ip,int uid);
/*
int pack3050(char *buf,short int seat,short int card);

int pack3051(char *buf,short int seat,short int *card,short int num,short int type);

int pack4051(char *buf,short int seat,short int ranking);

int pack5051(char *buf,int forkid,int desk);*/

int pack2060(char *buf,short int type,short int flag,long long int gold);

int pack3061(char *buf,int uid,long long int gold);

//MJ
int pack3070(char *buf,int forkid,int desk,int seat);

int pack4070(char *buf,int forkid,int desk);

int pack2071(char *buf,int forkid,int desk,int seat);

int pack2072(char *buf,short int status,short int mytime);

int pack2073(char *buf,short int flag,short int *cards,short int num);

int pack3073(char *buf,short int seat);

int pack4073(char *buf,short int direction,short int *cards,short int num);

int pack2074(char *buf,short int flag,short int type);

int pack3074(char *buf,short int flag,short int seat);

int pack4074(char *buf,int forkid,int desk);

int pack2075(char *buf,int forkid,int desk,int seat,short int flag,short int card);

int pack3076(char *buf,short int seat,short int type,short int card_seat,short int card,short int lastNum,short int handNum,short int win);

int pack2077(char *buf,short int flag,short int card);

int pack3077(char *buf,int forkid,int desk,int seat);

int pack4077(char *buf,short int type,short int seat,short int card,short int num);

int pack2078(char *buf,short int seat,short int num,short int card);

int pack3079(char *buf,short int seat,short int time);

int pack3080(char *buf,int forkid,int desk);

int pack3081(char *buf,short int flag,short int num,long long int *gold,long long int *gold_new);

int pack3082(char *buf,short int seat);

int pack2083(char *buf,short int flag,long long int gold);

int pack2090(char *buf,short int flag,short int msg_id);

int pack3090(char *buf,int uid,short int msg_type,short int msg_len,char *msg);

int pack3091(char *buf,int index,int site);

int pack2092(char *buf,short int flag,long long int money,int hbid);

int pack2093(char *buf,short int flag,long long int money,int tid);

int pack2094(char *buf,short int flag,long long int gold,long long int lose,long long int code,long long int gain,long long int money,int hbid,short int name_len,char *name);

int pack3094(char *buf,int hbid,int uid,short int name_len,char *name);

int pack4094(char *buf,int hbid,short int time);

int packX097(int command,short int flag,char *buf,int index,int site);