<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/22
 * Time: 14:44
 */

namespace User\Model;


use Think\Exception;

class VipModel extends BaseModel
{

    //获取Vip信息
    public function getInfo(){
        $sql = "select `level`,total_buy,bank_ratio,reffle_ratio,is_crehouse,is_inhouse from config_vip";
        try{
            $res = $this::query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取Vip信息根据等级
    public function getInfoByLevel($level){
        $sql = "select `level`,total_buy,bank_ratio,reffle_ratio,is_crehouse,is_inhouse,sign_in_ratio from config_vip where level = $level";
        try{
            $res = $this::query($sql);
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