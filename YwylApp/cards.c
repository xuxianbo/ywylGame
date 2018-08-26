#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "cards.h"
#include "mybase.h"

//初始化牌堆
void initCards(short int *cards,int card_max)
{
	int i;
	if (card_max == 32)
	{
		for (i=0;i<card_max;i++)
		{
			cards[i] = (short)(i/8*14+7+i%8);
		}
	}
	else if (card_max == 40)
	{
		for (i=0;i<card_max;i++)
		{
			cards[i] = i/4 + 1;
		}
	}
	else if (card_max == 52 || card_max == 54)
	{
		for (i=0;i<card_max;i++)
		{
			cards[i] = (short)(i/13*14+2+i%13);
		}
	}
	else if (card_max == 108)
	{
		int j;
		short int tmp;
		for (i=0;i<card_max;i++)
		{
			cards[i] = (i/36)*10 + (i/4)%9 + 1;
		}
		//return;
		/*for (i=0;i<card_max/2;i+=3)
		{
			j = rand()%(card_max/2) + card_max/2;
			tmp = cards[i];
			cards[i] = cards[j];
			cards[j] = tmp;
		}

		return;*/
		/*for (i=0;i<card_max;i++)
		{
			if (i < card_max/2)
			{
				j = rand()%(card_max/2) + card_max/2;
				tmp = cards[i];
				cards[i] = cards[j];
				cards[j] = tmp;
			}
			else
			{
				j = rand()%(card_max/2);
				tmp = cards[i];
				cards[i] = cards[j];
				cards[j] = tmp;
			}
			
		}*/
		
		for (i=0;i<card_max;i++)
		{
			j = rand()%card_max;
			tmp = cards[i];
			cards[i] = cards[j];
			cards[j] = tmp;
		}

		for (i=0;i<card_max;i++)
		{
			j = rand()%card_max;
			tmp = cards[i];
			cards[i] = cards[j];
			cards[j] = tmp;
		}

		for (i=0;i<card_max;i++)
		{
			j = rand()%card_max;
			tmp = cards[i];
			cards[i] = cards[j];
			cards[j] = tmp;
		}
		//cards[53] = 25;
	}
}

void sort_asc(short int *cards,short int num)
{
	short int i,j,tmp;
	for (i=0;i<num-1;i++)
	{
		for (j=0;j<num-1-i;j++)
		{
			if (cards[j] > cards[j+1])
			{
				tmp = cards[j];
				cards[j] = cards[j+1];
				cards[j+1] = tmp;
			}
		}
	}
}

//从小到大排序 0扔到最后
void order_by(short int *cards,short int num)
{
	short int i,j,k;
	for (i = 0; i < num; ++i)
	{
		for(j = i + 1; j < num; ++j)
		{
			
			if(cards[i] > cards[j])
			{
				k = cards[i];
				cards[i] = cards[j];
				cards[j] = k;
			}

			if(cards[i] == 0)
			{
				k = cards[i];
				cards[i] = cards[j];
				cards[j] = k;
			}
		}
	}
}

