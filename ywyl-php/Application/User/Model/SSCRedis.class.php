<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/24
 * Time: 13:22
 */

namespace User\Model;


use Think\Exception;

class SSCRedis extends BaseRedis
{
    /**
     * 获取键值
     * @param $key    rediskey值对应号码
     */
    public function getSscKey($key){
        $sscRedisKey = [
            1 => 'ssc_history_list_check', //历史牌型
            2 => 'ssc_bz_time', //上次豹子牌型的时间
            3 => 'ssc_', //时时彩的牌型和牌号
            4 => 'ssc_win_player', //时时彩赢家记录
            5 => 'ssc_rank', //时时彩排行榜
            6 => 'ssc_rank_aware',  //时时彩大奖榜
            7 => 'ssc_bz_', //时时彩豹子下注总金额
            8 => 'ssc_sj_', //时时彩顺金下注总金额
            9 => 'ssc_jh_', //时时彩金花下注总金额
            10 => 'ssc_sz_', //时时彩顺子下注总金额
            11 => 'ssc_dz_', //时时彩对子下注总金额
            12 => 'ssc_dp_', //时时彩单牌下注总金额
            13 => 'ssc_balance_flag', //时时彩结算状态
        ];
        if(!isset($sscRedisKey[$key])){
            return false;
        }
        return $sscRedisKey[$key];
    }

    /**
     * 写入redis
     * @param $key
     * @param string $time
     * @param string $qh
     * @param string $uid
     */
    public function setSscRedis($key,$value,$time = "",$qh = "",$uid = ""){
        $redis_key = $this->getSscKey($key);
        if($redis_key == -1){
            return false;
        }
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }

        if(empty($time)){
            $time = C('REDIS_TIMEOUT');
        }else if($time == 'x' || $time == 'X'){
            $time = '';
        }

        try{
            $redis = $this->redisConnect();
            $redis->set($redis_key,$value,$time);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }

        return true;
    }

    /**
     * 获取redis
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function getSscRedis($key,$qh = "", $uid = ""){
        $redis_key = $this->getSscKey($key);
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }

        try{
            $redis = $this->redisConnect();
            $res = $redis->get($redis_key);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }

        if(!$res || $res < 0){
            return 0;
        }
        return $res;
    }

    /**
     * 删除redis
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function delSscRedis($key,$qh = "", $uid = ""){
        $redis_key = $this->getSscKey($key);
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }

        try{
            $redis = $this->redisConnect();
            $redis->rm($redis_key);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }

        return true;
    }

    /**
     * redis自增
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function incSscRedis($key,$qh = "", $uid = "",$step = 1){
        $redis_key = $this->getSscKey($key);
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }

        try{
            $redis = $this->redisConnect();
            $redis->inc($redis_key,$step);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * redis自减
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function decSscRedis($key,$qh = "", $uid = "",$step = 1){
        $redis_key = $this->getSscKey($key);
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }
        try{
            $redis = $this->redisConnect();
            $redis->dec($redis_key,$step);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 获取redis过期时间
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function ttlSscRedis($key,$qh = "", $uid = ""){
        $redis_key = $this->getSscKey($key);
        if(!empty($qh)){
            $redis_key = $redis_key.$qh;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }

        try{
            $redis = $this->redisConnect();
            $res = $redis->ttl($redis_key);
            $this->close_redis($redis);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }

        if(!$res || $res < 0){
            return 0;
        }
        return $res;
    }
}