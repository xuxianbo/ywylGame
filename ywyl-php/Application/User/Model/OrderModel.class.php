<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:13
 */

namespace User\Model;


use Think\Exception;

class OrderModel extends BaseModel
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
                $sql = "select count(distinct(uid)) as count_num from user_order where status = 1 and create_time >= :start_time and create_time <= :end_time";
                break;
            case 2:
                $sql = "select count(order_id) as count_num from user_order where uid = :uid and status = 0 and create_time >= :start_time and create_time <= :end_time";
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
     * 获取时间区间内成功支付的玩家数
     * @param $start_time
     * @param $end_time
     * @return bool|mixed
     */
    public function getCountByTime($start_time,$end_time)
    {
        $sql = $this->getSql(1,[
            'start_time' => $start_time,
            'end_time' => $end_time,
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
     * 获取时间区间内玩家创建的无用订单数
     * @param $uid
     * @param $start_time
     * @param $end_time
     * @return bool|mixed
     */
    public function getOrderNotCountByTime($uid,$start_time,$end_time)
    {
        $sql = $this->getSql(2,[
            'uid' => $uid,
            'start_time' => $start_time,
            'end_time' => $end_time,
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

    //创建订单
    public function createOrder($uid,$goods_id,$type,$goods_price,$goods_type,$goods_num){
        $create_time = time();
        $sql = "insert into user_order (uid,goods_id,serial_number,`type`,create_time,goods_price,goods_type,goods_num) values ($uid,$goods_id,0,$type,$create_time,$goods_price,$goods_type,$goods_num)";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    //根据订单号获取订单信息
    public function getInfoByOrderId($order_id){
        $sql = "select * from user_order where order_id = $order_id";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        return $res[0];
    }

    //获取订单基本信息
    public function getBaseInfoById($order_id){
        $sql = "select a.uid,a.status,a.goods_id,a.type as pay_type,b.type as goods_type,a.create_time,b.price,b.goods_num,b.give_num,b.name,b.goods_desc,b.is_sale from user_order a,goods b where a.order_id = '$order_id' and a.goods_id = b.id";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        if($res[0]['goods_id'] == 20001)
        {
            $res[0]['price'] = 0.01;
        }
        return $res[0];
    }

    /**
     * 完成订单
     * @param $order_id
     * @param $serial_number
     * @return int|mixed
     */
    public function finishOrder($order_id,$serial_number){
        //获取订单信息
        $orderInfo = $this->getInfoByOrderId($order_id);
        if($orderInfo === false || $orderInfo == 0)
        {
            return $orderInfo;
        }
        if($orderInfo['status'] != 4)
        {
            return -602;
        }
        $uid = $orderInfo['uid'];

        $goods_num = $orderInfo['goods_num'];
        $goods_type = $orderInfo['goods_type'];
        $goods_price = $orderInfo['goods_price'];

        $this->startTrans();
        $userActiveModel = new UserActiveModel();
        switch ($goods_type)
        {
            case 1:
                $res = $userActiveModel->updateMoneyByUid($uid,$goods_num);
                if($res === false){
                    $this->rollback();
                    return false;
                }
                break;//货币买money
            default:
                $this->rollback();
                return false;
                break;
        }

        //更新用户充值总数
        $res = $userActiveModel->updateTotalBuy($uid,$goods_price);
        if($res === false)
        {
            $this->rollback();
            return false;
        }

        //修改订单状态
        $res = $this->updateStatusByOrderUId($order_id,1,$serial_number);
        if($res === false)
        {
            $this->rollback();
            return $res;
        }

        $this->commit();
        return true;
    }

    //修改订单状态
    public function updateStatusByOrderUId($order_id,$status,$serial_number = "0"){
        $sql = "update user_order set status = $status,serial_number = '$serial_number' where order_id = $order_id";
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
     * 获取用户是否购买过一个商品
     * @param $uid
     * @param $goods_id
     */
    public function getIsBuyGoods($uid,$goods_id)
    {
        $sql = "select order_id from user_order where uid = $uid and status != 0 and goods_id = $goods_id";
        try
        {
            $info = $this->query($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return $info;
    }

}