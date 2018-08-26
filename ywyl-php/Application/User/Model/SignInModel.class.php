<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/3/20
 * Time: 11:11
 */

namespace User\Model;


use Think\Exception;

class SignInModel extends BaseModel
{

    //获取配置信息
    public function getInfo($id = ""){
        if($id == ""){
            $sql = "select id,give_points_num,give_gold_num,give_diamond_num from config_sign_in";
        }else{
            $sql = "select id,give_points_num,give_gold_num,give_diamond_num from config_sign_in where id = $id";
        }
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }
}