<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/5/11
 * Time: 10:22
 */

namespace User\Model;


use Think\Exception;

class ActivityModel extends BaseModel
{

    /**
     * 获取sql语句
     * @param $code
     * @param string $bind
     * @return bool|string
     */
    protected function getSql($code,$bind = "")
    {
        switch ($code)
        {
            case 1:
                $sql = "select id,activity_name,activity_desc,reward_gold,reward_diamond,reward_points,reward_props,reward_props_num,start_time,end_time,status,create_time from config_activity where id = :id";
                break;
            default:
                return false;
        }

        if(!empty($bind))
        {
            $sql = $this->bindSql($sql,$bind);
        }

        return $sql;
    }

    /**
     * 根据id获取活动信息
     * @param $id
     * @return bool|mixed
     */
    public function getInfoById($id)
    {
        $sql = $this->getSql(1,[
            'id' => $id,
        ]);
        try
        {
            $res = $this->query($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }
}