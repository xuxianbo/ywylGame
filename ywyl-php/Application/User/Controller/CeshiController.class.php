<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/2/2
 * Time: 17:33
 */

namespace User\Controller;


use http\Env\Request;
use Think\Controller;
use Think\Log;
use User\Model\PayModel;
use User\Model\UserActiveModel;

class CeshiController extends Controller
{
//    public function bot_name(){
//        $admin_token = I('param.admin_token');
//        if($admin_token != "by_admin_token_123456"){
//            dump(false);
//        }
//        //将机器人的名字改为游客加id
//        $userActiveModel = new UserActiveModel();
//        $res = $userActiveModel->bot_name();
//        dump($res);
//    }

//    //将所有用户的名字改成游客加id取1W的余
//    public function set_name($admin_token = ""){
//        if($admin_token == ""){
//            $admin_token = I('param.admin_token');
//            if($admin_token != "by_admin_token_123456"){
//                dump(false);
//                exit;
//            }
//        }
//        $userActiveModel = new UserActiveModel();
//        $res = $userActiveModel->getInfo();
//        foreach ($res as $val){
//            $name = base64_encode("游客".$val['uid']%10000);
//            $name = "游客".rand(9900,9999);
//            $userActiveModel->updateName($val['uid'],$name);
//        }
//        dump($res);
//    }

//        public function set_bot_sex(){
//            $user_active = new UserActiveModel();
//            for($i = 9900;$i<10000;$i++){
//                $sex = $i%2;
//                $sql = "update user_active set sex = $sex,pic_head = $sex where uid = $i";
//                $res = $user_active->execute($sql);
//                if($res === false){
//                    echo 1;
//                    exit;
//                }
//            }
//        }
}