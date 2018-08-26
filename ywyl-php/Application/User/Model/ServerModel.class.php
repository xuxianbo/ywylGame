<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 20:52
 */

namespace User\Model;


use Think\Exception;

class ServerModel extends BaseModel
{
    /**
     * 获取服务器信息
     * @param $user_token
     * @return bool
     */
    public function getInfo(){
        $sql = "select ip,port,player_num_online,player_num_max from config_server_info where status < 10";
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($info)){
            return 0;
        }
        return $info;
    }
}