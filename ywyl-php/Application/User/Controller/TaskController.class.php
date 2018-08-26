<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 14:38
 */

namespace User\Controller;



use User\Model\TaskModel;
use User\Model\UserActiveModel;
use User\Model\UserBaseModel;

class TaskController extends BaseController
{
    /**
     * 获取用户任务信息
     * @param string $user_token
     * @param string $uid
     */
    public function getUserTask($user_token = "",$uid = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == ""){
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
        }
        $taskModel = new TaskModel();
        $task_achi_type = 1;
        $task_daily_type = 2;

        //获取用户成就任务信息
        $user_taks_achi = $taskModel->getUserTask($task_achi_type,$uid);
        $date = date("Ymd");
        if($user_taks_achi === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if(empty($user_taks_achi))
        {
            //添加用户成就任务
            $taks_achi = $taskModel->getUserTaskConfig($task_achi_type);
            if($taks_achi === false)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
            }
            else if(empty($taks_achi))
            {
                $user_taks_achi = [];
            }
            else
                {
                $taskModel->startTrans();
                foreach ($taks_achi as $key => $val)
                {
                    //获取用户成就任务
                    $res = $taskModel->getUserTaskById($task_achi_type,$val['id'],$uid);
                    if($res === false)
                    {
                        $taskModel->rollback();
                        $this->error_return(-1);
                    }
                    else if(empty($res))
                    {
                        //添加用户成就任务
                        $res = $taskModel->addUserTask($uid,$val['id'],$date);
                        if($res === false)
                        {
                            $taskModel->rollback();
                            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
                        }
                        $user_taks_achi[$key]['task_id'] = $val['id'];
                        $user_taks_achi[$key]['series_id'] = $val['series_id'];
                        $user_taks_achi[$key]['task_desc'] = $val['task_desc'];
                        $user_taks_achi[$key]['finish_num'] = $val['finish_num'];
                        $user_taks_achi[$key]['is_finish'] = '0';
                        $user_taks_achi[$key]['is_give'] = '0';
                        $user_taks_achi[$key]['date'] = $date;
                    }
                }
                $taskModel->commit();
            }
        }

        //获取用户日常任务信息
        $user_taks_daily = $taskModel->getUserTask($task_daily_type,$uid,$date);
        $date = date("Ymd");
        if($user_taks_daily === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if(empty($user_taks_daily))
        {
            //添加用户日常任务
            $taks_daily = $taskModel->getUserTaskConfig($task_daily_type);
            if($taks_daily === false)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
            }
            else if(empty($taks_daily))
            {
                $user_taks_daily = [];
            }
            else
                {
                //从日常任务中随机10个任务加上固定id为9999的任务组成一个11个任务集合添加进用户任务记录中
                $taskModel->startTrans();
                foreach ($taks_daily as $key => $val){
                    if($val['id'] == 9999){
                        //获取用户任务
                        $res = $taskModel->getUserTaskById($task_daily_type,$taks_daily[$val]['id'],$uid,$date);
                        if($res === false)
                        {
                            $taskModel->rollback();
                            $this->error_return(-1);
                        }
                        else if(empty($res))
                        {
                            $res = $taskModel->addUserTask($uid,$val['id'],$date);
                            if($res === false){
                                $taskModel->rollback();
                                $this->error_return(-1);
                            }
                            $user_taks_daily[10]['task_id'] = $val['id'];
                            $user_taks_daily[10]['series_id'] = $val['series_id'];
                            $user_taks_daily[10]['task_desc'] = $val['task_desc'];
                            $user_taks_daily[10]['finish_num'] = $val['finish_num'];
                            $user_taks_daily[10]['is_finish'] = '0';
                            $user_taks_daily[10]['is_give'] = '0';
                            $user_taks_daily[10]['date'] = $date;
                            array_splice($taks_daily,$key);
                        }

                    }
                }

                $taks_daily_rand = array_rand($taks_daily,10);

                foreach ($taks_daily_rand as $key => $val){
                    $res = $taskModel->getUserTaskById($task_daily_type,$taks_daily[$val]['id'],$uid,$date);
                    if($res === false)
                    {
                        $taskModel->rollback();
                        $this->error_return(-1);
                    }
                    else if(empty($res))
                    {
                        $res = $taskModel->addUserTask($uid,$taks_daily[$val]['id'],$date);
                        if($res === false)
                        {
                            $taskModel->rollback();
                            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
                        }
                        $user_taks_daily[$key]['task_id'] = $taks_daily[$val]['id'];
                        $user_taks_daily[$key]['series_id'] = $taks_daily[$val]['series_id'];
                        $user_taks_daily[$key]['task_type'] = $task_daily_type;
                        $user_taks_daily[$key]['task_desc'] = $taks_daily[$val]['task_desc'];
                        $user_taks_daily[$key]['finish_num'] = $taks_daily[$val]['finish_num'];
                        $user_taks_daily[$key]['is_finish'] = '0';
                        $user_taks_daily[$key]['is_give'] = '0';
                        $user_taks_daily[$key]['date'] = $date;
                    }

                }
                $taskModel->commit();
                sort($user_taks_daily);
            }
        }

        //获取用户的基本信息
        $userBaseModel = new UserBaseModel();
        $user_base = $userBaseModel->getInfoByToken($user_token);
        if($user_base === false){
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if($user_base == 0)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户资料为空'],'statusCode'=>-80002]);
        }

