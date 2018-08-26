<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 14:40
 */

namespace User\Model;

use Think\Cache\Driver\Redis;
use Think\Log;

class BaseRedis
{
    //链接redis
    public function redisConnect(){
        $redis = new Redis();
        $redis->connect('Redis',[
            'type'=>'Redis',
            'host'=>C('REDIS_HOST'),
            'port'=>C('REDIS_PORT'),
        ]);
        $redis->auth(C('REDIS_PASSWROD'));
        return $redis;
    }

    //错误日志
    protected function logError($e,$level){
        Log::write($e,$level);
        return true;
    }

    public function close_redis($redis){
        if(C('REDIS_PCONNECT')){
            $res = true;
        }else{
            $res = $redis->redis_close();
        }

        return $res;
    }
}