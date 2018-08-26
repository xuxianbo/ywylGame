<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/22
 * Time: 15:49
 */

namespace User\Model;


use Think\Exception;

class ReferralCodeModel extends BaseModel
{
    public function getInfoByCode($code){
        $time = time();
        $sql = "select * from referral_code where code = '$code' and out_time > $time";
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
}