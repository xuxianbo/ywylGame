<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:12
 */

namespace User\Model;


use Think\Exception;

class GoodsModel extends BaseModel
{
    /**
     * 获取sql语句
     * @param $code
     * @param string $bind
     * @return bool|mixed|string
     */
    protected function getSql($code,$bind = "")
    {
        switch ($code)
        {
            case 1:
                $sql = 'select id,app_id,apple_id,`type`,price,goods_num,give_num,`name`,goods_desc from config_goods where app_id = :app_id and status = 1';
                break;
            case 2:
                $sql = 'select app_id,apple_id,`type`,price,goods_num,give_num,`name`,goods_desc from config_goods where id = :id';
                break;
            case 3:
                $sql = 'select id,apple_id,`type`,price,goods_num,give_num,`name`,goods_desc from config_goods where app_id = :app_id and status = 1';
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
     * 获取商城信息
     * @return bool|mixed
     */
    public function getInfo($app_id)
    {
        $sql = $this->getSql(1,[
            'app_id' => $app_id
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

    /**
     * 获取商城信息
     * @param $id
     * @return bool|mixed
     */
    public function getInfoById($id)
    {
        $sql = $this->getSql(2,[
            'id' => $id
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

    /**
     * 获取商城信息
     * @return bool|mixed
     */
    public function getInfoByAppId($app_id)
    {
        $sql = $this->getSql(3,[
            'app_id' => $app_id
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