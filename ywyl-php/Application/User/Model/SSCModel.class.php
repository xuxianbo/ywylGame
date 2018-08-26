<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:39
 */

namespace User\Model;


use Think\Exception;
use Think\Log;

class SSCModel extends BaseModel
{
    //获取时时彩排行榜数据 根据金钱排序
    public function getGoldRank($start_time,$end_time){
        $limit = C('rank_num');
        $sql = "select a.*,b.name,b.pic_head from user_ssc_aaa a,user_active b where a.uid = b.uid and a.create_time > $start_time and a.create_time < $end_time order by a.gold_win desc limit $limit";
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $info;
    }

    //获取大奖榜
    public function getAAAGoldRank(){
        $limit = C('rank_num');
        $sql = "select a.*,b.name,b.pic_head from user_ssc_aaa a,user_active b where a.uid = b.uid and (a.check_type = 5 or a.check_type = 6) order by a.create_time desc limit $limit";
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $info;
    }

    /**
     * 上轮豹子时间
     * @return int|mixed|string
     */
    public function getBeforeBaoziTime(){
//        $sql = "select create_time from game_ssc_aaa where check_type = 5 or check_type = 6 order by create_time desc limit 1";
        $sql = "select create_time from game_ssc_aaa order by create_time desc limit 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }

    //获取上轮最大赢家信息
    public function getLastWinPlayer($time){
        $sql = "select a.*,b.name,b.pic_head from user_ssc_aaa a,user_active b where a.uid = b.uid and a.create_time > $time order by a.create_time desc limit 1";
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($info)){
            return 0;
        }
        $info = $info[0];
        return $info;
    }
}