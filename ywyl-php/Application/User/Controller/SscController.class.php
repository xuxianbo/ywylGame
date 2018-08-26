<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:45
 */

namespace User\Controller;

use User\Model\SSCModel;
use User\Model\SSCRedis;

class SscController extends BaseController
{
    //获取今日时时彩排行榜
    public function getSSCRank()
    {
        if(empty($user_token))
        {
            $user_token = I('param.user_token');
        }
        $this->getUserBaseByToken($user_token);
        $redisModel = new SSCRedis();
        $info = $redisModel->getSscRedis(5);
        if(!$info || $info == 0)
        {
            $sscModel = new SSCModel();
            $time = strtotime(date('Y-m-d'),time());
            $info = $sscModel->getGoldRank($time-1,$time+86400);
            if($info === false)
            {
                $info = 0;
            }
            else if(empty($info))
            {
                $info = 0;
            }
            else
            {
                $redisModel->setSscRedis(5,$info,C('ssc_rank_time'));
            }
        }

        $this->ajaxReturn(['status'=>'true','data'=>$info,'statusCode'=>1]);
    }

    //获取时时彩大奖榜
    public function getBaoziRank(){
        if(empty($user_token))
        {
            $user_token = I('param.user_token');
        }
        $this->getUserBaseByToken($user_token);
        $redisModel = new SSCRedis();
        $info = $redisModel->getSscRedis(6);
        if(!$info || $info == 0)
        {
            $sscModel = new SSCModel();
            $info = $sscModel->getAAAGoldRank();
            if($info === false)
            {
                $info = 0;
            }
            else if(empty($info))
            {
                $info = 0;
            }
            else
            {
                $redisModel->setSscRedis(6,$info,C('ssc_rank_time'));
            }
        }

        $this->ajaxReturn(['status'=>'true','data'=>$info,'statusCode'=>1]);
    }

    //获取上轮豹子时间
    public function getAAATime()
    {
        if(empty($user_token))
        {
            $user_token = I('param.user_token');
        }
        $this->getUserBaseByToken($user_token);
        $redisModel = new SSCRedis();
        $before_time = $redisModel->getSscRedis(2);
        if(!$before_time || $before_time == 0)
        {
            $sscModel = new SSCModel();
            $res = $sscModel->getBeforeBaoziTime();
            if($res === false)
            {
                $before_time = 0;
            }
            else if(empty($res))
            {
                $before_time = 0;
            }
            else
            {
                $before_time = $res[0]['create_time'];
                $redisModel->setSscRedis(2,$before_time,C('ssc_bz_time_time'));
            }
        }
        $res_data = [];
        $res_data['before_time'] = $before_time;
        $res_data['server_time'] = time();
        $res_data['baozi_time'] = time()-$before_time;
        $this->ajaxReturn(['status'=>'true','data'=>$res_data,'statusCode'=>1]);
    }

    //获取上轮最大赢家
    public function getLastWinPlay($user_token = "",$uid = "")
    {
        $this->getUserBaseByToken($user_token);
        $redisModel = new SSCRedis();
        $info = $redisModel->getSscRedis(4);
        if(!$info || $info == 0)
        {
            $sscModel = new SSCModel();
            $time = time()-C('ssc_time');
            $info = $sscModel->getLastWinPlayer($time);
            if($info === false)
            {
                $info = [];
            }
            else if($info == 0)
            {
                $info = [];
            }
            else
            {
                $redisModel->setSscRedis(4,$info,C('ssc_time'));
            }
        }

        $this->ajaxReturn(['status'=>'true','data'=>$info,'statusCode'=>1]);
    }
}