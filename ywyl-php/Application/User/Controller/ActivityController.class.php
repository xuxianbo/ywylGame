<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/5/11
 * Time: 10:12
 */

namespace User\Controller;


use User\Model\ActivityModel;
use User\Model\OrderModel;

class ActivityController extends BaseController
{

    /**
     * 判断活动是否已经结束
     * @array $activity_info 活动信息
     */
    private function activityIsOver($activity_info,$time = "",$is_die = true)
    {
        if(empty($time))
        {
            $time = time();
        }

        //判断活动是否过期
        if(!is_array($activity_info))
        {
            $this->error_return(-11001005);
        }
        else if($activity_info['status'] == 0)
        {
            if($is_die === true)
            {
                $this->error_return(-11001002);
            }
            else
            {
                return 1;
            }
        }
        else if($time > $activity_info['end_time'] && $activity_info['end_time'] != 0)
        {
            if($is_die === true)
            {
                $this->error_return(-11001003);
            }
            else
            {
                return 2;
            }
        }
        else if($time < $activity_info['start_time'])
        {
            if($is_die === true)
            {
                $this->error_return(-11001004);
            }
            else
            {
                return 3;
            }
        }

        return 4;
    }

    /**
     * 获取幸运充值活动信息
     * @param string $user_token
     * @param string $uid
     */
    public function getLunckPayUserCount($user_token = "", $uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        //获取活动需求
        $activityModel = new ActivityModel();
        $activity_info = $activityModel->getInfoById(1);
        if($activity_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($activity_info))
        {
            $this->error_return(11001001);
        }
        else
        {
            $activity_info = $activity_info[0];
        }

        $time = time();

        //判断活动是否过期
        $now_status = $this->activityIsOver($activity_info,$time,false);

        $count = 0;
        //获取时间内的充值玩家数
        $orderModel = new OrderModel();
        $order_info = $orderModel->getCountByTime($activity_info['start_time'],$activity_info['end_time']);
        if($order_info === false)
        {
            $this->error_return(-1);
        }
        else if(!empty($order_info[0]['count_num']))
        {
            $count = $order_info[0]['count_num'];
        }

        $count = $count + 856;

        $this->success_return(['user_count' => $count, 'start_time' => date("Y.m.d",$activity_info['start_time']), 'end_time' => date("Y.m.d",$activity_info['end_time']), 'status' => $activity_info['status'], 'now_status' => $now_status],"获取成功");
    }
}