<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/26
 * Time: 15:39
 */

namespace User\Model;


use Think\Exception;

class ExchangeCodeModel extends BaseModel
{
    //获取兑换码列表
    public function getInfoByCode($code){
        $time = time();
        $sql = "select * from exchange_code where code = '$code' and out_time > $time";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        return $res[0];
    }

    //用户使用兑换码
    public function updateReferById($referral_info,$uid){
        $id = $referral_info['id'];
        $sql = "update user_active a,exchange_code b set b.uid = a.uid,a.gold = a.gold+b.get_gold,a.diamond = a.diamond+b.get_diamond where a.uid = $uid and b.id = $id";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }
}