<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/6/27
 * Time: 10:13
 */

namespace User\Model;


use Think\Exception;

class OrderRedis extends BaseRedis
{
    /**
     * 获取redis键值
     * @param $key
     * @return bool|mixed
     */
    public function getKey($key){
        $redis_key = [
            1 => 'order_',   //order_order_id

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
    public function setRedis($key,$value,$time = "",$sign = ''){
        $redis_key = $this->getKey($key);
        if($redis_key === false)
        {
            return false;
        }
        if(!empty($sign))
        {
            $redis_key = $redis_key.$sign;
        }

        if(empty($time))
        {
            $time = C('REDIS_TIMEOUT');
        }
        else if($time == 'x' || $time == 'X')
        {
            $time = '';
        }

        try
        {
            $redis = $this->redisConnect();
            $redis->set($redis_key,$value,$time);
            $this->close_redis($redis);
        }
        catch (Exception $e)
        {
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
    public function getRedis($key,$sign = ''){
        $redis_key = $this->getKey($key);
        if($redis_key === false)
        {
            return false;
        }

        if(!empty($sign))
        {
            $redis_key = $redis_key.$sign;
        }
        try
        {
            $redis = $this->redisConnect();
            $res = $redis->get($redis_key);
            $this->close_redis($redis);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }

        if(!$res || $res < 0){
            return 0;
        }
        return $res;
    }
}