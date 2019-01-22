//#define DEBUG_CARDS_H_

void initCards(short int *cards,int card_max);

int get_one(short int *cards,short int *max);

void getMoreCards(short int flag,short int num,short int *cards,short int *max,short int *val);

int check_type(short int a,short int b,short int c);

int cards_pk(short int *a,short int *b);

void print_card(short int *card);

void cutCards(short int *cards,short int *max,short int points,short int type);

void getThreeCardsByType(int type,short int *cards,short int *max,short int *val);
//TTZ
int cards_pk_ttz(short int *a,short int *b);

short int check_type_ttz(short int *card);

//OX
short int check_type_ox(short int *card,short int model,short int kan);

int cards_pk_ox(short int *a,short int *b,short int type_a,short int type_b);

int get_times_by_type(short int type,short int model);

//GD
void shuffle_cards(short int *cards,int num);

void deal_cards(short int *cards,short int *player,int seat,short int deal_num,short int level);

void desc_cards(short int *cards,short int num,short int level);

int check_type_gd(short int level,short int type,short int *cards,short int cards_num);

/* 获取牌数所代表的牌号
 * card 牌数
*/
short int get_point(short int card,short int max);

/* 获取花色
 * card 牌数
 * return i 1方块，2梅花，3红心，4黑桃，5王牌
*/
short int get_color(short int card);

/* 获取牌堆中最大相同牌数量
 * cards   牌堆
 * cards_num   牌堆内牌的数量
*/
short int get_equal_num(short int *cards,int cards_num);

short int get_heart_num(short int level_point,short int *cards,short int cards_num);

int get_gap_num_by_point(short int *cards_point,short int cards_num);

//MJ
void getMJCards(short int num,short int *cards,short int *max,short int *val);

void sort_asc(short int *cards,short int num);

void delMJCardsByCard(short int *cards,short int *num,short int card);

short int check_type_mj(short int *cards,short int num,short int *check_cards,short int is_jiang,short int is_men_qing,short int is_zhong_zhang,short int is_qing,short int is_yao_jiu);

int is_win_mj(short int *cards,short int num);

short int is_ting_mj(short int *cards,short int num,short int *ting_cards);

void order_by(short int *cards,short int num);