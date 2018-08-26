<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/22
 * Time: 19:28
 */

namespace User\Model;


use Think\Exception;

class RaffleModel extends BaseModel
{
    //获取奖励信息
    public function getAwardInfo(){
        $sql = "select id,award_desc,award_num,award_pro,award_type from config_raffle_award where status = 1";
        try
        {
            $res = $this->query($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }

    //获取总概率
    public function getProCount(){
        $sql = "select sum(award_pro) as award_pro_sum from config_raffle_award where status = 1";
        try
        {
            $res = $this->query($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }

    /**
     * 给出奖品
     * @param $raffle_info
     * @return int|string
     */
    public function getRandAward($raffle_info){
        $res = $this->getProCount();
        if($res === false)
        {
            return false;
        }
        else
        {
            $pro_max = $res[0]['award_pro_sum'];
        }

        $rand = rand(1,$pro_max);
        $pro_a = 0;
        $pro_b = 0;
        foreach ($raffle_info as $key => $val)
        {
            if($val['award_pro'] != 0)
            {
                $pro_a = $pro_a+$val['award_pro'];
                if($rand <= $pro_a && $rand > $pro_b)
                {
                    return $key;
                }
                $pro_b = $pro_a;
            }
        }
    }
}