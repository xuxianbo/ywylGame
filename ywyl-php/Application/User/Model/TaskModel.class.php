<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 14:21
 */

namespace User\Model;


use Think\Exception;

class TaskModel extends BaseModel
{
    /**
     * 添加任务配置
     * @param $data
     * @param $time
     * @return bool
     */
    public function addUserTaskConfig($data,$time){
        $id = $data['id'];
        $series_id  = $data['series_id '];
        $task_desc = $data['task_desc'];
        $finish_num = $data['finish_num'];
        $give_points = $data['give_points'];
        $give_gold = $data['give_gold'];
        $give_diamond = $data['give_diamond'];
        $other_type = $data['other_type'];
        $give_other = $data['give_other'];
        $sql = "insert into config_task values ($id,$series_id,'$task_desc',$finish_num,$give_points,$give_gold,$give_diamond,$give_other,$other_type,0,$time)";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 获取任务配置   同系列任务只能获取到id最小的一条
     * @param $type   任务类型   1为成就任务   2为日常任务
     * @return bool|mixed
     */
    public function getUserTaskConfig($type){
        switch ($type){
            case 1:
                $min_id = C('by_task_achi.min_id');
                $max_id = C('by_task_achi.max_id');
                $sql = "select id,series_id,task_desc,finish_num,give_points,give_gold,give_diamond,status from config_task where id between $min_id and $max_id and status = 1";
                break;
            case 2:
                $min_id = C('by_task_daily.min_id');
                $max_id = C('by_task_daily.max_id');
                $fixed_id = C('by_task_fixed_id');
                $sql = "select id,series_id,task_desc,finish_num,give_points,give_gold,give_diamond,status from config_task where (id = $fixed_id or id between $min_id and $max_id) and status = 1";
                break;
            default:
                $this->logError("出现未定义的任务类型",'EMERG');
                return false;
                break;
        }
        try{
            $user_tasks = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $user_tasks;
    }

    /**
     * 获取任务配置   根据id
     * @param $taks_id
     * @return bool|mixed
     */
    public function getUserTaskConfigById($id){
        $sql = "select id,series_id,task_desc,finish_num,give_points,give_gold,give_diamond,status from config_task where id = $id";
        try{
            $user_tasks = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $user_tasks;
    }

    /**
     * 获取用户任务记录
     * @param $type   任务类型   1为成就任务   2为日常任务
     * @param $uid    用户id
     * @return bool|mixed
     */
    public function getUserTask($type,$uid,$date = ""){
        switch ($type){
            case 1:
                $min_id = C('by_task_achi.min_id');
                $max_id = C('by_task_achi.max_id');
                $sql = "select a.task_id,b.task_desc,b.series_id,b.finish_num,a.is_finish,a.is_give,a.`date`,b.give_points,b.give_gold,b.give_diamond,b.give_other,b.other_type from user_task a,config_task b where b.status = 1 and uid = $uid and task_id between $min_id and $max_id and a.task_id = b.id order by task_id asc";
                break;
            case 2:
                $min_id = C('by_task_daily.min_id');
                $max_id = C('by_task_daily.max_id');
                $fixed_id = C('by_task_fixed_id');
                $sql = "select a.task_id,b.task_desc,b.series_id,b.finish_num,a.is_finish,a.is_give,a.`date`,b.give_points,b.give_gold,b.give_diamond,b.give_other,b.other_type from user_task a,config_task b where b.status = 1 and uid = $uid and (task_id between $min_id and $max_id or task_id = $fixed_id) and a.task_id = b.id and `date` = $date order by task_id asc";
                break;
            default:
                $this->logError("出现未定义的任务类型",'EMERG');
                return false;
                break;
        }
        try{
            $user_tasks = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $user_tasks;
    }

    /**
     * 获取用户任务记录   根据用户id和任务id   日常任务还需要根据时间
     * @param $type
     * @param $task_id
     * @param $uid
     * @param $date
     * @return bool|mixed
     */
    public function getUserTaskById($type,$task_id,$uid,$date = ""){
        switch ($type){
            case 1:
                $sql = "select a.task_id,b.task_desc,b.series_id,b.finish_num,b.give_points,b.give_gold,b.give_diamond,a.is_finish,a.is_give,a.`date` from user_task a,config_task b where uid = $uid and task_id  = $task_id and b.id = a.task_id";
                break;
            case 2:
                $sql = "select a.task_id,b.task_desc,b.series_id,b.finish_num,b.give_points,b.give_gold,b.give_diamond,a.is_finish,a.is_give,a.`date` from user_task a,config_task b where uid = $uid and task_id  = $task_id and b.id = a.task_id and a.`date` = $date";
                break;
            default:
                $this->logError("出现未定义的任务类型",'EMERG');
                return false;
                break;
        }
        try{
            $user_tasks = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $user_tasks;
    }

    /**
     * 添加用户任务记录
     * @param $uid
     * @param $task_id
     * @param $date
     * @return bool
     */
    public function addUserTask($uid,$task_id,$date){
        $sql = "insert into user_task (uid,task_id,`date`) values ('$uid','$task_id','$date')";
        try{
             $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 用户已经领取任务奖励
     * @param $type
     * @param $uid
     * @param $task_id
     * @param $date
     * @return bool
     */
    public function setFinishTask($type,$task_id,$uid,$date = ""){
        switch ($type){
            case 1:
                $sql = "update user_task set is_give = 1 where uid = $uid and task_id = $task_id";
                break;
            case 2:
                $sql = "update user_task set is_give = 1 where uid = $uid and task_id = $task_id and `date` = $date";
                break;
            default:
                $this->logError("出现未定义的任务类型",'EMERG');
                return false;
                break;
        }
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 判断用户是否完成任务
     * @param $type
     * @param $task_id
     * @param $uid
     * @param $series_id
     * @param $finish_num
     * @param $date
     * @return bool
     */
    public function isFinishTask($type,$task_id,$uid,$series_id,$finish_num,$date = "",$info){
        $res = 0;
        switch ($series_id)
        {
            case 701:
                break;
            case 702:
                if($info['is_modname'] == 0){
                    $res = true;
                }
                break;
            case 703:
                if(!is_numeric($info['pic_head']))
                {
                    $res = true;
                }
                break;
            case 704:
                if(!is_numeric($info['real_name']))
                {
                    $res = true;
                }
                break;
            case 705:
                if($info['total_buy'] != 0){
                    $res = true;
                }
                break;
            case 706:
                if($info['vip'] != 0){
                    $res = true;
                }
                break;
            default:
                $this->logError("出现未定义的任务系列类型",'EMERG');
                return false;
                break;
        }
        if($res === true)
        {
            switch ($type){
                case 1:
                    $sql = "update user_task set is_finish = 1 where uid = $uid and task_id = $task_id";
                    break;
                case 2:
                    $sql = "update user_task set is_finish = 1 where uid = $uid and task_id = $task_id and `date` = $date";
                    break;
                default:
                    $this->logError("出现未定义的任务类型",'EMERG');
                    return false;
                    break;
            }
            try{
                $this->execute($sql);
            }catch (Exception $e){
                $this->logError($e,'EMERG');
                return false;
            }
            return true;
        }
        else
        {
            return $res;
        }
    }
}