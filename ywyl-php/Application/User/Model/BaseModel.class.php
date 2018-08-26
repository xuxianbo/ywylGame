<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 11:58
 */

namespace User\Model;


use Think\Log;
use Think\Model;

class BaseModel extends Model
{
    //是否自动检测数据表字段信息
    protected $autoCheckFields = false;

    //错误日志
    protected function logError($e,$level){
        Log::write($e,$level);
        return -1;
    }

    /**
     * 将数据放入sql语句中对应的占位符上
     * @param $sql
     * @param $bind
     * @return mixed
     */
    protected function bindSql($sql,$bind)
    {
        if(is_array($bind))
        {
            foreach ($bind as $key => $value)
            {
                if(is_numeric($value))
                {
                    $sql = str_replace(":$key",$value,$sql);
                }
                else
                {
                    $sql = str_replace(":$key","'$value'",$sql);
                }

            }
        }
        return $sql;
    }
}