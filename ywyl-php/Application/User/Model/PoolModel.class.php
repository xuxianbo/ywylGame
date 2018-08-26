<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:35
 */

namespace User\Model;


use Think\Exception;

class PoolModel extends BaseModel
{
    //获取彩池数据根据id
    public function getInfoById($gid){
        $sql = "select * from game_pool where gid = $gid";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //更新彩池数据
    public function updateInfoByid($gid,$pool,$pool_win,$pool_water){
        $sql = "update game_pool set pool = pool+$pool,pool_win = pool_win+$pool_win,pool_water = pool_water+$pool_water where gid = $gid";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        return 1;
    }
}