<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 14:40
 */

namespace User\Model;


use Think\Exception;

class UserRedis extends BaseRedis
{
    /**
     * 获取redis键值
     * @param $key
     * @return bool|mixed
     */
    public function getKey($key){
        $redis_key = [
            1 => '',   //键值为用户token   玩家uid
            2 => 'user_base_',   //玩家基本信息
            3 => 'user_active_',   //玩家信息
            4 => 'gold_name_',   //玩家金币、姓名、vip等级
            5 => 'gold_rank',   //玩家金币排行榜
            6 => 'gold_get_rank',   //玩家昨日收入排行榜
            7 => 'gold_rank_user',   //用户查看的玩家金币排行榜
            8 => 'gold_get_rank_user',   //用户查看的玩家昨日收入排行榜
            9 => 'youke_regis_limit',   //游客注册ip限制
            10 => 'user_login_',   //用户手机号错误限制
            11 => 'user_ip_',   //用户手机号错误限制
        ];
        if(!isset($redis_key[$key])){
            return false;
        }
        return $redis_key[$key];
    }

    /**
     * 写入redis
     * @param $key
     * @param $value
     * @param string $time
     * @param string $uid
     * @param string $user_token
     * @return bool
     */
    public function setRedis($key,$value,$time = "",$uid = "",$user_token = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }
        if(!empty($user_token)){
            $redis_key = $redis_key.$user_token;
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
     * 读取redis
     * @param $key
     * @param string $uid
     * @param string $user_token
     * @return bool|mixed
     */
    public function getRedis($key,$uid = "",$user_token = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }
        if(!empty($user_token)){
            $redis_key = $redis_key.$user_token;
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
     * @param string $uid
     * @param string $user_token
     * @return bool
     */
    public function delRedis($key,$uid = "",$user_token = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }
        if(!empty($user_token)){
            $redis_key = $redis_key.$user_token;
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
     * 获取redis过期时间
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function ttlSscRedis($key,$uid = "",$user_token = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($uid)){
            $redis_key = $redis_key.$uid;
        }
        if(!empty($user_token)){
            $redis_key = $redis_key.$user_token;
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