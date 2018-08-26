<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 14:33
 */

namespace Initias\Controller;


use Initias\Model\InitiaRedis;
use Think\Log;
use User\Controller\BaseController;
use User\Model\ByDayModel;
use User\Model\SSCModel;
use User\Model\SSCRedis;
use User\Model\UserActiveModel;
use User\Model\UserRedis;

class InitiaController extends BaseController
{
    /**
     * token验证
     * @param string $initia_token
     */
    protected function tokenVerify($initia_token = '')
    {
        $ip = $_SERVER["REMOTE_ADDR"];
        if(empty($initia_token)){
            Log::write("未传递admin_token访问该方法，IP地址为:$ip",'notice');
            $this->ajaxReturn(false);
        }
        else if($initia_token != C('INITIA_TOKEN'))
        {
            Log::write("未传递admin_token访问该方法，IP地址为:$ip",'notice');
            $this->ajaxReturn(false);
        }
        else
        {
            Log::write("ip:$ip 使用访问该方法",'info');
        }
    }

    /**
     * 每天零点重置用户需要重置的信息
     * @param string $Initia_token
     */
    public function initiaUserInfo($Initia_token = '')
    {
        $this->tokenVerify($Initia_token);

        $userActiveModel = new UserActiveModel();
        $res = $userActiveModel->initiaUserInfo();
        $this->ajaxReturn($res);
    }

    /**
     * 初始化排行榜
     * @param string $Initia_token
     */
    public function initiaRank($Initia_token = ''){
        $this->tokenVerify($Initia_token);

        $userActiveModel = new UserActiveModel();

        //获取财富榜
        $goldRank = $userActiveModel->getGoldRank();
        if($goldRank === false)
        {
            $this->ajaxReturn(['msg'=>'数据库错误','code'=>-1]);
        }
        else if(empty($goldRank))
        {
            $goldRank = '';
        }
        else
        {
            foreach ($goldRank as $k => $v)
            {
                if(empty($v['sign']))
                {
                    $goldRank[$k]['sign'] = 0;
                }
            }
        }

        //将数据存入redis
        $userBaseRedis = new UserRedis();
        $userBaseRedis->setRedis(5,$goldRank,310);

        $this->ajaxReturn(true);
    }

    /**
     * 初始化用户排行榜
     * @param string $Initia_token
     */
    public function initiaGetUserRank($Initia_token = ''){
        $this->tokenVerify($Initia_token);

        //将数据存入redis
        $userBaseRedis = new UserRedis();
        $goldRank = $userBaseRedis->getRedis(5);
        if($goldRank != false)
        {
            $userBaseRedis->setRedis(7,$goldRank,'x');
        }
        $this->ajaxReturn(true);
    }
}