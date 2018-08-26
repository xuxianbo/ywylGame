<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 16:27
 */

namespace User\Model;

use Think\Exception;

class CodeRedis extends BaseRedis
{
    /**
     * 获取redis键值
     * @param $key
     * @return bool|mixed
     */
    public function getKey($key){
        $redis_key = [
            1 => 'code_',   //ip请求发送短信次数
            2 => 'phone_',   //ip请求发送短信次数
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
    public function setRedis($key,$value,$time = "",$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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
     * redis自增
     * @param $key
     * @param $step
     * @param string $ip
     * @param string $phone
     * @param string $scene
     * @return bool
     */
    public function incRedis($key,$step = 1,$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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
     * redis自减少
     * @param $key
     * @param $step
     * @param string $ip
     * @param string $phone
     * @param string $scene
     * @return bool
     */
    public function decRedis($key,$step = 1,$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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
     * 读取redis
     * @param $key
     * @param string $uid
     * @param string $user_token
     * @return bool|mixed
     */
    public function getRedis($key,$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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
     * 获取redis过期时间
     * @param $key
     * @param string $ip
     * @param string $phone
     * @param string $scene
     * @return bool|int
     */
    public function getRedisTTL($key,$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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

    /**
     * 删除redis
     * @param $key
     * @param string $uid
     * @param string $user_token
     * @return bool
     */
    public function delRedis($key,$ip = "",$phone = "",$scene = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false){
            return false;
        }
        if(!empty($ip)){
            $redis_key = $redis_key.$ip;
        }
        if(!empty($phone)){
            $redis_key = $redis_key.$phone;
        }
        if(!empty($scene)){
            $redis_key = $redis_key.'_'.$scene;
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
}