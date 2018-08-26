<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 14:40
 */

namespace User\Model;


use Think\Exception;

class GameRedis extends BaseRedis
{
    /**
     * 获取redis键值
     * @param $key
     * @return bool|mixed
     */
    public function getKey($key){
        $redis_key = [
            1 => 'win_gold_rank_info_',   //键值为用户token   玩家uid
        ];
        if(!isset($redis_key[$key]))
        {
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
    public function setRedis($key,$value,$game_id = "",$time = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false)
        {
            return false;
        }
        if(!empty($game_id))
        {
            $redis_key = $redis_key.$game_id;
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
    public function getRedis($key,$game_id = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false)
        {
            return false;
        }
        if(!empty($game_id))
        {
            $redis_key = $redis_key.$game_id;
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
     * @param string $uid
     * @param string $user_token
     * @return bool
     */
    public function delRedis($key,$game_id = ""){
        $redis_key = $this->getKey($key);
        if($redis_key === false)
        {
            return false;
        }
        if(!empty($game_id))
        {
            $redis_key = $redis_key.$game_id;
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
}