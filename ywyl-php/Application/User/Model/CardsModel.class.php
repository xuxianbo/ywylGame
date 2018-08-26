<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/24
 * Time: 14:51
 */

namespace User\Model;


class CardsModel
{
    //获取开奖结果
    public function create_jh($ssc_id){
        $cardsModel = new Cards();
        $res = $cardsModel->check_type($ssc_id);
        return $res;
    }

    //获取牌型
    public function check_type($ssc_id){
        //获取时间基数
        $date = date('Ymd',time());
        ;
        $date = config('shishicai.srand_head').$date.config('shishicai.srand_bottom');
        srand($date);

        //获取牌型
        $id = substr($ssc_id,4,4);
        if($id == 0){
            return false;
        }
        for($i = 0; $i < $id; $i++){
            $type = rand(10000,100000000);
            $type = $type%10000;
        }

        //获取牌号
        $check_type = config("shishicai_check_type");
        foreach ($check_type as $k => $v){
            $interval = explode(',',$v);
            if($interval[0] < $type && $type <= $interval[1]){
                switch ($k){
                    case "Abaozi":
                        $check = 7;
                        break;
                    case "baozi":
                        $check = 6;
                        break;
                    case "shunjin":
                        $check = 5;
                        break;
                    case "jinhua":
                        $check = 4;
                        break;
                    case "shunzi":
                        $check = 3;
                        break;
                    case "duizi":
                        $check = 2;
                        break;
                    case "danpai":
                        $check = 1;
                        break;
                }
            }
        }

        //获取到牌
        $res['card'] = $this->cards_type($check);
        $res['card_type'] = $check;
        return $res;
    }

