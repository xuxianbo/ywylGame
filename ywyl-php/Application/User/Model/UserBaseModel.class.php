<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/21
 * Time: 11:57
 */

namespace User\Model;


use Think\Exception;

class UserBaseModel extends BaseModel
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
                $sql = 'insert into user_base (passwd,phone,app_id,phone_type,phone_name,phone_str,md5,create_time,login_time,regis_ip,login_ip,openid,unionid) values (:passwd,:phone,:app_id,:type_phone,:name_phone,:str_phone,:md5,:create_time,:login_time,:regis_ip,:login_ip,:openid,:unionid)';
                break;
            case 2:
                $sql = 'select uid,id_card,real_name,invita_uid,agent_id,login_ip,login_time,openid,unionid,status from user_base where uid = :uid';
                break;
            case 3:
                $sql = 'select uid,id_card,real_name,invita_uid,agent_id,login_ip,login_time,openid,unionid,status from user_base where user_token = :user_token';
                break;
            case 4:
                $sql = 'select id from user_login_by_day where uid = :uid and ip = :ip and `date` = :date';
                break;
            case 5:
                $sql = 'insert into user_login_by_day (uid,ip,`date`) values (:uid,:ip,:date)';
                break;
            case 6:
                $sql = 'select uid,phone_str,user_token,invita_uid from user_base where unionid = :unionid';
                break;
            case 7:
                $sql = 'select uid,user_token,status from user_base where phone = 0 and phone_str = :phone_str and openid = :openid';
                break;
            case 8:
                $sql = 'update user_base set app_id = :app_id, phone_name = :phone_name, phone_type = :phone_type, phone_str = :phone_str where uid = :uid';
                break;
            case 9:
                $sql = 'update user_base set user_token = :user_token,login_ip = :ip,login_time = :time where uid = :uid';
                break;
            case 10:
                $sql = 'update user_base set `user_token` = :user_token where uid = :uid';
                break;
            case 11:
                $sql = 'update user_base set real_name = :real_name,id_card = :id_card where uid = :uid';
                break;
            case 12:
                $sql = "select uid,agent_id_1,agent_id_2,agent_id_3 from user_relation where uid = :uid";
                break;
            case 13:
                $sql = "insert into user_relation (uid,agent_id_1,agent_id_2,agent_id_3) values (:uid,:agent_id_1,:agent_id_2,:agent_id_3)";
                break;

            case 1014:
                $sql = 'select * from user_login_by_day where uid = :uid order by id desc limit 1';
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
     * 添加用户
     * @param $data
     * @return bool|string
     */
    public function addUser($data){
        $passwd = isset($data['passwd']) ? $data['passwd']:0;
        $phone = isset($data['phone']) ? $data['phone']:0;
        $app_id = isset($data['app_id']) ? $data['app_id']:0;
        $phone_type = isset($data['phone_type']) ? $data['phone_type']:0;
        $phone_name = isset($data['phone_name']) ? $data['phone_name']:0;
        $phone_str = isset($data['phone_str']) ? $data['phone_str']:0;
        $md5 = isset($data['md5']) ? $data['md5'] : 0;
        $create_time = isset($data['create_time']) ? $data['create_time']:0;
        $login_time = isset($data['last_time']) ? $data['last_time']:$create_time;
        $regis_ip = isset($data['regis_ip']) ? $data['regis_ip']:0;
        $login_ip = isset($data['last_ip']) ? $data['last_ip']:$regis_ip;
        $openid = isset($data['openid']) ? $data['openid']:'a';
        $unionid = isset($data['unionid']) ? $data['unionid']:'a';

        //base64用户手机名称
        $phone_name = base64_encode($phone_name);

        $sql = $this->getSql(1,[
            'passwd' => $passwd,
            'phone' => $phone,
            'app_id' => $app_id,
            'type_phone' => $phone_type,
            'name_phone' => $phone_name,
            'str_phone' => $phone_str,
            'md5' => $md5,
            'create_time' => $create_time,
            'login_time' => $login_time,
            'regis_ip' => $regis_ip,
            'login_ip' => $login_ip,
            'openid' => $openid,
            'unionid' => $unionid,
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

        if($res === false)
        {
            return $res;
        }
        else
        {
            $uid = $this->getLastInsID();
            return $uid;
        }
    }

    /**
     * 根据uid获取用户信息
     * @param $uid
     * @return bool|int
     */
    public function getInfoById($uid){
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
     * 根据token获取用户信息
     * @param $user_token
     * @return bool
     */
    public function getInfoByToken($user_token)
    {
        $sql = $this->getSql(3,[
            'user_token' => $user_token,
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
     * 添加用户登录记录
     * @param $uid
     * @param $time
     * @param $ip
     * @return bool
     */
    public function addLoginRe($uid,$time,$ip){
        $date = date('Ymd',$time);

        $sql = $this->getSql(4,[
            'uid' => $uid,
            'ip' => $ip,
            'date' => $date,
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
            $sql = $this->getSql(5,[
                'uid' => $uid,
                'ip' => $ip,
                'date' => $date,
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
        }
        return true;
    }

    /**
     * 根据openid获取用户id
     * @param $openid
     * @return bool|mixed
     */
    public function getUidByUnionid($unionid)
    {
        $sql = $this->getSql(6,[
            'unionid' => $unionid,
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
     * 根据手机识别码和手机注册ip地址查找用户信息
     * @param $phone_str
     * @param $regis_ip
     * @return bool|int|mixed
     */
    public function getInfoByIpPhStr($phone_str){
        $openid = 'a';
        $sql = $this->getSql(7,[
            'phone_str' => $phone_str,
            'openid' => $openid,
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
     *  更新用户信息
     * @param $uid
     * @param $data
     */
    public function updateUserInfo($uid,$data)
    {
        //base64用户手机名称
        $data['phone_name'] = base64_encode($data['phone_name']);

        $sql = $this->getSql(8,[
            'app_id' => $data['app_id'],
            'phone_name' => $data['phone_name'],
            'phone_type' => $data['phone_type'],
            'phone_str' => $data['phone_str'],
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
     * 更新用户token
     * @param $uid
     * @param $time
     * @param $ip
     * @return array|bool|int
     */
    public function updateTokenById($uid,$phone_str,$time,$ip){
        $user_token = md5($uid.$phone_str.$ip.$time);
        $sql = $this->getSql(9,[
            'user_token' => $user_token,
            'ip' => $ip,
            'time' => $time,
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

        return $user_token;
    }

    /**
     * 根据uid更新清空用户token
     * @param $uid
     * @return int
     */
    public function delTokenByUid($uid){
        $sql = $this->getSql(10,[
            'user_token' => "0",
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
     * 用户实名认证
     * @param $uid
     * @param $real_name
     * @param $id_card
     * @return bool
     */
    public function setUserRealName($uid,$real_name,$id_card){
        $sql = $this->getSql(11,[
            'real_name' => $real_name,
            'id_card' => $id_card,
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
     * 获取玩家代理关系
     * @param $uid
     * @return bool
     */
    public function getUserRelation($uid)
    {
        $sql = $this->getSql(12,[
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
     * 获取用户ip地址
     * @param $uid
     */
    public function getUserIp($uid)
    {
        $sql = $this->getSql(1014,[
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
     * 添加用户代理关系
     * @param $uid
     * @param $agent_id_1
     * @param $agent_id_2
     * @param $agent_id_3
     * @return bool
     */
    public function addUserRelation($uid,$agent_id_1,$agent_id_2,$agent_id_3)
    {
        $sql = $this->getSql(13,[
            'uid' => $uid,
            'agent_id_1' => $agent_id_1,
            'agent_id_2' => $agent_id_2,
            'agent_id_3' => $agent_id_3,
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

//    /**
//     * 根据手机号获取用户信息
//     * @param $phone
//     * @return bool
//     */
//    public function getMd5PwdByPhone($phone){
//        $sql = "select uid,passwd,md5,phone_str,user_token from user_base where phone = $phone";
//        try{
//            $info = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($info)){
//            return 0;
//        }
//        return $info[0];
//    }
//
//    /**
//     * 更新用户密码
//     * @param $uid
//     * @param $passwd
//     * @param $md5
//     * @param $is_login 是否是登录中
//     * @return bool
//     */
//    public function updatePWMd5ById($uid,$passwd,$md5,$is_login){
//        if($is_login){
//            $sql = "update user_base set passwd = '$passwd',md5 = '$md5' where uid = $uid";
//        }else{
//            $sql = "update user_base set passwd = '$passwd',md5 = '$md5',user_token = 0 where uid = $uid";
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
//     * 绑定用户账号
//     * @param $uid
//     * @param $data
//     * @return bool
//     */
//    public function updateUser($uid,$data){
//        $phone = $data['phone'];
//        $passwd = $data['passwd'];
//        $md5 = $data['md5'];
//        $sql = "update user_base set phone = $phone,passwd = '$passwd',md5 = $md5 where uid = $uid";
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
//     * 更新用户手机号
//     * @param $uid
//     * @param $data
//     * @return bool
//     */
//    public function updatePhone($uid,$phone){
//        $sql = "update user_base set phone = $phone where uid = $uid";
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
//     * 获取用户游戏次数
//     * @param $uid
//     * @param string $game_id
//     * @return array|bool
//     */
//    public function getUserTotalInfo($uid,$game_id = "")
//    {
//        if(empty($game_id))
//        {
//            $gameids = $this->getConfigHouseGid();
//        }
//        else
//        {
//            $gameids = [['gameid' => $game_id]];
//        }
//
//        $user_total = [];
//        $i = 0;
//        foreach ($gameids as $val)
//        {
//            $res = $this->getUserTotal($uid,$val['gameid']);
//            if($res === false)
//            {
//                return false;
//            }
//            else if(empty($res))
//            {
//                $user_total[$val['gameid']]['game_total'] = 0;
//                $user_total[$val['gameid']]['game_win'] = 0;
//                $res = $this->addGameDataByUid($uid,$val['gameid']);
//                if($res === false)
//                {
//                    return false;
//                }
//            }
//            else
//            {
//                $res = $res[0];
//                $user_total[$val['gameid']]['game_total'] = $res['game_total'];
//                $user_total[$val['gameid']]['game_win'] = $res['game_win'];
//            }
//            $i++;
//        }
//        return $user_total;
//    }
//
//    /**
//     * 添加用户游戏记录
//     * @param $uid
//     * @param $game_id
//     * @return bool
//     */
//    public function addGameDataByUid($uid,$game_id){
//        $sql = "insert into game_data (gid,uid,game_total,game_win) values ($game_id,$uid,0,0)";
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
//     * 查找用户游戏次数
//     * @param $uid
//     * @param $game_id
//     * @return bool|mixed
//     */
//    public function getUserTotal($uid,$game_id){
//        $sql = "select game_total,game_win from game_data where uid = $uid and gid = $game_id";
//        try{
//            $res = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return $res;
//    }
//
//    /**
//     * 查询当前所拥有游戏
//     * @return bool|mixed
//     */
//    public function getConfigHouseGid(){
//        $sql = "select distinct(gameid) from config_house where gameid > 0";
//        try{
//            $res = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        return $res;
//    }

}