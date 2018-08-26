<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/6/15
 * Time: 17:15
 */

namespace User\Controller;

use User\Model\UserActiveModel;
use User\Model\UserMailModel;

class MailController extends BaseController
{
    /**
     * 获取玩家的邮件
     * @param string $user_token
     * @param string $uid
     */
    public function getUserMail($user_token = "",$uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $time = time();
        $userMailModel = new UserMailModel();
        $list = $userMailModel->getList($uid,$time);
        if($list === false)
        {
            $this->error_return(-1);
        }
        else if(empty($list))
        {
            $list = [];
        }
        else
        {
            foreach ($list as $k => $v)
            {
                $list[$k]['create_time'] = date("Y-m-d",$v['create_time']);
            }
        }
        $this->success_return(['list' => $list]);
    }

    /**
     * 获取邮件内容
     * @param string $user_token
     * @param string $uid
     * @param $mail_id
     */
    public function getMailInfo($user_token = "",$uid = "",$mail_id = "")
    {
        if(empty($mail_id))
        {
            $this->error_return(-13001001);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $userMailModel = new UserMailModel();

        $time = time();
        //获取邮件内容
        $info = $userMailModel->getInfoById($mail_id,$time);
        if($info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($info))
        {
            $info = [];
        }
        else if($info[0]['uid'] != $uid)
        {
            $this->error_return(-13001002);
        }
        else
        {
            $info = $info[0];
        }

        //修改邮件状态
        if($info['status'] == 1)
        {
            $res = $userMailModel->lookStatus($mail_id);
            if($res === false)
            {
                $this->error_return(-1);
            }
            else
            {
                $info['status'] = 2;
            }
        }

        $info['create_time'] = date("Y-m-d",$info['create_time']);

        $this->success_return(['info' => $info]);
    }

    /**
     * 领取邮件附件
     * @param string $user_token
     * @param string $uid
     * @param string $mail_id
     */
    public function getMailGiving($user_token = "",$uid = "",$mail_id = "")
    {
        if(empty($mail_id))
        {
            $this->error_return(-13001001);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $userMailModel = new UserMailModel();

        $time = time();

        //获取邮件内容
        $info = $userMailModel->getInfoById($mail_id,$time);
        if($info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($info))
        {
            $this->error_return(-13001007);
        }
        else if($info[0]['uid'] != $uid)
        {
            $this->error_return(-13001002);
        }
        else
        {
            $info = $info[0];
            if($info['is_give'] == 1)
            {
                $this->error_return(-13001006);
            }
        }

        $userActiveModel = new UserActiveModel();

        $userMailModel->startTrans();

        //邮件加锁
        $info = $userMailModel->lockMailById($mail_id,$time);
        if($info === false)
        {
            $userMailModel->rollback();
            $this->error_return(-1);
        }
        else if(empty($info))
        {
            $userMailModel->rollback();
            $this->error_return(-13001007);
        }
        else if($info[0]['uid'] != $uid)
        {
            $userMailModel->rollback();
            $this->error_return(-13001002);
        }
        else
        {
            $info = $info[0];
            if($info['is_give'] == 1)
            {
                $userMailModel->rollback();
                $this->error_return(-13001006);
            }
        }

        //获取附件
        switch ($info['giving'])
        {
            case 0:
                $userMailModel->rollback();
                $this->error_return(-13001003);
                break;
            case 1:
                $res = $userActiveModel->updateGoldByUid($uid,$info['giving_num']);
                break;
            case 2:
                $res = $userActiveModel->updateMoneyByUid($uid,$info['giving_num']);
                break;
            default:
                $userMailModel->rollback();
                $this->error_return(-13001004);
                break;
        }
        if($res === false)
        {
            $userMailModel->rollback();
            $this->error_return(-1);
        }

        //修改邮件状态
        $res = $userMailModel->delMailGiv($mail_id);
        if($res === false)
        {
            $userMailModel->rollback();
            $this->error_return(-1);
        }

        $userMailModel->commit();

        //更新redis
        $user_active = $this->getUserActiveRedis($uid);

        $this->success_return(['giving' => $info['giving'], 'giving_num' => $info['giving_num'],'user_active' => $user_active]);
    }

    /**
     * 删除用户邮件
     * @param string $user_token
     * @param string $uid
     * @param string $mail_id
     * @param string $is_del
     */
    public function delUserMail($user_token = "",$uid = "",$mail_id = "",$is_del = "")
    {
        if(empty($mail_id))
        {
            $this->error_return(-13001001);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $userMailModel = new UserMailModel();

        $time = time();

        //获取邮件内容
        $info = $userMailModel->getInfoById($mail_id,$time);
        if($info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($info))
        {
            $info = [];
        }
        else if($info[0]['uid'] != $uid)
        {
            $this->error_return(-13001002);
        }
        else
        {
            $info = $info[0];
        }

        if($info['is_give'] != 2 && $is_del != 1)
        {
            $this->error_return(-13001005);
        }

        //删除邮件
        $res = $userMailModel->delMailById($mail_id);
        if($res === false)
        {
            $this->error_return(-1);
        }

        $this->success_return('',"删除成功");
    }
}