    //根据牌型随机获取到符合牌型的牌
    //牌型 6：豹子，5：顺金，4：金花，3：顺子，2：对子，1：单牌
    //花色 1：方块，2：梅花，3：红桃，4：黑桃
    public function cards_type($check){
        srand();
        //获取扑克牌 和扑克牌数量
        $puke = [2,3,4,5,6,7,8,9,10,11,12,13,14];

        //单牌
        if($check == 1){
            //抽第一张牌
            $rand_one = rand(0,12);
            $rand_one_hua = rand(1,4);//随机花色
            $res_card[0] = $puke[$rand_one] + ($rand_one_hua-1)*14;

            //丢弃第一张牌
            array_splice($puke,$rand_one,1);

            //抽第二张牌
            $rand_two = rand(0,11);
            $cardTwo = $puke[$rand_two];
            $rand = rand(1,4);
            $res_card[1] = $cardTwo + ($rand-1)*14;

            if($rand_two == 6){
                $rand_two = $rand_two-2;
            }
            if($rand_two != 0){
                $rand_two = $rand_two-1;
            }

            //丢弃第二张牌以及相邻牌
            array_splice($puke,$rand_two,3);

            //抽第三张牌
            $cardThree = rand(0,7);
            $cardThree = $puke[$cardThree];

            //选择第三张牌花色
            $j_3 = 0;
            for ($i = 1; $i <= 4; $i++){
                if($i != $rand_one_hua){
                    $hua[$j_3] = $cardThree + ($i-1)*14;
                    $j_3++;
                }
            }
            $rand_three_hua = rand(1,2);
            $res_card[2] = $hua[$rand_three_hua];

        }

        //对子
        if($check == 2){
            //抽第一张牌和第二张牌
            $rand_one = rand(0,12);
            $rand_one_hua = rand(1,4);//随机花色
            $res_card[0] = $puke[$rand_one] + ($rand_one_hua-1)*14;

            //获取牌的值和花色
            $cardOne =  $puke[$rand_one];
            $cardOneHua = $rand_one_hua;

            //选择第二张牌的花色
            $j_2 = 0;
            for ($i = 1; $i <= 4; $i++){
                if($i != $cardOneHua){
                    $hua[$j_2] = $cardOne + ($i-1)*14;
                    $j_2++;
                }
            }
            $rand_two_hua = rand(1,2);
            $res_card[1] = $hua[$rand_two_hua];
            $cardTwo = $res_card[1]%14?$res_card[1]%14:14;
            array_splice($puke,$rand_one,1);

            //抽取第三张牌
            $cardThree = rand(0,11);
            $cardThree = $puke[$cardThree];
            $rand_three_hua = rand(1,4);
            $res_card[2] = $cardThree + ($rand_three_hua-1)*14;
        }

        //顺子
        if($check == 3){
            //抽取第一张牌
            $rand = rand(2,12);
            $rand_one_hua = rand(1,4);//随机花色
            $res_card[0] = $puke[$rand] + ($rand_one_hua-1)*14;

            //获取牌的值和花色
            $cardOneHua = $rand_one_hua;

            //获取第二张牌
            $j_2 = 0;
            for ($i = 1; $i <= 4; $i++){
                if($i != $cardOneHua){
                    $hua[$j_2] = $puke[$rand-1] + ($i-1)*14;
                    $j_2++;
                }
            }
            $rand_two_hua = rand(1,2);
            $res_card[1] = $hua[$rand_two_hua];

            //获取第三张牌
            $rand_three_hua = rand(1,4);
            $res_card[2] = $puke[$rand-2] + ($rand_three_hua-1)*14;
        }

        //金花
        if($check == 4){
            //抽第一张牌
            $rand_one = rand(0,12);
            $rand_one_hua = rand(1,4);//随机花色
            $res_card[0] = $puke[$rand_one] + ($rand_one_hua-1)*14;

            //丢弃第一张牌
            array_splice($puke,$rand_one,1);

            //抽第二张牌
            $rand_two = rand(0,11);
            $cardTwo = $puke[$rand_two];
            $res_card[1] = $cardTwo + ($rand_one_hua-1)*14;

            //丢弃第二张牌以及相邻牌
            if($rand_two == 6){
                $rand_two = $rand_two-2;
            }
            if($rand_two != 0){
                $rand_two = $rand_two-1;
            }
            array_splice($puke,$rand_two,3);

            //抽第三张牌
            $cardThree = rand(0,8);
            $cardThree = $puke[$cardThree];
            $res_card[2] = $cardThree + ($rand_one_hua-1)*14;
        }

        //顺金
        if($check == 5){
            //抽取三张牌
            $rand = rand(2,12);
            $rand_one_hua = rand(1,4);//随机花色
            $res_card[0] = $puke[$rand] + ($rand_one_hua-1)*14;
            //获取第二张牌
            $res_card[1] = $puke[$rand-1] + ($rand_one_hua-1)*14;
            //获取第三张牌
            $res_card[2] = $puke[$rand-2] + ($rand_one_hua-1)*14;
        }

        //豹子
        if($check == 6){
            //获取基础牌值和被丢弃的花色
            $rand_one = rand(0,11);
            $rand_one_hua = rand(1,4);//随机花色

            //获取其他花色牌
            $j_2 = 0;
            for ($i = 1; $i <= 4; $i++){
                if($i != $rand_one_hua){
                    $card_3[] = $puke[$rand_one] + ($i-1)*14;
                    $j_2++;
                }
            }

            //获取三张牌
            $res_card[0] = $card_3[0];
            $res_card[1] = $card_3[1];
            $res_card[2] = $card_3[2];
        }

        //三个A
        if($check == 7){
            //获取基础牌值和被丢弃的花色
            $rand_one_hua = rand(1,4);//随机花色

            //获取其他花色牌
            $j_2 = 0;
            for ($i = 1; $i <= 4; $i++){
                if($i != $rand_one_hua){
                    $card_3[] = 14 + ($i-1)*14;
                    $j_2++;
                }
            }

            //获取三张牌
            $res_card[0] = $card_3[0];
            $res_card[1] = $card_3[1];
            $res_card[2] = $card_3[2];
        }

        rsort($res_card);
        if($res_card[0] == $res_card[1]){
            $res_card = $this->cards_type($check);
        }
        //从大到小排序
        $card = [];
        $a = ($res_card[0]%14 == 0)? 14 : $res_card[0]%14;
        $aa = $res_card[0];
        $b = ($res_card[1]%14 == 0)? 14 : $res_card[1]%14;
        $bb= $res_card[1];
        $c = ($res_card[2]%14 == 0)? 14 : $res_card[2]%14;
        $cc = $res_card[2];
        if($a < $b){
            $res_card[0] = $bb;
            $res_card[1] = $aa;
            $res_card[2] = $cc;
            if($b >= $c){
                if($a < $c){
                    $res_card[1] = $cc;
                    $res_card[2] = $aa;
                }
            }
            if($b < $c){
                $res_card[0] = $cc;
                $res_card[1] = $bb;
                $res_card[2] = $aa;
            }

        }else if($a < $c){
            $res_card[0] = $cc;
            $res_card[1] = $aa;
            $res_card[2] = $bb;
        }else{
            $res_card[0] = $aa;
            $res_card[1] = $bb;
            $res_card[2] = $cc;
            if($a >= $c){
                if($b < $c){
                    $res_card[1] = $cc;
                    $res_card[2] = $bb;
                }else{
                    $res_card[1] = $bb;
                    $res_card[2] = $cc;
                }

            }
            if($a < $c){
                $res_card[0] = $cc;
                $res_card[1] = $aa;
                $res_card[2] = $bb;
            }
        }
        return $res_card;
    }
}