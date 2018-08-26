<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/4/13
 * Time: 11:17
 */

namespace User\Model;


use Think\Exception;

class PropsModel extends BaseModel
{

    //获取sql语句
    protected function getSql($code,$bind = "")
    {
        switch ($code)
        {
            case 1:
                $sql = "select b.props_id,a.props_type,a.props_name,a.props_desc,a.props_max_num,a.props_use_num,a.give_num,b.props_num,a.is_use from config_props a, user_props b where b.uid = :uid and b.props_id = a.id and b.props_num != 0";
                break;
            case 2:
                $sql = "select b.props_id,a.props_type,a.props_name,a.props_desc,a.props_max_num,a.props_use_num,a.give_num,b.props_num,a.is_use from config_props a, user_props b where b.uid = :uid and b.props_id = :prop_id and b.props_id = a.id";
                break;
            case 3:
                $sql = "insert into user_props (uid,props_id,props_num) values (:uid,:prop_id,:num)";
                break;
            case 4:
                $sql = "update user_props set props_num = props_num + :num where uid = :uid and props_id = :prop_id";
                break;
            case 5:
                $sql = "update user_props set props_num = props_num - :num where uid = :uid and props_id = :prop_id";
                break;
            case 6:
                $sql = "insert into user_props_record (uid,props_id,use_num,use_time) values (:uid,:prop_id,:use_num,:use_time)";
                break;
            case 7:
                $sql = "select * from config_props";
                break;
            case 8:
                $sql = "insert into user_recharge_phone (uid,phone,num,create_time) values (:uid,:phone,:num,:create_time)";
                break;
            case 9:
                $sql = "select id as goods_id, goods_name, goods_desc, price, price_type, give_num, status from props_goods where app_id = :app_id and status > 0";
                break;
            case 10:
                $sql = "select id as goods_id, goods_name, goods_desc, price, price_type, give_num, status from props_goods where app_id = :app_id and status = :status";
                break;
            case 11:
                $sql = "select props_id,goods_name, goods_desc, price, price_type, give_num, status from props_goods where id = :goods_id";
                break;
            case 12:
                $sql = "insert into user_props (uid,props_id,props_num) values (:uid,:props_id,:props_num)";
                break;
            case 13:
                $sql = "update user_props set props_num = props_num + :props_num where uid = :uid and props_id = :props_id";
                break;
            case 14:
                $sql = "update props_goods set buy_num = buy_num + 1 where id = :goods_id";
                break;
            case 15:
                $sql = "select * from config_props where id = :props_id";
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
     * 获取玩家道具信息
     * @param $uid
     * @param string $prop_id
     * @return bool|mixed
     */
    public function getPropsInfo($uid,$prop_id = "")
    {
        if($prop_id == "")
        {
            $sql = $this->getSql(1,[
                'uid' => $uid,
            ]);
        }
        else
        {
            $sql = $this->getSql(2,[
                'uid' => $uid,
                'prop_id' => $prop_id,
            ]);
        }

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
     * 添加玩家道具
     * @param $uid
     * @param $prop_id
     * @param $num
     * @return bool|int
     */
    public function addProps($uid,$prop_id,$num)
    {
        $res = $this->getPropsInfo($uid,$prop_id);
        if($res === false)
        {
            return false;
        }
        else if($res[0]['props_num'] + $num > $res[0]['props_max_num'])
        {
            return -10000;
        }
        else if(empty($res))
        {
            $sql = $this->getSql(3,[
                'uid' => $uid,
                'prop_id' => $prop_id,
                'num' => $num,
            ]);
        }
        else
        {
            $sql = $this->getSql(4,[
                'uid' => $uid,
                'prop_id' => $prop_id,
                'num' => $num,
            ]);
        }

        try
        {
            $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return 1;
    }

    /**
     * 减少玩家道具
     * @param $uid
     * @param $prop_id
     * @param $num
     * @return bool
     */
    public function reduceProps($uid,$prop_id,$num)
    {

        $sql = $this->getSql(5,[
            'uid' => $uid,
            'prop_id' => $prop_id,
            'num' => $num,
        ]);

        try
        {
            $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 添加玩家道具使用记录
     * @param $uid
     * @param $prop_id
     * @param $use_num
     * @param $use_time
     * @return bool
     */
    public function addPropsUseRecord($uid,$prop_id,$use_num,$use_time)
    {
        $sql = $this->getSql(6,[
            'uid' => $uid,
            'prop_id' => $prop_id,
            'use_num' => $use_num,
            'use_time' => $use_time,
        ]);

        try
        {
            $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 获取道具表配置
     * @return bool|mixed
     */
    public function getPropsConfig()
    {
        $sql = $this->getSql(7);
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
     * 添加用户话费充值记录
     * @param $uid
     * @param $phone
     * @param $give_num
     * @param $time
     * @return bool
     */
    public function addPhoneMoneyRecord($uid,$phone,$give_num,$time)
    {
        $sql = $this->getSql(8,[
            'uid' => $uid,
            'phone' => $phone,
            'num' => $give_num,
            'create_time' => $time,
        ]);
        try
        {
            $res = $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 获取道具表配置
     * @param $app_id
     * @return bool|mixed
     */
    public function getPropsShop($app_id)
    {
        $sql = $this->getSql(9,[
            'app_id' => $app_id,
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
     * 根据商品状态获取商品
     * @param $app_id
     * @param $status
     * @return bool|mixed
     */
    public function getPropsShopByStatus($app_id,$status)
    {
        $sql = $this->getSql(10,[
            'app_id' => $app_id,
            'status' => $status,
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
     * 根据商品id获取商品信息
     * @param $goods_id
     * @return bool|mixed
     */
    public function getPropsGoodsInfoById($goods_id)
    {
        $sql = $this->getSql(11,[
            'goods_id' => $goods_id,
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
     * 添加用户持有道具
     * @param $uid
     * @param $phone
     * @param $give_num
     * @param $time
     * @return bool
     */
    public function addUserProps($uid,$props_id,$props_num)
    {
        $sql = $this->getSql(12,[
            'uid' => $uid,
            'props_id' => $props_id,
            'props_num' => $props_num,
        ]);
        try
        {
            $res = $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 修改用户持有道具数量
     * @param $uid
     * @param $phone
     * @param $give_num
     * @param $time
     * @return bool
     */
    public function setUserPropsNum($uid,$props_id,$props_num)
    {
        $sql = $this->getSql(13,[
            'uid' => $uid,
            'props_id' => $props_id,
            'props_num' => $props_num,
        ]);
        try
        {
            $res = $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 增加道具商品的购买次数
     * @param $goods_id
     * @return bool
     */
    public function addPropsGoodsNum($goods_id)
    {
        $sql = $this->getSql(14,[
            'goods_id' => $goods_id,
        ]);
        try
        {
            $res = $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 获取道具配置信息
     * @param $props_id
     */
    public function getPropsConfigInfo($props_id)
    {
        $sql = $this->getSql(15,[
            'props_id' => $props_id,
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