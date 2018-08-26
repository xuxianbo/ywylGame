<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/5/24
 * Time: 10:52
 */
namespace Initias\Model;

use Think\Exception;
use User\Model\BaseRedis;

class InitiaRedis extends BaseRedis
{
    /**
     * 获取键值
     * @param $key    rediskey值对应号码
     */
    public function getKey($key){
        $redisKey = [
            'bot_bjl_rank', //百家乐机器人榜
            'bot_ssc_rank', //时时彩机器人榜
        ];

        if(!isset($redisKey[$key]))
        {
            return false;
        }

        return $redisKey[$key];
    }

    /**
     * 写入redis
     * @param $key
     * @param string $time
     * @param string $qh
     * @param string $uid
     */
    public function setInitiaRedis($key, $value, $time = "", $qh = ""){
        $redis_key = $this->getKey($key);
        if($redis_key == false)
        {
            return false;
        }
        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
        }

        if(empty($time))
        {
            $time = C('REDIS_TIMEOUT');
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
     * 获取redis
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function getInitiaRedis($key, $qh = ""){
        $redis_key = $this->getKey($key);

        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
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

    /**
     * 删除redis
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function delInitiaRedis($key, $qh = ""){
        $redis_key = $this->getKey($key);

        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
        }


        try
        {
            $redis = $this->redisConnect();
            $redis->rm($redis_key);
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
     * redis自增
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function incInitiaRedis($key, $qh = "", $step = 1){
        $redis_key = $this->getKey($key);

        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
        }

        try
        {
            $redis = $this->redisConnect();
            $redis->inc($redis_key,$step);
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
     * redis自减
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function decInitiaRedis($key, $qh = "", $step = 1){
        $redis_key = $this->getKey($key);
        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
        }

        try
        {
            $redis = $this->redisConnect();
            $redis->dec($redis_key,$step);
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
     * 获取redis过期时间
     * @param $key
     * @param string $qh
     * @param string $uid
     */
    public function ttlInitiaRedis($key, $qh = ""){
        $redis_key = $this->getKey($key);
        if(!empty($qh))
        {
            $redis_key = $redis_key.$qh;
        }


        try
        {
            $redis = $this->redisConnect();
            $res = $redis->ttl($redis_key);
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