        $userActiveModel = new UserActiveModel();
        $user_active = $userActiveModel->getInfoById($uid);
        if($user_active === false){
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if(empty($user_active))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户资料为空'],'statusCode'=>-80003]);
        }

        $user_info = [];
        foreach ($user_base as $k => $v){
            $user_info[$k] = $v;
        }
        foreach ($user_active as $k => $v){
            $user_info[$k] = $v;
        }
        //检测用户成就任务是否达到要求
        foreach ($user_taks_achi as $key => $val){
            if($val['is_finish'] == 1)
            {
                $user_taks_achi[$key]['task_plan'] = $val['finish_num'];
            }
            else
            {
                $res = $taskModel->isFinishTask($task_achi_type,$val['task_id'],$uid,$val['series_id'],$val['finish_num'],$date,$user_info);
                if($res === false)
                {
                    $user_taks_achi[$key]['task_plan'] = 0;
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
                }
                else if($res === true)
                {
                    $user_taks_achi[$key]['task_plan'] = $val['finish_num'];
                    $user_taks_achi[$key]['is_finish'] = '1';
                }
                else
                {
                    $user_taks_achi[$key]['task_plan'] = $res;
                }
            }

        }

        //检测用户日常任务是否达到要求
        foreach ($user_taks_daily as $key => $val){
            $user_taks_daily[$key]['task_plan'] = 0;
            if($val['is_give'] == 1)
            {
                $user_taks_daily[$key]['is_finish'] = 1;
            }
//            if($val['is_finish'] == 1)
//            {
//                $user_taks_daily[$key]['task_plan'] = $val['finish_num'];
//            }
//            else
//            {
//                $res = $this->isFinishTask($task_daily_type,$val['task_id'],$uid,$val['series_id'],$val['finish_num'],$date);
//                if($res === false)
//                {
//                    $user_taks_daily[$key]['task_plan'] = 0;
//                }
//                else if($res === true)
//                {
//                    $user_taks_daily[$key]['task_plan'] = $val['finish_num'];
//                }
//                else
//                {
//                    $user_taks_daily[$key]['task_plan'] = $res;
//                }
//            }
        }

        $this->ajaxReturn(['status'=>'true','data'=>['user_taks_achi'=>$user_taks_achi,'user_taks_daily'=>$user_taks_daily],'statusCode'=>1]);
    }

    /**
     * 用户完成任务
     * @param string $user_token
     * @param string $uid
     * @param string $task_id
     * @param string $task_type
     */
    public function userFinishTask($user_token = "",$uid = "",$task_id = "",$task_type = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == ""){
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
        }
        if(empty($task_id))  $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递task_id'],'statusCode'=>-80101]);
        if(empty($task_type))  $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递task_type'],'statusCode'=>-80102]);
        $date = date("Ymd");
        //获取任务信息
        $taskModel = new TaskModel();
        $task_info = $taskModel->getUserTaskById($task_type,$task_id,$uid,$date);
        if($task_info === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if(empty($task_info))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户没有该任务'],'statusCode'=>-80103]);
        }
        else
        {
            $task_info = $task_info[0];
            if($task_info['is_finish'] == 0 && $task_type == 1)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户任务没有完成'],'statusCode'=>-80104]);
            }
            else if($task_info['is_give'] == 1)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>"任务奖励已经领取"],'statusCode'=>-80105]);
            }
            else if($task_info['is_give'] == 0 && ($task_info['is_finish'] == 1 || $task_type == 2) )
            {
                //发放任务奖励
                $userActiveModel = new UserActiveModel();
                $give_points_num = $task_info['give_points'];
                $give_gold_num = $task_info['give_gold'];
                $give_diamond_num = $task_info['give_diamond'];

                //给与奖励
                $userActiveModel->startTrans();
                if($give_points_num > 0){
                    $res = $userActiveModel->updatePointsByUid($uid,$give_points_num);
                    if($res !== true){
                        $userActiveModel->rollback();
                        $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
                    }
                }
                if($give_gold_num > 0){
                    $res = $userActiveModel->updateGoldByUid($uid,$give_gold_num);
                    if($res !== true){
                        $userActiveModel->rollback();
                        $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
                    }
                }
                if($give_diamond_num > 0){
                    $res = $userActiveModel->updateDiamondByUid($uid,$give_diamond_num);
                    if($res !== true){
                        $userActiveModel->rollback();
                        $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
                    }
                }

                //修改任务记录状态
                $res = $taskModel->setFinishTask($task_type,$task_id,$uid,$date);
                if($res !== true)
                {
                    $userActiveModel->rollback();
                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
                }
                else
                {
                    $userActiveModel->commit();
                    //更新用户redis
                    $user_active = $this->getUserActiveRedis($uid);
                    $this->ajaxReturn(['status'=>'true','data'=>[
                        'message'=>'领取成功',
                        'user_gold' => $user_active['gold'],
                        'user_diamond' => $user_active['diamond'],
                        'user_points' => $user_active['points'],
                    ],'statusCode'=>1]);
                }

            }
            else
            {
                $this->ajaxReturn(['status'=>'true','data'=>['message'=>"任务奖励已经领取"],'statusCode'=>1]);
            }
        }
    }
}