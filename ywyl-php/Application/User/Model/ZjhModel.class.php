<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 14:47
 */

namespace User\Model;


use Think\Exception;

class ZjhModel extends BaseModel
{
//    //获取用户至今为止的炸金花普通玩法的记录信息
//    public function getUserZjhNorByDayInfoByUid($uid){
//        $sql = "select sum(gain) as gain_sum, sum(game_total) as game_total_sum,sum(game_win) as game_win_sum from user_zjh_normal_by_day where uid = $uid";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if (empty($info)){
//            return 0;
//        }
//        return $info[0];
//    }
//
//    //获取用户具体一天的炸金花普通玩法的记录信息
//    public function getUserZjhNorByDayInfoByUidTime($uid,$time){
//        $date = date('Ymd',$time);
//        $sql = "select gain,game_total,game_win from user_zjh_normal_by_day where uid = $uid and `date` = $date";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if (empty($info)){
//            return 0;
//        }
//        return $info[0];
//    }
}