int is_win_mj(short int *cards,short int num)
{
	if(num == 2)
	{
		if(cards[0] == cards[1])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(num < 5)
	{
		return 0;
	}
	else
	{
		if((num - 2) % 3 != 0)
		{
			return 0;
		}

		short int i,j=0;
		if(num == 14)
		{
			for(i = 0;i <num;)
			{
				if(cards[i] == cards[i+1])
				{
					i = i + 2;
					j++;
				}
				else
				{
					break;
				}
			}

			if(j == 7)
			{
	    		return 1;
			}
		}


		//记录将牌位置
		short int jiang = -1;

		short int new_num;

		short int a,b,c;

		short int cards_tmp_1[20] = {0};

		while(1)
		{
			i = jiang + 1;
			if(i >= num)
			{
				return 0;
			}

			for(j = 0; j < num; ++j)
			{
				cards_tmp_1[j] = cards[j];
			}

			if(jiang != -1)
			{
				if(cards_tmp_1[i] == cards_tmp_1[jiang])
				{
					++i;
				}
			}

			//寻找将牌位置，记录将牌第二个，并擦除该两张牌
			jiang = -1;
			for(;i < num - 1; ++ i)
			{
				if(cards_tmp_1[i] == cards_tmp_1[i + 1])
				{
					jiang = i + 1;
					cards_tmp_1[i] = 0;
					cards_tmp_1[i + 1] = 0;
					order_by(cards_tmp_1,num);
					break;
				}
			}

			// printf("i = %d\n", i);
			// printf("jiang = %d\n", jiang);

			// printf("牌堆是:");
			// for(j = 0; j < num; ++j)
			// {
			// 	printf("%d,",cards_tmp_1[j]);
			// }
			// printf("\n");

			//没有将牌，不能胡牌
			if(jiang == -1)
			{
				return 0;
			}

			//剩下的牌必须是3的倍数
			//从左边第一张牌开始，必须能组成刻子或者顺子才能胡牌，否则不能胡
			new_num = num - 2;
			while(new_num >= 3)
			{
				if(cards_tmp_1[0] == cards_tmp_1[1] && cards_tmp_1[0] == cards_tmp_1[2])
				{
					//找到了刻子 擦除
					cards_tmp_1[0] = 0;
					cards_tmp_1[1] = 0;
					cards_tmp_1[2] = 0;
					order_by(cards_tmp_1,num);
					new_num = new_num - 3;
				}
				else
				{
					//找顺子
					a = cards_tmp_1[0],b = 0,c = 0;
					for(j = 1;j < new_num; ++j)
					{
						if(cards_tmp_1[j] - a == 1)
						{
							a = cards_tmp_1[j];
							if(b == 0)
							{
								b = j;
							}
							else if(c == 0)
							{
								c = j;
								break;
							}
						}
					}

					if(b != 0 && c != 0)
					{
						cards_tmp_1[0] = 0;
						cards_tmp_1[b] = 0;
						cards_tmp_1[c] = 0;
						order_by(cards_tmp_1,num);
						new_num = new_num - 3;
					}
					else
					{
						break;
					}
				}
			}

			if(new_num == 0)
			{
				break;
			}
		}

		// printf("new_num = %d\n", new_num);
		if(new_num != 0)
		{
			return 0;
		}
		return 1;
	}
}

short int is_ting_mj(short int *cards,short int num,short int *ting_cards)
{
	short int i,j,k = 0;

	short int board[30] = { 0 };

	int res = 0;

	short int tmp_num = num + 1;

	//在数组给记录下对应牌值出现的次数
	for(i = 0;i < num;++i)
	{
		++board[cards[i]];
	}

	for (j = 1; j <= 29; ++j)
	{
		short int cards_tmp[14] = {0};
		for(i = 0;i < num;++i)
		{
			cards_tmp[i] = cards[i];
		}

		if(j % 10 != 0)
		{
			cards_tmp[num] = j;
			order_by(cards_tmp,tmp_num);

			//当牌不等于4的时候
			res = is_win_mj(cards_tmp,tmp_num);
			if(res == 1)
			{
				ting_cards[k] = j;
				k++;
			}
		}
	}

	return k;
}

/**
 * 获取牌符合的牌型
 * cards 玩家手牌
 * num 玩家手牌数
 * is_jiang 玩家碰杠牌是否全是258 如果是传1
 * is_men_qing 玩家是否有过碰牌或者明杠 如果是传1
 * is_zhong_zhang 玩家碰杠牌中是否有1或9 如果否传1
 * check_cards 牌堆牌型 顺序是清一色 对对胡 七对 全幺九 中张 将 门清
*/

short int check_type_mj(short int *cards,short int num,short int *check_cards,short int is_jiang,short int is_men_qing,short int is_zhong_zhang,short int is_qing)
{
	short int i,j;

	short int *cards_tmp = cards;

	short int board[30] = { 0 };

	//三条数
	short int cnt_kan = 0;

	//顺子数
	short int cnt_sun = 0;

	//对子数
	short int cnt_dui = 0;

	//四对数
	short int cnt_gen = 0;

	if((num - 2) % 3 == 0)
	{
		short int n = (num - 2) / 3;

		//在数组给记录下对应牌值出现的次数
		for(i = 0;i < num;++i)
		{
			++board[cards_tmp[i]];
		}

		int qi_dui = 1,quan_yao = 1;

		if(num != 14)
		{
			qi_dui = 0;
			quan_yao = 0;
		}

		//开始检测  清一色
		for (j = 1; j <= 9;)
		{
			if(j + 2 <= 9)
			{

				//如果牌值和牌值+1 +2 都存在那么说明存在这一个顺子 顺子加一 下次检测的牌值加3
				if (board[j] && board[j + 1] && board[j + 2])
				{
					if(board[j] == 2 && board[j + 1] == 2 && board[j + 2] == 2)
					{
						cnt_sun = cnt_sun + 2;
					}
					else
					{
						cnt_sun++;
					}

					if(j % 10 != 1 && j % 10 != 9 && (j + 1) % 10 != 1 && (j + 1) % 10 != 9 && (j + 2) % 10 != 1 && (j + 2) % 10 != 9 && quan_yao == 1)
					{
						quan_yao = 0;
					}

					j += 3;
				}
				else
				{
					if(board[j] == 2)
					{
						if(j % 10 != 1 && j % 10 != 9 && quan_yao == 1)
						{
							quan_yao = 0;
						}
					}
					j++;
				}
			}
			else
			{
				j++;
			}
		}

		for (j = 11; j <= 19;)
		{
			if(j + 2 <= 19)
			{

				//如果牌值和牌值+1 +2 都存在那么说明存在这一个顺子 顺子加一 下次检测的牌值加3
				if (board[j] && board[j + 1] && board[j + 2])
				{
					if(board[j] == 2 && board[j + 1] == 2 && board[j + 2] == 2)
					{
						cnt_sun = cnt_sun + 2;
					}
					else
					{
						cnt_sun++;
					}

					if(j % 10 != 1 && j % 10 != 9 && (j + 1) % 10 != 1 && (j + 1) % 10 != 9 && (j + 2) % 10 != 1 && (j + 2) % 10 != 9 && quan_yao == 1)
					{
						quan_yao = 0;
					}

					j += 3;
				}
				else
				{
					if(board[j] == 2)
					{
						if(j % 10 != 1 && j % 10 != 9 && quan_yao == 1)
						{
							quan_yao = 0;
						}
					}
					j++;
				}
			}
			else
			{
				j++;
			}
		}

		for (j = 21; j <= 29;)
		{
			if(j + 2 <= 29)
			{

				//如果牌值和牌值+1 +2 都存在那么说明存在这一个顺子 顺子加一 下次检测的牌值加3
				if (board[j] && board[j + 1] && board[j + 2])
				{
					if(j % 10 != 1 && j % 10 != 9 && (j + 1) % 10 != 1 && (j + 1) % 10 != 9 && (j + 2) % 10 != 1 && (j + 2) % 10 != 9 && quan_yao == 1)
					{
						quan_yao = 0;
					}

					if(board[j] == 2 && board[j + 1] == 2 && board[j + 2] == 2)
					{
						cnt_sun = cnt_sun + 2;
					}
					else
					{
						cnt_sun++;
					}
					j += 3;
				}
				else
				{
					if(board[j] == 2)
					{
						if(j % 10 != 1 && j % 10 != 9 && quan_yao == 1)
						{
							quan_yao = 0;
						}
					}
					j++;
				}
			}
			else
			{
				j++;
			}
		}

		//printf("一条有%d张\n", board[1]);
		//检测有多少对子 三条 根 还有是否是七对和全幺九
		for (j = 1; j <= 29; ++j)
		{
			//如果一个牌值出现的次数大于等于了3次 则三条加1
			if (board[j] == 2)
			{
				cnt_dui++;
			}
			else if (board[j] == 3)
			{
				cnt_kan++;
				if(j % 10 != 1 && j % 10 != 9 && quan_yao == 1)
				{
					quan_yao = 0;
				}
			}
			else if (board[j] == 4 || board[j] == 5)
			{
				cnt_gen++;
			}

			//开始检测是否有七对和全幺九
			if(num == 14)
			{
				if(board[j] != 0)
				{
					if(board[j] != 2 && qi_dui == 1 && board[j] != 4)
					{
						qi_dui = 0;
					}
				}
			}
			else
			{
				qi_dui = 0;
				quan_yao = 0;
			}
		}

		//判断是否清一色
		if(is_qing == 1)
		{
			check_cards[0] = 1;
		}

		//判断对对胡
		if(cnt_kan == n && cnt_dui == 1)
		{
			check_cards[1] = 1;
		}

		//判断七对
		if(qi_dui == 1)
		{
			check_cards[2] = 1;
		}

		//判断全幺九
		if(quan_yao == 1)
		{
			check_cards[3] = 1;
		}

		//判断中张
		if(is_zhong_zhang == 1 && board[1] == 0 && board[9] == 0 && board[11] == 0 && board[19] == 0 && board[21] == 0 && board[29] == 0)
		{
			check_cards[4] = 1;
		}

		//判断将牌
		if(is_jiang == 1 && board[2] + board[5] + board[8] + board[12] + board[15] + board[18] + board[22] + board[25] + board[28] == num)
		{
			check_cards[5] = 1;
		}

		//判断门清
		if(is_men_qing == 1 && check_cards[0] != 1)
		{
			check_cards[6] = 1;
		}
	}

	return cnt_gen;
}

void delMJCardsByCard(short int *cards,short int *num,short int card)
{
	short int i,j;
	for (i=0;i<*num;i++)
	{
		if (cards[i] == card)
		{
			cards[i] = 0;
			for (j=i;j<*num-1;j++)
			{
				cards[j] = cards[j+1];
				cards[j+1] = 0;
			}
			break;
		}
	}
	(*num)--;
}

void getMJCards(short int num,short int *cards,short int *max,short int *val)
{
	int i,j,tmp;
	for (i=0;i<num;i++)
	{
		val[i] = cards[i];
	}

	for (i=0;i<(*max-num);i++)
	{
		cards[i] = cards[i+num];
	}

	*max -= num;
	//冒泡排序
	for (i=0;i<num-1;i++)
	{
		for (j=0;j<num-1-i;j++)
		{
			if (val[j] > val[j+1])
			{
				tmp = val[j];
				val[j] = val[j+1];
				val[j+1] = tmp;
			}
		}
	}
}

//洗牌
void shuffle_cards(short int *cards,int num)
{
	short int cards_1[54];
	short int cards_2[54];

	if (num == 1)
	{
		initCards(cards_1,54);
		//to do 乱序
	}
	else
	{
		initCards(cards_1,54);
		initCards(cards_2,54);

		int i,k,j,T,tmp;
		k = 54;
		for(i=0;i<54;i++)
		{
			cards[i] = cards_1[i];
			cards[k] = cards_2[i];
			k++;
		}
		
		T = rand()%4 + 2;
		
		while(T--)
		{
			for(i=0;i<54;i++)
			{
				j = rand()%54+54;
				tmp = cards[i];
				cards[i] = cards[j];
				cards[j] = tmp;
			}
		}
	}
}

//发牌
void deal_cards(short int *cards,short int *player,int seat,short int deal_num,short int level)
{	
	int i;
	for(i = 0; i < deal_num; i++)
	{
		player[i] = cards[i+seat*deal_num];
		cards[i+seat*deal_num] = 0;
	}

	desc_cards(player,deal_num,level);
}

//带主倒序 level -1 无主牌
void desc_cards(short int *cards,short int num,short int level)
{
	short int tmp,a1,a2;
	int i,j;
	//特殊处理
	if (level >= 2)
	{
		level = level%14?level%14:14;
	}
	
	//带主倒序
	for (i=0;i<num-1;i++)
	{
		for (j=0;j<num-1-i;j++)
		{
			if (cards[j] == 0)
			{
				a1 = 0;
			}
			else
			{
				a1 = get_point(cards[j],14);
			}
			
			if (cards[j+1] == 0)
			{
				a2 = 0;
			}
			else
			{
				a2 = get_point(cards[j+1],14);
			}
			
			
			if (a1 == level)
			{
				//主牌
				a1 += 13;
			}

			if (a2 == level)
			{
				a2 += 13;
			}


			if (a1 < a2)
			{
				tmp = cards[j];
				cards[j] = cards[j+1];
				cards[j+1] = tmp;
			}
			else if (a1 == a2 && cards[j] < cards[j+1])
			{
				tmp = cards[j];
				cards[j] = cards[j+1];
				cards[j+1] = tmp;
			}
		}
	}
}

/* 掼蛋 判断牌型是否一致 返回当前最大牌值 1单牌，2对子，3三不带，4顺子，5三带二，6木板，7钢板，8炸弹(4张)，9炸弹(5张)，10同花顺，11炸弹(6张),12炸弹(7张),13炸弹(8张),14炸弹(9张),15炸弹(10张), 16火箭
 * return -1失败, N>0 当前最大牌值 
 * level 当前主牌
 * type  要判断的牌型
 * cards   需要判断的牌
 * cards_num   需要判断的牌的数量
*/
int check_type_gd(short int level,short int type,short int *cards,short int cards_num)
{
	if(cards_num < 0 || cards_num > 10)
	{
		return -1;
	}

	if (cards_num == 0)
	{
		//不出牌
		return 0;
	}

	int i,j,tmp_point,tmp_color,k,max;
	short int cards_point[10];
	short int heart,same;
	level = level%14?level%14:14;
	
	if (type == 1)
	{
		//单牌
		if (cards_num == 1)
		{
			max = get_point(cards[0],14);
			if (max == level)
			{
				max += 14;
			}
			return max;
		}
		else
		{
			return -1;
		}
	}
	//红心主数量
	heart = get_heart_num(level,cards,cards_num);
	//剔除红心主
	j = 0;
	for (i=0;i<cards_num;i++)
	{
		tmp_color = get_color(cards[i]);
		tmp_point = get_point(cards[i],14);

		if (tmp_color != 3 || tmp_point != level)
		{
			cards_point[j] = get_point(cards[i],14);
			j++;
		}
	}

	//无主排序
	desc_cards(cards_point,j,-1);
	
	//红心主除外 相同点数牌的数量
	same = get_equal_num(cards_point,j);

	switch (type)
	{
		case 2:
			//对子
			if (cards_num == 2)
			{
				switch (heart)
				{
					case 0:
						if (same == 2)
						{
							max = cards_point[0];
							if (max == level)
							{
								max += 14;
							}
							return max;
						}
						break;
					case 1:
						if (cards_point[0] < 58)
						{
							//非鬼牌
							max = cards_point[0];
							if (max == level)
							{
								max += 14;
							}
							return max;
						}
						break;
					case 2:
						return level+14;
					default:
						return -1;
				}
			}
			break;
		case 3:
			//三不带
			if (cards_num == 3 && cards_point[0] < 58)
			{
				switch (heart)
				{
					case 0:
						if (same == 3)
						{
							max = cards_point[0];
							if (max == level)
							{
								max += 14;
							}
							return max;
						}
						break;
					case 1:
						if (same == 2)
						{
							max = cards_point[0];
							if (max == level)
							{
								max += 14;
							}
							return max;
						}
						break;
					case 2:
						max = cards_point[0];
						if (max == level)
						{
							max += 14;
						}
						return max;
					default:
						return -1;
				}
			}
			break;
		case 4:
			//顺子
			if (cards_num == 5 && cards[0] < 58)
			{
				//正向
				k = get_gap_num_by_point(cards_point,j);
				
				if (k <= heart)
				{
					max = cards_point[0] + heart - k;
					if (max > 14)
					{
						max = 14;
					}
					return max;
				}
				//是否反向
				if (cards_point[0] == 14)
				{
					cards_point[0] = 1;
					desc_cards(cards_point,j,-1);
					k = get_gap_num_by_point(cards_point,j);
					if (k <= heart)
					{
						return 5;
					}
				}
			}
			break;
		case 5:
			//三带二
			if (cards_num == 5)
			{
				switch (heart)
				{
					case 0:
						if (same == 3)
						{
							if (cards_point[1] == cards_point[2])
							{
								if (cards_point[3] == cards_point[4])
								{
									max = cards_point[0];
									if (max == level)
									{
										max += 14;
									}
									return max;
								}
							}
							else
							{
								if (cards_point[0] == cards_point[1])
								{
									max = cards_point[2];
									if (max == level)
									{
										max += 14;
									}
									return max;
								}
							}
						}
						break;
					case 1:
						if (same == 3)
						{
							if (cards_point[0] < 58)
							{
								if (cards_point[0] == cards_point[1])
								{
									max = cards_point[0];
									if (max == level)
									{
										max += 14;
									}
									return max;
								}
								else
								{
									max = cards_point[1];
									if (max == level)
									{
										max += 14;
									}
									return max;
								}
							}
						}
						else if (same == 2)
						{
							if (cards_point[0] == cards_point[1] && cards_point[2] == cards_point[3])
							{
								//
								if (cards_point[0] >= 58 && cards_point[2] < 58)
								{
									max = cards_point[2];
									if (max == level)
									{
										max += 14;
									}
									return max;
								}
								else if (cards_point[0] < 58)
								{
									k = cards_point[0];
									if (k == level)
									{
										k += 14;
									}

									max = cards_point[2];
									if (max == level)
									{
										max += 14;
									}

									if (k > max)
									{
										return k;
									}

									return max;
								}

								//
							}
						}
						break;
					case 2:
						if (same == 3)
						{
							return cards_point[0];
						}
						else if (same == 2)
						{
							if (cards_point[0] >= 58 && cards_point[2] < 58)
							{
								max = cards_point[2];
								if (max == level)
								{
									max += 14;
								}
								return max;
							}
							else if (cards_point[0] < 58)
							{
								k = cards_point[0];
								if (k == level)
								{
									k += 14;
								}

								max = cards_point[2];
								if (max == level)
								{
									max += 14;
								}

								if (k > max)
								{
									return k;
								}

								return max;
							}
						}
						break;
					default:
						return -1;
				}
			}
			break;
		case 6:
			//木板
			if (cards_num == 6 && cards_point[0] < 58 && same == 2)
			{
				switch (heart)
				{
					case 0:
						if (cards_point[0] == cards_point[1] && cards_point[2] == cards_point[3] && cards_point[4] == cards_point[5])
						{
							//是否顺
							if (cards_point[0] == cards_point[2] + 1 && cards_point[2] == cards_point[4] + 1)
							{
								return cards_point[0];
							}

							//AA3322
							if (cards_point[0] == 14 && cards_point[2] == 3 && cards_point[4] == 2)
							{
								return 3;
							}
						}
						break;
					case 1:
						if (cards_point[0] != cards_point[1])
						{
							if (cards_point[1] == cards_point[2] && cards_point[3] == cards_point[4])
							{
								//是否顺
								if (cards_point[0] == cards_point[2] + 1 && cards_point[2] == cards_point[4] + 1)
								{
									return cards_point[0];
								}

								//AA3322
								if (cards_point[0] == 14 && cards_point[2] == 3 && cards_point[4] == 2)
								{
									return 3;
								}
							}
						}
						else if (cards_point[3] != cards_point[4])
						{
							if (cards_point[0] == cards_point[1] && cards_point[2] == cards_point[3])
							{
								//是否顺
								if (cards_point[0] == cards_point[2] + 1 && cards_point[2] == cards_point[4] + 1)
								{
									return cards_point[0];
								}
								//AA3322
								if (cards_point[0] == 14 && cards_point[2] == 3 && cards_point[4] == 2)
								{
									return 3;
								}
							}
						}
						else
						{
							if (cards_point[0] == cards_point[1] && cards_point[3] == cards_point[4])
							{
								//是否顺
								if (cards_point[0] == cards_point[2] + 1 && cards_point[2] == cards_point[4] + 1)
								{
									return cards_point[0];
								}

								//AA3322
								if (cards_point[0] == 14 && cards_point[2] == 3 && cards_point[4] == 2)
								{
									return 3;
								}
							}
						}
						break;
					case 2:
						if (cards_point[0] == cards_point[1])
						{
							if (cards_point[2] != cards_point[3])
							{
								//是否顺
								if (cards_point[1] == cards_point[2] + 1 && cards_point[2] == cards_point[3] + 1)
								{
									return cards_point[0];
								}

								//AA3322
								if (cards_point[1] == 14 && cards_point[2] == 3 && cards_point[3] == 2)
								{
									return 3;
								}
							}
							else
							{
								if (cards_point[1] - cards_point[2] == 1)
								{
									max = cards_point[0] + 1;
									if (max > 14)
									{
										max = 14;
									}
									return max;
								}
								else if (cards_point[1] - cards_point[2] == 2)
								{
									return cards_point[0];
								}

								if (cards_point[1] == 14 && cards_point[2] <= 3)
								{
									return 3;
								}
							}
							
						}
						else if (cards_point[1] == cards_point[2])
						{
							//是否顺
							if (cards_point[0] == cards_point[2] + 1 && cards_point[2] == cards_point[3] + 1)
							{
								return cards_point[0];
							}

							//AA3322
							if (cards_point[0] == 14 && cards_point[2] == 3 && cards_point[3] == 2)
							{
								return 3;
							}
						}
						else if (cards_point[2] == cards_point[3])
						{
							if (cards_point[0] != cards_point[1])
							{
								//是否顺
								if (cards_point[0] == cards_point[1] + 1 && cards_point[1] == cards_point[2] + 1)
								{
									return cards_point[0];
								}

								//AA3322
								if (cards_point[0] == 14 && cards_point[1] == 3 && cards_point[2] == 2)
								{
									return 3;
								}
							}
						}
						break;
					default:
						return -1;
				}
			}
			break;
		case 7:
			//钢板
			if (cards_num == 6 && cards_point[0] < 58)
			{
				switch (heart)
				{
					case 0:
						if (cards_point[0] == cards_point[1] && cards_point[1] == cards_point[2] && cards_point[3] == cards_point[4] && cards_point[4] == cards_point[5])
						{
							if (cards_point[2] == cards_point[3] + 1)
							{
								return cards_point[0];
							}

							if (cards_point[2] == 14 && cards_point[3] == 2)
							{
								return 2;
							}
						}
						break;
					case 1:
						if (cards_point[1] != cards_point[2])
						{
							if (cards_point[0] == cards_point[1] && cards_point[2] == cards_point[3] && cards_point[3] == cards_point[4])
							{
								if (cards_point[1] == cards_point[2] + 1)
								{
									return cards_point[0];
								}

								if (cards_point[1] == 14 && cards_point[2] == 2)
								{
									return 2;
								}
							}
						}
						else
						{
							if (cards_point[3] == cards_point[4] && cards_point[0] == cards_point[1] && cards_point[1] == cards_point[2])
							{
								if (cards_point[2] == cards_point[3] + 1)
								{
									return cards_point[0];
								}

								if (cards_point[2] == 14 && cards_point[3] == 2)
								{
									return 2;
								}
							}
						}
						break;
					case 2:
						if (cards_point[0] == cards_point[1] && cards_point[1] == cards_point[2])
						{
							if (cards_point[2] == cards_point[3] + 1)
							{
								return cards_point[0];
							}

							if (cards_point[2] == 14 && cards_point[3] == 2)
							{
								return 2;
							}
						}
						else if (cards_point[1] == cards_point[2] && cards_point[2] == cards_point[3])
						{
							if (cards_point[0] == cards_point[1] + 1)
							{
								return cards_point[0];
							}

							if (cards_point[0] == 14 && cards_point[1] == 2)
							{
								return 2;
							}
						}
						else if (cards_point[0] == cards_point[1] && cards_point[2] == cards_point[3])
						{
							if (cards_point[1] == cards_point[2] + 1)
							{
								return cards_point[0];
							}

							if (cards_point[1] == 14 && cards_point[2] == 2)
							{
								return 2;
							}
						}
						break;
					default:
						return -1;
				}
			}
			break;
		case 8:
			//4炸
			if (cards_num == 4 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 9:
			//5炸
			if (cards_num == 5 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 11:
			//6炸
			if (cards_num == 6 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 12:
			//7炸
			if (cards_num == 7 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 13:
			//8炸
			if (cards_num == 8 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 14:
			//9炸
			if (cards_num == 9 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 15:
			//10炸
			if (cards_num == 10 && cards_point[0] < 58)
			{
				if (same + heart == cards_num)
				{
					max = cards_point[0];
					if (max == level)
					{
						max += 14;
					}
					return max;
				}
			}
			break;
		case 10:
			//同花顺
			if (cards_num == 5 && cards_point[0] < 58)
			{
				k = 0;
				for (i=0;i<cards_num;i++)
				{
					tmp_color = get_color(cards[i]);
					tmp_point = get_point(cards[i],14);

					if (tmp_color != 3 || tmp_point != level)
					{
						if (k == 0)
						{
							k = tmp_color;
						}
						else
						{
							if (k != tmp_color)
							{
								return -1;
							}
						}
					}
				}
				//正向
				k = get_gap_num_by_point(cards_point,j);
				if (k <= heart)
				{
					max = cards_point[0] + heart - k;
					if (max > 14)
					{
						max = 14;
					}
					return max;
				}
				//是否反向
				if (cards_point[0] == 14)
				{
					cards_point[0] = 1;
					desc_cards(cards_point,j,-1);
					k = get_gap_num_by_point(cards_point,j);
					if (k <= heart)
					{
						return 5;
					}
				}
				
			}
			break;
		case 16:
			//4王
			if (cards_num == 4)
			{
				for (i=0;i<cards_num;i++)
				{
					if (cards_point[i] < 58)
					{
						return -1;
					}
				}
				return cards_point[0];
			}
			break;
		default:
			return -1;
	}
	return -1;
}

//获取顺子缺口值
int get_gap_num_by_point(short int *cards_point,short int cards_num)
{
	int i,k;
	k = 0;
	for (i=1;i<cards_num;i++)
	{
		if (cards_point[i] + 1 != cards_point[i-1])
		{
			k += (cards_point[i-1] - cards_point[i] - 1);
		}
	}
	return k;
}

//获取红心主牌数量 
short int get_heart_num(short int level_point,short int *cards,short int cards_num)
{
	int i,point,color,num=0;
	for (i=0;i<cards_num;i++)
	{
		point = get_point(cards[i],14);
		color = get_color(cards[i]);

		if (point == level_point && color == 3)
		{
			num++;
		}

		if (num == 2)
		{
			return num;
		}
	}

	return num;
}

/* 获取牌数所代表的牌号
 * card 牌数
*/
short int get_point(short int card,short int max)
{
	if (max == 10)
	{
		if (card == 10)
		{
			return 5;
		}
		else
		{
			return card*10;
		}
	}
	else
	{
		if (card < 58)
		{
			return card%14?card%14:max;
		}
		else
		{
			return card;
		}
	}
}

/* 获取花色
 * card 牌数
 * return i 1方块，2梅花，3红心，4黑桃，5王牌
*/
short int get_color(short int card)
{
	return card%14?card/14+1:card/14;
}

/* 获取牌堆中最大相同牌数量
 * cards   牌堆
 * cards_num   牌堆内牌的数量
*/
short int get_equal_num(short int *cards,int cards_num)
{
	if(cards_num < 1)
	{
		return -1;
	}

	int i,tmp,j,k,l;
	i = 0;
	j = 0;
	k = 0;
	l = 0;
	for(i = 0; i < cards_num; i++)
	{
		tmp = get_point(cards[i],14);

		if(j != tmp)
		{
			j = tmp;
			if(k > l)
			{
				l = k;
			}
			k = 1;
		}
		else
		{
			k++;
		}
	}

	if (k > l)
	{
		return k;
	}

	return l;
}

int get_one(short int *cards,short int *max)
{
	int i,j,result;
	if (*max <= 0)
	{
		return -1;
	}
	else
	{
		i = myRandom();
		i = i%(*max);
	}
	result=cards[i];

	for (j=i;j<(*max);j++)
	{
		cards[j] = cards[j+1];
	}
	(*max)--;
	return result;
}

//points 点数 type 花色
void cutCards(short int *cards,short int *max,short int points,short int type)
{
	int i,j;
	short int a,b,tmp[52];
	j = 0;
	if (points > 0 && type > 0)
	{
		for (i=0;i<(*max);i++)
		{
			a = get_point(cards[i],14);
			b = get_color(cards[i]);
			if (a == points && b == type)
			{
				cards[i] = 0;
			}
			else
			{
				tmp[j] = cards[i];
				j++;
			}
		}
	}
	else if (points > 0)
	{
		for (i=0;i<(*max);i++)
		{
			a = get_point(cards[i],14);
			if (a == points)
			{
				cards[i] = 0;
			}
			else
			{
				tmp[j] = cards[i];
				j++;
			}
		}
	}
	else if (type > 0)
	{
		for (i=0;i<(*max);i++)
		{
			b = get_color(cards[i]);
			if (b == type)
			{
				cards[i] = 0;
			}
			else
			{
				tmp[j] = cards[i];
				j++;
			}
		}
	}
	else
	{
		return;
	}

	*max = j;
	for (i=0;i<j;i++)
	{
		cards[i] = tmp[i];
	}
}

//获取指定类型的三张牌
void getThreeCardsByType(int type,short int *cards,short int *max,short int *val)
{
	int i,j;
	short int tmp,a1,a2,b1,b2;
	
	if (type < 5)
	{
		val[0] = get_one(cards,max);
	}
	else if (type == 5)
	{
		//豹子
		//cutCards(cards,max,14,-1);
		//val[0] = get_one(cards,max);
		val[0] = (myRandom()%4)*14 + myRandom()%12 + 2;
	}
	else if (type == 6)
	{
		//AAA
		val[0] = (myRandom()%4+1)*14;
	}
	a1 = get_point(val[0],14);
	a2 = get_color(val[0]);
	switch (type)
	{
		case 0:
			//单牌
			cutCards(cards,max,a1,-1);
			val[1] = get_one(cards,max);
			b1 = get_point(val[1],14);
			b2 = get_color(val[1]);
			if (a2 == b2)
			{
				//花色一样,干掉这一花色
				cutCards(cards,max,-1,a2);
			}
			cutCards(cards,max,b1,-1);
			//点数可连
			if (a1 == b1+1)
			{
				if (a1 == 14)
				{
					cutCards(cards,max,b1-1,-1);
				}
				else if (b1 == 2)
				{
					cutCards(cards,max,a1+1,-1);
				}
				else 
				{
					cutCards(cards,max,b1-1,-1);
					cutCards(cards,max,a1+1,-1);
				}
			}
			else if (a1 == b1-1)
			{
				if (b1 == 14)
				{
					cutCards(cards,max,a1-1,-1);
				}
				else if (a1 == 2)
				{
					cutCards(cards,max,b1+1,-1);
				}
				else 
				{
					cutCards(cards,max,a1-1,-1);
					cutCards(cards,max,b1+1,-1);
				}
			}
			else if (a1 == b1+2)
			{
				cutCards(cards,max,a1-1,-1);
			}
			else if (a1 == b1-2)
			{
				cutCards(cards,max,a1+1,-1);
			}
			val[2] = get_one(cards,max);
			break;
		case 1:
			//对子
			val[1] = get_one(cards,max);
			b1 = get_point(val[1],14);
			if (a1 == b1)
			{
				cutCards(cards,max,a1,-1);
			}
			else
			{
				for (i=2;i<15;i++)
				{
					if (i != a1 && i != b1)
					{
						cutCards(cards,max,(short)i,-1);
					}
				}
			}
			val[2] = get_one(cards,max);
			break;
		case 2:
			//顺子
			if (a1 == 13)
			{
				//生成了k
				val[1] = (myRandom()%4)*14 + a1 - 1;
				b2 = get_color(val[1]);
				if (a2 == b2)
				{
					val[2] = ((a2+myRandom()%3)%4)*14 + a1 - 2;
				}
				else
				{
					val[2] = (myRandom()%4)*14 + a1 - 2;
				}
				
			}
			else if (a1 == 14)
			{
				//生成了A
				val[1] = (myRandom()%4)*14 + a1 - 12;
				b2 = get_color(val[1]);
				if (a2 == b2)
				{
					val[2] = ((a2+myRandom()%3)%4)*14 + a1 - 11;
				}
				else
				{
					val[2] = (myRandom()%4)*14 + a1 - 11;
				}
				
			}
			else
			{
				val[1] = (myRandom()%4)*14 + a1 + 1;
				b2 = get_color(val[1]);
				if (a2 == b2)
				{
					val[2] = ((a2+myRandom()%3)%4)*14 + a1 + 2;
				}
				else 
				{
					val[2] = (myRandom()%4)*14 + a1 + 2;
				}
				
			}

			break;
		case 3:
			//金花
			for (i=1;i<=4;i++)
			{
				if (i != a2)
				{
					cutCards(cards,max,-1,(short)i);
				}
			}
			val[1] = get_one(cards,max);
			b1 = get_point(val[1],14);
			//点数可连
			if (a1 == b1+1)
			{
				//a1 大
				if (a1 == 14)
				{
					cutCards(cards,max,b1-1,-1);
				}
				else if (b1 == 2)
				{
					cutCards(cards,max,a1+1,-1);
				}
				else 
				{
					cutCards(cards,max,b1-1,-1);
					cutCards(cards,max,a1+1,-1);
				}
			}
			else if (a1 == b1-1)
			{
				//a1 小
				if (b1 == 14)
				{
					cutCards(cards,max,a1-1,-1);
				}
				else if (a1 == 2)
				{
					cutCards(cards,max,b1+1,-1);
				}
				else 
				{
					cutCards(cards,max,a1-1,-1);
					cutCards(cards,max,b1+1,-1);
				}
			}
			else if (a1 == b1+2)
			{
				//a1 大
				cutCards(cards,max,a1-1,-1);
			}
			else if (a1 == b1-2)
			{
				//a1 小
				cutCards(cards,max,a1+1,-1);
			}
			val[2] = get_one(cards,max);
			break;
		case 4:
			//顺金
			if (a1 == 13)
			{
				//生成了k
				val[1] = val[0]-1;
				val[2] = val[0]-2;
			}
			else if (a1 == 14)
			{
				//生成了A
				val[1] = val[0]-12;
				val[2] = val[0]-11;
			}
			else
			{
				val[1] = val[0]+1;
				val[2] = val[0]+2;
			}
			break;
		case 5:
			//豹子
			j = 0;
			for (i=1;i<5;i++)
			{
				if (i != a2)
				{
					val[j] = (i-1)*14 + a1;
					j++;
				}	
			}
			break;
		case 6:
			//AAA
			j = 0;
			for (i=1;i<5;i++)
			{
				if (i != a2)
				{
					val[j] = i*14;
					j++;
				}	
			}
			break;
		default:
			//单牌
			cutCards(cards,max,a1,-1);
			val[1] = get_one(cards,max);
			b1 = get_point(val[1],14);
			b2 = get_color(val[1]);
			if (a2 == b2)
			{
				//花色一样,干掉这一花色
				cutCards(cards,max,-1,a2);
			}
			cutCards(cards,max,b1,-1);
			//点数可连
			if (a1 == b1+1)
			{
				if (a1 == 14)
				{
					cutCards(cards,max,b1-1,-1);
				}
				else if (b1 == 2)
				{
					cutCards(cards,max,a1+1,-1);
				}
				else 
				{
					cutCards(cards,max,b1-1,-1);
					cutCards(cards,max,a1+1,-1);
				}
			}
			else if (a1 == b1-1)
			{
				if (b1 == 14)
				{
					cutCards(cards,max,a1-1,-1);
				}
				else if (a1 == 2)
				{
					cutCards(cards,max,b1+1,-1);
				}
				else 
				{
					cutCards(cards,max,a1-1,-1);
					cutCards(cards,max,b1+1,-1);
				}
			}
			else if (a1 == b1+2)
			{
				cutCards(cards,max,a1-1,-1);
			}
			else if (a1 == b1-2)
			{
				cutCards(cards,max,a1+1,-1);
			}
			val[2] = get_one(cards,max);
			break;
	}
	//hands 生成完毕
	//冒泡倒序
	for (i=0;i<2;i++)
	{
		for (j=0;j<2-i;j++)
		{
			if (get_point(val[j],14) < get_point(val[j+1],14))
			{
				tmp = val[j];
				val[j] = val[j+1];
				val[j+1] = tmp;
			}
			else if (get_point(val[j],14) == get_point(val[j+1],14) && val[j] < val[j+1])
			{
				tmp = val[j];
				val[j] = val[j+1];
				val[j+1] = tmp;
			}
		}
	}
}

//获取N张点数倒序的牌
void getMoreCards(short int flag,short int num,short int *cards,short int *max,short int *val)
{
	short int ap,bp;
	int i,j,tmp;
	for (i=0;i<num;i++)
	{
		val[i] = get_one(cards,max);
	}
	//冒泡倒序
	for (i=0;i<num-1;i++)
	{
		for (j=0;j<num-1-i;j++)
		{

			if (flag == 0)
			{
				//0
				ap = get_point(val[j],14);
				bp = get_point(val[j+1],14);
			}
			else if (flag == 1)
			{
				//1
				ap = get_point(val[j],1);
				bp = get_point(val[j+1],1);
			}
			else 
			{
				//2 ...
				ap = get_point(val[j],10);
				bp = get_point(val[j+1],10);
			}


			if (ap < bp)
			{
				tmp = val[j];
				val[j] = val[j+1];
				val[j+1] = tmp;
			}
			else if (ap == bp && val[j] < val[j+1])
			{
				tmp = val[j];
				val[j] = val[j+1];
				val[j+1] = tmp;
			}
		}
	}
}

int cards_pk_ttz(short int *a,short int *b)
{
	short int type_a,type_b,a1,a2,b1,b2,a_total,b_total;
	type_a = check_type_ttz(a);
	type_b = check_type_ttz(b);
	//printf("-------------ttz pk start------------\n");
	//printf("type_a=%d,type_b=%d\n",type_a,type_b);
	if (type_a > type_b)
	{
		//a大
		return 1;
	}
	else if (type_a < type_b)
	{
		//b大
		return 2;
	}
	//相等情况

	a1 = get_point(a[0],10);
	a2 = get_point(a[1],10);
	b1 = get_point(b[0],10);
	b2 = get_point(b[1],10);
	//printf("a1=%d,a2=%d,b1=%d,b2=%d\n",a1,a2,b1,b2);
	if (type_a == 4 || type_a == 2)
	{
		return 1;
	}
	else if (type_a == 3)
	{
		if (a1 >= b1)
		{
			return 1;
		}
		else 
		{
			return 2;
		}
	}
	else
	{
		a_total = (a1+a2)%100; 
		b_total = (b1+b2)%100;

		//printf("a_total=%d,b_total=%d\n",a_total,b_total);
		if (a_total > b_total)
		{
			return 1;
		}
		else if (a_total < b_total)
		{
			return 2;
		}

		//相等情况
		if (a1 >= b1)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

//1:单牌 2:28 3:豹子 4:双中
short int check_type_ttz(short int *card)
{
	short int ap,bp;
	ap = get_point(card[0],10);
	bp = get_point(card[1],10);

	if (ap == 5 && bp == 5)
	{
		return 4;
	}
	else if (ap == bp)
	{
		return 3;
	}
	else if ((ap == 20 && bp == 80) || (ap == 80 && bp == 20))
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

//0：无牛，1-9：牛1-牛9，10：牛牛，11：五花牛，12：顺子牛，13：同花牛，14：葫芦牛，15：同花顺，16：炸弹牛，17：五小牛
short int check_type_ox(short int *card,short int model,short int kan)
{
	//传参已排序 a>b>c>d>e
	int i,count=0,j=0,k=0,l=0,z;
	int ishun=0,ihua=0,max=0;
	short int points[5];//扑克点数
	short int color[5];	//扑克花色
	short int value[5];	//扑克牛值
	
	for (i=0;i<5;i++)
	{
		points[i] = get_point(card[i],1);
		color[i] = get_color(card[i]);
		value[i] = points[i]>10?10:points[i];
	}
	
	for (i=0;i<5;i++)
	{
		count += value[i];
	}

	if (count < 10 && points[0] < 5)
	{
		return 17;
	}
	
	for (i=0;i<5;i++)
	{
		if (points[i] == points[0])
		{
			j++;
		}

		if (points[i] == points[4])
		{
			k++;
		}

		if (points[i] == points[1])
		{
			l++;
		}
	}

	if (j == 4 || k == 4)
	{
		return 16;
	}

	//model
	if (model == 1)
	{
		//特殊牌型判断
		if ((points[0] == points[1]+1 && points[1] == points[2]+1 && points[2] == points[3]+1 && points[3] == points[4]+1) \
			|| (points[0] == 13 && points[1] == 12 && points[2] == 11 && points[3] == 10 && points[4] == 1))
		{
			ishun = 1;
		}

		if (color[0] == color[1] && color[1] == color[2] && color[2] == color[3] && color[3] == color[4])
		{
			ihua = 1;
		}

		if (ishun == 1 && ihua == 1)
		{
			return 15;
		}
		else if (ishun == 1)
		{
			return 12;
		}
		else if (ihua == 1)
		{
			return 13;
		}

		if (j == 3 && points[3] == points[4])
		{
			return 14;
		}

		if (k == 3 && points[0] == points[1])
		{
			return 14;
		}
	}

	if (count == 50)
	{
		for (i=0;i<5;i++)
		{
			if (points[i] == 10)
			{
				return 10;
			}
		}
		return 11;
	}

	if (kan == 1)
	{
		//坎顺斗
		if (j == 3)
		{
			max = (value[3] + value[4])%10;
			if (max == 0)
			{
				return 10;
			}
		}
		else if (k == 3)
		{
			max = (value[0] + value[1])%10;
			if (max == 0)
			{
				return 10;
			}
		}
		else if (l == 3)
		{
			max = (value[0] + value[4])%10;
			if (max == 0)
			{
				return 10;
			}
		}

		for (i=0;i<5;i++)
		{
			for (j=i+1;j<5;j++)
			{
				for (k=j+1;k<5;k++)
				{
					if ((points[i] == points[j]+1 && points[j] == points[k]+1) || (points[i] == 13 && points[j] == 12 && points[k] == 1))
					{
						//顺子
						count = 0;
						for (l=0;l<5;l++)
						{
							if (l != i && l != j && l != k)
							{
								count += value[l];
							}
						}

						count %= 10;
						if (count == 0)
						{
							return 10;
						}

						if (count > max)
						{
							max = count;
						}

					}
				}
			}
		}
		//
	}

	for (i=0;i<5;i++)
	for (j=0;j<5;j++)
	for (k=0;k<5;k++)
	{
		if (i != j && i != k && j != k)
		{
			if ((value[i] + value[j] + value[k])%10 == 0)
			{
				//有牛
				count = 0;
				for (z=0;z<5;z++)
				{
					if (z != i && z != j && z != k)
					{
						count += value[z];
					}
				}
				count %= 10;

				if (count == 0)
				{
					return 10;
				}

				if (count > max)
				{
					return count;
				}
				else
				{
					return max;
				}
			}
		}
	}
	
	//无牛
	return max;
}
//牛牛PK
//0：无牛，1-9：牛1-牛9，10：牛牛，11：五花牛，12：顺子牛，13：同花牛，14：葫芦牛，15：同花顺，16：炸弹牛，17：五小牛
int cards_pk_ox(short int *a,short int *b,short int type_a,short int type_b)
{
	short int point_a,point_b;
	short int a0,a1,b0,b1;
	short int ac,bc;
	if (type_a > type_b)
	{
		//a大
		return 1;
	}
	else if (type_a < type_b)
	{
		//b大
		return 2;
	}
	
	switch (type_a)
	{
		case 16:
			//炸弹
			a0 = get_point(a[0],14);
			a1 = get_point(a[1],14);
			if (a0 != a1)
			{
				a0 = a1;
			}
			b0 = get_point(b[0],14);
			b1 = get_point(b[1],14);
			if (b0 != b1)
			{
				b0 = b1;
			}
			
			if (a0 > b0)
			{
				return 1;
			}
			else
			{
				return 2;
			}
		case 15:
			//同花顺
		case 12:
			//顺子牛
			a0 = get_point(a[0],14);
			a1 = get_point(a[4],14);
			ac = get_color(a[0]);

			b0 = get_point(b[0],14);
			b1 = get_point(b[4],14);
			bc = get_color(b[0]);

			if (a0 > b0)
			{
				return 1;
			}
			else if (a0 < b0)
			{
				return 2;
			}

			if (a1 > b1)
			{
				return 1;
			}
			else if (a1 < b1)
			{
				return 2;
			}

			if (ac > bc)
			{
				return 1;
			}
			else 
			{
				return 2;
			}

		case 14:
			//葫芦牛
			a0 = get_point(a[1],14);
			a1 = get_point(a[2],14);
			if (a0 != a1)
			{
				a0 = a1;
			}
			
			b0 = get_point(b[1],14);
			b1 = get_point(b[2],14);
			if (b0 != b1)
			{
				b0 = b1;
			}
			
			if (a0 > b0)
			{
				return 1;
			}
			else
			{
				return 2;
			}
	}

	point_a = get_point(a[0],1);
	point_b = get_point(b[0],1);
	if (point_a > point_b || (point_a == point_b && a[0] > b[0]))
	{
		return 1;
	}
	else 
	{
		return 2;
	}
}
//牛牛翻倍
int get_times_by_type(short int type,short int model)
{
	if (model == 1)
	{
		//激情模式
		if (type == 0)
		{
			return 1;
		}
		else if (type < 7)
		{
			return 2;
		}
		else if (type < 9)
		{
			return 3;
		}
		else if (type < 10)
		{
			return 4;
		}
		else if (type == 10)
		{
			return 5;
		}
		else if (type < 13)
		{
			return 6;
		}
		else if (type == 13)
		{
			return 7;
		}
		else if (type == 14)
		{
			return 8;
		}
		else if (type < 16)
		{
			return 9;
		}
		else
		{
			return 10;
		}
	}

	if (type < 7)
	{
		return 1;
	}
	else if (type < 9)
	{
		return 2;
	}
	else if (type < 10)
	{
		return 3;
	}
	else if (type == 10)
	{
		return 4;
	}
	else
	{
		return 5;
	}
}

//7：三A，6：豹子，5：顺金，4：金花，3：顺子，2：对子，1：单牌
int check_type(short int a,short int b,short int c)
{
	//传参已排序 a>b>c
	short int a1,a2,b1,b2,c1,c2;
	a1 = get_point(a,14);
	a2 = get_color(a);
	b1 = get_point(b,14);
	b2 = get_color(b);
	c1 = get_point(c,14);
	c2 = get_color(c);
	if (a1 == 14 && b1 == 14 && c1 == 14)
	{
		//三A
		return 7;
	}
	else if (a1 == b1 && b1 == c1)
	{
		//豹子
		return 6;
	}
	else if ((a1==b1 && b1!=c1)||(a1==c1 && b1!=c1)||(b1==c1 && b1!=a1))
	{
		//对子
		return 2;
	}
	else if ((a1-1==b1 && b1-1==c1) || (a1 == 14 && b1 == 3 && c1 == 2))
	{
		if (a2==b2 && b2==c2)
		{
			//顺金
			return 5;
		}
		else
		{
			//顺子
			return 3;
		}
	}
	else if (a2==b2 && b2==c2)
	{
		//金花
		return 4;
	}
	else
	{
		//单牌
		return 1;
	}
}

//return 1：a大，2：b大，-1：error
int cards_pk(short int *a,short int *b)
{
	int type_a,type_b;
	type_a = check_type(a[0],a[1],a[2]);
	type_b = check_type(b[0],b[1],b[2]);

	if (type_a > type_b)
	{
		//a大
		return 1;
	}
	else if (type_a < type_b)
	{
		//b大
		return 2;
	}

	//type一样
	int a1_x,a1_y,a2_x,a2_y,a3_x,a3_y,b1_x,b1_y,b2_x,b2_y,b3_x;
	int aa_x,a_x,bb_x,b_x,aa1_y,aa2_y;
	a1_x = get_point(a[0],14);
	a1_y = get_color(a[0]);
	a2_x = get_point(a[1],14);
	a2_y = get_color(a[1]);
	a3_x = get_point(a[2],14);
	a3_y = get_color(a[2]);
	b1_x = get_point(b[0],14);
	b1_y = get_color(b[0]);
	b2_x = get_point(b[1],14);
	b2_y = get_color(b[1]);
	b3_x = get_point(b[2],14);
	switch (type_a)
	{
		case 1:
			//单牌 与 金花判断规则一样
		case 4:
			//金花
			if (a1_x > b1_x)
			{
				return 1;
			}
			else if (a1_x < b1_x)
			{
				return 2;
			}

			if (a2_x > b2_x)
			{
				return 1;
			}
			else if (a2_x < b2_x)
			{
				return 2;
			}

			if (a3_x > b3_x)
			{
				return 1;
			}
			else if (a3_x < b3_x)
			{
				return 2;
			}

			if (a1_y > b1_y)
			{
				return 1;
			}
			else
			{
				return 2;
			}

		case 2:
			//对子
			a1_x==a2_x?(aa_x=a1_x,aa1_y=a1_y,aa2_y=a2_y,a_x=a3_x):(a1_x==a3_x?(aa_x=a1_x,aa1_y=a1_y,aa2_y=a3_y,a_x=a2_x):(aa_x=a3_x,aa1_y=a3_y,aa2_y=a2_y,a_x=a1_x));
			b1_x==b2_x?(bb_x=b1_x,b_x=b3_x):(b1_x==b3_x?(bb_x=b1_x,b_x=b2_x):(bb_x=b3_x,b_x=b1_x));
			if (aa_x > bb_x)
			{
				return 1;
			}
			else if (aa_x < bb_x)
			{
				return 2;
			}

			if (a_x > b_x)
			{
				return 1;
			}
			else if (a_x < b_x)
			{
				return 2;
			}
			
			if (aa1_y==4 || aa2_y==4)
			{
				return 1;
			}
			else
			{
				return 2;
			}

		case 3:
			//顺子 与 顺金判断规则一样
		case 5:
			//顺金
			if (a1_x == 14 && a2_x == 3)
			{
				a1_x = a2_x;
				a1_y = a2_y;
			}

			if (b1_x == 14 && b2_x == 3)
			{
				b1_x = b2_x;
				b1_y = b2_y;
			}

			if (a1_x > b1_x)
			{
				return 1;
			}
			else if (a1_x < b1_x)
			{
				return 2;
			}

			if (a1_y > b1_y)
			{
				return 1;
			}
			else
			{
				return 2;
			}
			
		case 6:
			//豹子
			if (a1_x > b1_x)
			{
				return 1;
			}
			else
			{
				return 2;
			}

		default:
			return -1;
	}
}

void print_card(short int *card)
{
	int i;
	for (i=0;i<3;i++)
	{
		switch (get_color(card[i]))
		{
			case 1:
				printf("%d=>方块%d,",card[i],get_point(card[i],14));
				break;
			case 2:
				printf("%d=>梅花%d,",card[i],get_point(card[i],14));
				break;
			case 3:
				printf("%d=>红桃%d,",card[i],get_point(card[i],14));
				break;
			case 4:
				printf("%d=>黑桃%d,",card[i],get_point(card[i],14));
				break;
			default:
				printf("color error!\n");
				return;
		}
	}

	switch (check_type(card[0],card[1],card[2]))
	{
		case 1:
			printf("单牌");
			break;
		case 2:
			printf("对子");
			break;
		case 3:
			printf("顺子");
			break;
		case 4:
			printf("金花");
			break;
		case 5:
			printf("顺金");
			break;
		case 6:
			printf("豹子");
			break;
		case 7:
			printf("AAA");
			break;
		default:
			printf("type error\n");
			return;
	
	}
	printf("\n");
}
#ifdef DEBUG_CARDS_H_ 
#define CARD_MAX 32
#define SEAT_MAX 10

void create_jh(short int *cards,short int *length,short int last[][3])
{
	int i,j,seat,max=0,x,y;
	srand((int)time(NULL));
	for (i=0;i<SEAT_MAX;i++)
	{
		getMoreCards(0,3,cards,length,last[i]);
	}
}

int main()
{
	printf("\n");
	int i,j,king;
	short int cards[CARD_MAX],last[SEAT_MAX][3];
	short int max = CARD_MAX;
	initCards(cards,CARD_MAX);
	create_jh(cards,&max,last);
	return 0;
}
#endif
