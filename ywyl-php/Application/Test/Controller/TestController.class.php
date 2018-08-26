<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/5/11
 * Time: 10:12
 */

namespace Test\Controller;


use Think\Log;
use User\Controller\BaseController;
use User\Model\UserBaseModel;

class TestController extends BaseController
{
    //支付测试
    private function test_pay($type,$order,$total_fee)
    {
        //获取支付配置
        $pay_config = C('lai_dian_fu_config');
        switch ($type)
        {
            case 1:
                $pay_config['paytype'] = 'alipaywap';
                break;
            case 2:
                $pay_config['paytype'] = 'wxh5';
                break;
            default:
                $this->ajaxReturn('false');
                break;
        }

        if(!empty($total_fee))
        {
            $pay_config['total_fee'] = $total_fee;
        }

        $pay_config['sdorderno'] = $order;
        //构建签名
        $pay_config['sign'] = md5('version='.$pay_config['version'].'&customerid='.$pay_config['customerid'].'&total_fee='.$pay_config['total_fee'].'&sdorderno='.$pay_config['sdorderno'].'&notifyurl='.$pay_config['notifyurl'].'&returnurl='.$pay_config['returnurl'].'&'.$pay_config['app_key']);

        $this->assign('pay_config',$pay_config);
        $this->display('table');
    }

    //同步返回测试
    private function test_returnurl()
    {
        $pay_config = C('lai_dian_fu_config');
        $data = $_GET;
        $sign =  md5('customerid='.$data['customerid'].'&status='.$data['status'].'&sdpayno='.$data['sdpayno'].'&sdorderno='.$data['sdorderno'].'&total_fee='.$data['total_fee'].'&paytype='.$data['paytype'].'&'.$pay_config['app_key']);
        if($sign == $data['sign'] && $data['status'] == 1)
        {
            $this->assign('status','验证成功');
        }
        else
        {
            $this->assign('status','验证失败');
        }

        $this->display('index');
    }

    //异步通知测试
    private function test_notifyurl()
    {
        $pay_config = C('lai_dian_fu_config');
        $data = $_POST;

        $sign =  md5('customerid='.$data['customerid'].'&status='.$data['status'].'&sdpayno='.$data['sdpayno'].'&sdorderno='.$data['sdorderno'].'&total_fee='.$data['total_fee'].'&paytype='.$data['paytype'].'&'.$pay_config['app_key']);

        $str = $data['status'].'|'.$data['customerid'].'|'.$data['sdpayno'].'|'.$data['sdorderno'].'|'.$data['total_fee'].'|'.$data['paytype'].'|'.$data['sign'];

        if($sign == $data['sign'])
        {
            if($data['status'] == 1)
            {
                Log::write("通知成功 \n".$str,'PAY');
                echo 'success';
                exit;
            }
            else
            {
                echo 'fail';
                Log::write("通知失败 \n".$str,'PAY');
                exit;
            }

        }
        else
        {
            Log::write("通知失败 \n".$str,'PAY');
            echo 'signerr';
        }
    }

    //让玩家成为代理
    public function getAgent($uid)
    {
        $userModel = new UserBaseModel();
        $sql = "select * from user_agent_active where uid = $uid";
        $res = $userModel->query($sql);
        if($res)
        {
            echo '用户已经是代理';
        }
        else if(empty($res))
        {
            $sql = "select * from user_base where uid = $uid";
            $userInfo = $userModel->query($sql);
            if(!$userInfo)
            {
                echo '没有找到用户';
            }
            else
            {
                $userModel->startTrans();
                $userInfo = $userInfo[0];
                $agent_id = $userInfo['invita_uid'];
                if($agent_id == 0)
                {
                    $agent_id = 1;
                }
                $md5 = rand(1000,9999);
                $passwd = md5($uid.$md5);
                $time = time();
                $sql = "insert into user_agent_active (uid,passwd,md5,agent_id,agent_status,create_time) values ('$uid','$passwd','$md5','$agent_id',1,'$time')";
                $res = $userModel->execute($sql);
                if($res === false)
                {
                    $userModel->rollback();
                    echo '操作失败1';
                }
                $sql = "update user_base set agent_id = $agent_id where uid = $uid";
                $res = $userModel->execute($sql);
                if($res === false)
                {
                    $userModel->rollback();
                    echo '操作失败2';
                }
                $userModel->commit();
                echo '成功';
            }
        }

    }

    //让玩家成为代理
    public function getAgentGeneral($uid,$fandian)
    {
        $userModel = new UserBaseModel();
        $sql = "select * from user_agent_active where uid = $uid";
        $user_info = $userModel->query($sql);
        if(!$user_info)
        {
            echo '用户已经是代理';
        }
        else
        {
            $user_info = $user_info[0];
            if($user_info['agent_id'] != 1)
            {
                echo '用户不是系统的直属代理';
            }
            else if($user_info['is_general'] == 1)
            {
                echo '用户已经是大区代理';
            }
            else
            {
                $sql = "update user_agent_active set is_general = 1,rebates_ratio = $fandian where uid = $uid";
                $res = $userModel->execute($sql);
                if($res === false)
                {
                    echo '修改失败';
                }
                else
                {
                    echo '修改成功';
                }
            }
        }

    }

}