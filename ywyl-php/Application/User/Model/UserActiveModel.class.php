<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/19
 * Time: 13:59
 */

namespace User\Model;


use Think\Exception;
use Think\Log;

class UserActiveModel extends BaseModel
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
            case 2:
                $sql = 'select gold from user_active where uid = :uid';
                break;
            case 3:
                $sql = 'update user_active set broadcast_num = broadcast_num + :num where uid = :uid';
                break;
            case 4:
                $sql = 'select `name`,pic_head,sex,sign,gold,money,house,reffle_num,total_buy,is_buy_money from user_active where uid = :uid';
                break;
            case 5:
                $sql = 'insert into user_active (uid,gold,money,`name`,reffle_num,pic_head,sex) values (:uid,:gold,:money,:name,:is_reffle,:pic_head,:sex)';
                break;
            case 6:
                $sql = 'update user_active set gold = :gold,money = :money,reffle_num = :is_reffle where uid = :uid';
                break;
            case 9:
                $sql = 'update user_active set `name` = :name,sex = :sex,pic_head = :pic_head where uid = :uid';
                break;
            case 10:
                $sql = 'select uid,pic_head,money,`name`,sign from user_active where uid != 10000 and uid != 10001 order by money desc limit :limit';
                break;
            case 11:
                $sql = 'select gold from user_active where uid = :uid';
                break;
            case 12:
                $sql = 'update user_active set gold = gold + :get_gold where uid = :uid';
                break;
            case 13:
                $sql = 'select money from user_active where uid = :uid';
                break;
            case 14:
                $sql = 'update user_active set money = money + :get_num where uid = :uid';
                break;
            case 15:
                $sql = 'update user_active set total_buy = total_buy + :price,is_buy_money = is_buy_money - :is_buy_money where uid = :uid';
                break;
            case 16:
                $sql = 'update user_active set reffle_num = reffle_num - 1 where uid = :uid';
                break;
            case 18:
                $sql = 'select now_sign_in,is_sign_in,sign_in_time from user_sign_in where uid = :uid';
                break;
            case 19:
                $sql = 'insert into user_sign_in (uid) values (:uid)';
                break;
            case 20:
                $sql = 'update user_active set reffle_num = :reffle_num,is_buy_money = :is_buy_money where uid > 9999';
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
     * 根据id获取用户的持有金币数
     * @param $uid
     * @return bool|mixed
     */
    public function getGoldById($uid)
    {
        $sql = $this->getSql(2,[
            'uid' => $uid,
        ]);
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

    /**
     * 修改用户道具数量
     * @param $uid
     * @param $num
     * @return bool|mixed
     */
    public function setUserBroadcastNum($uid,$num)
    {
        $sql = $this->getSql(3,[
            'uid' => $uid,
            'num' => $num,
        ]);
        try
        {
            $info = $this->execute($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 根据id获取信息
     * @param $uid
     * @return bool|int
     */
    public function getInfoById($uid){
        $sql = $this->getSql(4,[
            'uid' => $uid,
        ]);
        try
        {
            $info = $this->query($sql);
        }
        catch (Exception $e)
        {
            $this->logError($e,'EMERG');
            return false;
        }
        if(!empty($info))
        {
            $info = $info[0];
        }
        return $info;
    }

    /**
     * 添加用户信息
     * @param $uid
     * @param $name
     * @return bool
     */
    public function addUser($uid,$name = "",$sex = 0,$pic_head = ""){
        $gold = C('USER_DEFAULT.DEFAULT_GOLD');
        $money = C('USER_DEFAULT.DEFAULT_MONEY');
        $is_reffle = C('USER_DEFAULT.DEFAULT_REFFLE_NUM');

        //微信的性别跟我们设定的性别不一样，所以需要检测判断一下
        if($sex == 2)
        {
            $sex = 1;
        }
        else
        {
            $sex = 0;
        }

        //如果头像地址为数字或者为空的话等于性别
        if(is_numeric($pic_head) || empty($pic_head))
        {
            $pic_head = $sex;
        }

        //处理空昵称并加密昵称
        if($name == "")
        {
            $num = $uid;
            $name = "游客".$num;
        }
        $name = base64_encode($name);

        $sql = $this->getSql(5,[
            'uid' => $uid,
            'gold' => $gold,
            'money' => $money,
            'name' => $name,
            'is_reffle' => $is_reffle,
            'pic_head' => $pic_head,
            'sex' => $sex,
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
     * 补全用户信息
     * @param $uid
     * @return bool
     */
    public function updateUserInfo($uid)
    {
        $gold = C('USER_DEFAULT.DEFAULT_GOLD');
        $money = C('USER_DEFAULT.DEFAULT_MONEY');
        $is_reffle = C('USER_DEFAULT.DEFAULT_REFFLE_NUM');

        $sql = $this->getSql(6,[
            'uid' => $uid,
            'gold' => $gold,
            'money' => $money,
            'is_reffle' => $is_reffle,
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
     * 更新用户的姓名和头像
     * @param $uid
     * @param $name
     * @param $pic_head
     * @param $sex
     */
    public function setWecherLoginUserInfo($uid,$name = "",$pic_head,$sex)
    {
        //微信的性别跟我们设定的性别不一样，所以需要检测判断一下
        if($sex == 2)
        {
            $sex = 1;
        }
        else
        {
            $sex = 0;
        }

        //如果头像地址为数字或者为空的话等于性别
        if(is_numeric($pic_head) || empty($pic_head))
        {
            $pic_head = $sex;
        }

        //处理空昵称并加密昵称
        if($name == "")
        {
            $num = $uid;
            $name = "游客".$num;
        }
        $name = base64_encode($name);

        $sql = $this->getSql(9,[
            'uid' => $uid,
            'name' => $name,
            'pic_head' => $pic_head,
            'sex' => $sex,
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
     * 获取现金最多的用户信息
     * @return bool|int|mixed
     */
    public function getGoldRank(){
        $limit = C('record_num');
        $sql = $this->getSql(10,[
            'limit' => $limit
        ]);
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

    /**
     * 更新用户金币
     * @param $uid
     * @param $gold
     * @return int
     */
    public function updateGoldByUid($uid,$get_gold){
        $sql = $this->getSql(11,[
            'uid' => $uid
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

        if(empty($res))
        {
            return 0;
        }
        $gold = $res[0]['gold'];
        if($gold+$get_gold < 0)
        {
            return -11;
        }

        $sql = $this->getSql(12,[
            'get_gold' => $get_gold,
            'uid' => $uid,
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
     * 更新用户money
     * @param $uid
     * @param $gold
     * @return int
     */
    public function updateMoneyByUid($uid,$get_num){
        $sql = $this->getSql(13,[
            'uid' => $uid
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

        if(empty($res))
        {
            return 0;
        }
        $money = $res[0]['gold'];
        if($money + $get_num < 0)
        {
            return -11;
        }

        $sql = $this->getSql(14,[
            'get_num' => $get_num,
            'uid' => $uid,
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
     * 更新用户的充值总数
     * @param $uid
     * @param $price
     * @return bool
     */
    public function updateTotalBuy($uid,$price){
        $sql = $this->getSql(15,[
            'price' => $price,
            'uid' => $uid,
            'is_buy_money' => $price,
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
     * 更新用户抽奖次数
     * @param $uid
     * @return bool
     */
    public function updateIsReffleByUid($uid){
        $sql = $this->getSql(16,[
            'uid' => $uid,
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
     * 每天零点重置用户需要重置的数据
     * @return bool
     */
    public function initiaUserInfo()
    {
        $reffle_num = C('USER_DEFAULT.DEFAULT_REFFLE_NUM');
        $is_buy_money = C('USER_DEFAULT.DEFAULT_BUY_NUM');

        $sql = $this->getSql(20,[
            'reffle_num' => $reffle_num,
            'is_buy_money' => $is_buy_money,
        ]);

        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /*以下方法sql较为繁琐*/
    /**
     * 更新用户性别和个人签名
     * @param $uid
     * @param $sex
     * @param $sign
     * @return int
     */
    public function updateSexSignByUid($uid,$sex,$sign,$is_change_pic_head){
        if($sex == '' && $sign != '')
        {
            $sql = "update user_active set sign = '$sign' where uid = $uid";
        }
        else if($is_change_pic_head === true)
        {
            if($sign == '' && $sex != '')
            {
                $sql = "update user_active set sex = $sex,pic_head = $sex where uid = $uid";
            }
            else
            {
                $sql = "update user_active set sex = $sex,pic_head = $sex,sign = '$sign' where uid = $uid";
            }
        }
        else if($is_change_pic_head === false)
        {
            if($sign == '' && $sex != '')
            {
                $sql = "update user_active set sex = $sex where uid = $uid";
            }
            else
            {
                $sql = "update user_active set sex = $sex,sign = '$sign' where uid = $uid";
            }
        }
        else
        {
            return false;
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
        return true;
    }

    /**
     * 修改用户签到信息
     * @param $uid
     * @param string $now_sign_in 用户第几次签到
     * @param string $is_sign_in 用户是否签到   0为否   1为是
     * @param string $sign_in_time 最近一次签到时间的时间戳
     * @return bool
     */
    public function setUserSignIn($uid,$now_sign_in = "",$is_sign_in = "",$sign_in_time = ""){
        $set = "";
        if(!empty($now_sign_in))
        {
            $set .= "now_sign_in = $now_sign_in,";
        }
        if(!empty($now_sign_in))
        {
            $set .= "is_sign_in = $is_sign_in,";
        }
        if(!empty($now_sign_in))
        {
            $set .= "sign_in_time = $sign_in_time,";
        }
        $set = substr($set,0,strlen($set)-1);
        $sql = "update user_sign_in set $set where uid = $uid";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    //以下的方法都不需要 后面根据方法删除对应调用的逻辑

    /**
     * 更新用户钻石
     * @param $uid
     * @param $user_diamond
     * @return bool|int
     */
    public function updateDiamondByUid($uid,$user_diamond){
        $sql = "select diamond from user_active where uid = $uid";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        $diamond = $res[0]['diamond'];
        if($diamond+$user_diamond<0){
            return -13;
        }

        $sql = "update user_active set diamond = diamond+$user_diamond where uid = $uid";
        try{
            $res = $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

    /**
     * 更新用户积分
     * @param $uid
     * @param $user_points
     * @return int
     */
    public function updatePointsByUid($uid,$user_points){
        $sql = "select points from user_active where uid = $uid";
        try{
            $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        $points = $res[0]['points'];
        if($points+$user_points<0){
            return -11;
        }
        $sql = "update user_active set points = points+$user_points where uid = $uid";
        try{
            $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return true;
    }

//    /**
//     * 根据用户id更新用户Vip等级
//     * @param $uid
//     * @param $vip
//     * @return bool
//     */
//    public function updateVipByUid($uid){
//        $sql = "select b.level from user_active a,config_vip b where a.uid = $uid and (a.total_buy > b.total_buy or a.total_buy = b.total_buy) order by b.total_buy desc limit 1";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($info)){
//            return 0;
//        }
//        $vip = $info[0]['level'];
//        $sql = "update user_active set vip = $vip where uid = $uid";
//        try{
//            $this->execute($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return true;
//    }
//
//    /**
//     * 根据uid更新姓名
//     * @param $uid
//     * @param $name
//     * @return int
//     */
//    public function updateNameByUid($uid,$name,$isModName){
////        //base64用户名称
////        $name = base64_encode($name);
//        if($isModName <= 0){
//            $sql = "update user_active set `name` = '$name' where uid = $uid";
//        }else{
//            $sql = "update user_active set `name` = '$name',is_modname = is_modname-1 where uid = $uid";
//        }
//        try{
//            $this->execute($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return true;
//    }
//
//    /**
//     * 验证姓名是否合法
//     * @param $name
//     * @return bool|int
//     */
//    public function validationName($name){
//        $sql = "select val from screen_word where val = '$name'";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($info)){
//            return 0;
//        }
//        return true;
//    }
//
//    /**
//     * 更新用户头像
//     * @param $uid
//     * @param $pic_head
//     * @return bool|int
//     */
//    public function updatePicHeadById($uid,$pic_head){
//        $sql = "update user_active set pic_head = '$pic_head' where uid = $uid";
//        try{
//            $this->execute($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return true;
//    }
//
//    /**
//     * 批量修改机器人名字更改
//     */
//    public function bot_name(){
//        $sql = "select uid from user_active where uid < 10000";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($info)){
//            return true;
//        }
//        foreach ($info as $key => $value){
//            $name = "游客".$value['uid'];
//            $sql = "update user_active set `name` = '$name' where uid = ".$value['uid'];
//            try{
//                $res = $this->execute($sql);
//            }catch (Exception $e){
//                $this->logError($e,'EMERG');
//                return false;
//            }
//            if($res === false){
//                return $res;
//            }
//        }
//        return true;
//    }
//
//    /**
//     * 获取用户签到信息
//     * @param $uid   用户id
//     * @return bool|mixed
//     */
//    public function getUserSignInInfo($uid){
//        $sql = $this->getSql(18,[
//            'uid' => $uid,
//        ]);
//
//        try
//        {
//            $res = $this->query($sql);
//        }
//        catch (Exception $e)
//        {
//            $this->logError($e,'EMERG');
//            return false;
//        }
//
//        return $res;
//    }
//
//    /**
//     * 添加用户签到信息
//     * @param $uid   用户id
//     * @return bool
//     */
//    public function addUserSignIn($uid){
//        $sql = $this->getSql(19,[
//            'uid' => $uid,
//        ]);
//        try{
//            $this->execute($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return true;
//    }

}