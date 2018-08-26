<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/3/28
 * Time: 11:07
 */

namespace User\Model;


use Think\Exception;

class GameModel extends BaseModel
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
            case 5:
                $sql = 'select gameid,grade,num,gold_type,gold_min,bottom,people,seat_max,cards_num from config_house';
                break;
            case 6:
                $sql = 'select uid,gold_win from user_bjl_aaa where create_time >=:start_time and create_time <= :end_time and uid >= :start_id and uid < :end_id and `check_type` = 0';
                break;
            case 7:
                $sql = 'select uid,gold_win from user_ssc_aaa where create_time >=:start_time and create_time <= :end_time and uid >= :start_id and uid < :end_id';
                break;
            case 8:
                $sql = 'select gameid,`type`,grade,num,gold_type,gold_min,bottom,people,seat_max,cards_num from config_house where gameid = :game_id';
                break;

            //炸金花游戏记录
            case 10101:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_zjh_record where id < :start_id and `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10102:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_zjh_record where id < :start_id and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10103:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_zjh_record where `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10104:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_zjh_record where (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;

            //牛牛游戏记录
            case 10201:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_ox_record where id < :start_id and `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10202:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_ox_record where id < :start_id and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10203:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_ox_record where `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;
            case 10204:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid5,uid6,uid7,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid5_gold,uid6_gold,uid7_gold,uid1_name,uid2_name,uid3_name,uid4_name,uid5_name,uid6_name,uid7_name,create_time';
                $sql = "select $sel from game_ox_record where (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4 or uid5 = :uid5 or uid6 = :uid6 or uid7 = :uid7) order by id desc limit :page_num";
                break;

            //百家乐游戏记录
            case 10300:
                $sql = 'select a.uid,b.name,gain_total as gain,a.create_time from game_bjl_record a,user_active b where a.qh = :qh and a.uid = b.uid and a.uid = :uid'; //根据qh和uid获取到玩家下注信息
                break;
            case 10304:
                $sql = 'select id,qh from game_bjl_record where uid = :uid order by id desc limit :page_num'; //根据时间获取一定范围内指定玩家的下注信息
                break;
            case 10305:
                $sql = 'select id,qh from game_bjl_record where uid = :uid and id < :start_id order by id desc limit :page_num'; //根据时间获取一定范围内指定玩家的下注信息
                break;
            case 10306:
                $sql = 'select banker,banker_name,banker_gain,uid1,uid2,uid3,uid4,uid5,gain1,gain2,gain3,gain4,gain5,name1,name2,name3,name4,name5 from game_bjl_reward where qh = :qh'; //根据期号获取该局的庄家和前5名玩家的游戏情况
                break;

            //百人推筒子游戏记录
            case 10400:
                $sql = 'select a.uid,b.name,gain_total as gain,a.create_time from game_ttz_record a,user_active b where a.qh = :qh and a.uid = b.uid and a.uid = :uid'; //根据qh和uid获取到玩家下注信息
                break;
            case 10404:
                $sql = 'select id,qh from game_ttz_record where uid = :uid order by id desc limit :page_num'; //根据时间获取一定范围内指定玩家的下注信息
                break;
            case 10405:
                $sql = 'select id,qh from game_ttz_record where uid = :uid and id < :start_id order by id desc limit :page_num'; //根据时间获取一定范围内指定玩家的下注信息
                break;
            case 10406:
                $sql = 'select banker,banker_name,banker_gain,uid1,uid2,uid3,uid4,uid5,gain1,gain2,gain3,gain4,gain5,name1,name2,name3,name4,name5 from game_ttz_reward where qh = :qh'; //根据期号获取该局的庄家和前5名玩家的游戏情况
                break;

            //五子棋游戏记录
            case 10502:
                $sel = 'id,type,grade,uid1,uid2,uid1_gold,uid2_gold,uid1_name,uid2_name,create_time';
                $sql = "select $sel from game_wzq_record where id < :start_id and (uid1 = :uid1 or uid2 = :uid2) order by id desc limit :page_num";
                break;
            case 10504:
                $sel = 'id,type,grade,uid1,uid2,uid1_gold,uid2_gold,uid1_name,uid2_name,create_time';
                $sql = "select $sel from game_wzq_record where (uid1 = :uid1 or uid2 = :uid2) order by id desc limit :page_num";
                break;

            //麻将游戏记录
            case 10601:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid1_name,uid2_name,uid3_name,uid4_name,create_time';
                $sql = "select $sel from game_mj_record where id < :start_id and `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4) order by id desc limit :page_num";
                break;
            case 10602:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid1_name,uid2_name,uid3_name,uid4_name,create_time';
                $sql = "select $sel from game_mj_record where id < :start_id and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4) order by id desc limit :page_num";
                break;
            case 10603:
                $sel = 'id,grade,uid1,uid2,uid3,uid4,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid1_name,uid2_name,uid3_name,uid4_name,create_time';
                $sql = "select $sel from game_mj_record where `type` = :game_type and (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4) order by id desc limit :page_num";
                break;
            case 10604:
                $sel = 'id,type,grade,uid1,uid2,uid3,uid4,uid1_gold,uid2_gold,uid3_gold,uid4_gold,uid1_name,uid2_name,uid3_name,uid4_name,create_time';
                $sql = "select $sel from game_mj_record where (uid1 = :uid1 or uid2 = :uid2 or uid3 = :uid3 or uid4 = :uid4) order by id desc limit :page_num";
                break;

            //牛牛牌型活动
            case 20000: //获取用户牌型最后一次的兑换时间
                $sql = 'select create_time from activity_ox_record where uid = :uid and check_type = :check_type order by id desc limit 1';
                break;
            case 20001: //获取用户牌型在兑换后底注最大的一次
                $sql = 'select id,bottom,pai_1,pai_2,pai_3,pai_4,pai_5 from user_prize_top where uid = :uid and check_type = :check_type and create_time > :create_time order by bottom desc limit 1';
                break;
            case 20002: //增加用户领取牌型记录
                $sql = 'insert into activity_ox_record (uid,prize_id,check_type,create_time) values (:uid,:prize_id,:check_type,:create_time)';
                break;
            case 20003: //完善用户领取牌型记录
                $sql = 'update activity_ox_record set money = :gold where uid = :uid and create_time = :create_time';
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
     * 获取游戏房间配置
     * @param $game_id
     * @return bool|mixed
     */
    public function getGameHouseConfig($game_id)
    {
        if(empty($game_id))
        {
            $sql = $this->getSql(5);
        }
        else
        {
            $sql = $this->getSql(8,[
                'game_id' => $game_id
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
     * 获取根据时间和uid返回获取百家乐的彩金记录
     * @param $start_time
     * @param $end_time
     * @param $start_id
     * @param $end_id
     * @return bool|mixed
     */
    public function getBjlInfoByTimeUid($start_time,$end_time,$start_id,$end_id)
    {
        $sql = $this->getSql(6,[
            'start_time' => $start_time,
            'end_time' => $end_time,
            'start_id' => $start_id,
            'end_id' => $end_id,
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
     * 获取根据时间和uid返回获取时时彩的彩金记录
     * @param $start_time
     * @param $end_time
     * @param $start_id
     * @param $end_id
     * @return bool|mixed
     */
    public function getSscInfoByTimeUid($start_time,$end_time,$start_id,$end_id)
    {
        $sql = $this->getSql(7,[
            'start_time' => $start_time,
            'end_time' => $end_time,
            'start_id' => $start_id,
            'end_id' => $end_id,
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
     * 获取玩家游戏记录
     * @param $uid
     * @param $game_id
     * @param $game_type
     * @param $start_num
     * @param $page_num
     * @return bool|mixed
     */
    public function getUserGameRecordByUidGid($uid,$game_id,$game_type,$start_id,$page_num)
    {
        $code = 0;
        $time = time() - 30 * 86400;
        switch ($game_id)
        {
            case 0:
                if(empty($start_id))
                {
                    $code = 10304;
                }
                else
                {
                    $code = 10305;
                }
                $sql = $this->getSql($code,[
                    'uid' => $uid,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                    'time' => $time,
                ]);
                break;
            case 1:
                if($game_type == -1 && empty($start_id))
                {
                    $code = 10104;
                }
                else if($game_type == -1 && !empty($start_id))
                {
                    $code = 10102;
                }
                else if($game_type != -1 && empty($start_id))
                {
                    $code = 10103;
                }
                else if($game_type != -1 && !empty($start_id))
                {
                    $code = 10101;
                }
                else
                {
                    return false;
                }
                $sql = $this->getSql($code,[
                    'game_type' => $game_type,
                    'uid1' => $uid,
                    'uid2' => $uid,
                    'uid3' => $uid,
                    'uid4' => $uid,
                    'uid5' => $uid,
                    'uid6' => $uid,
                    'uid7' => $uid,
                    'time' => $time,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                ]);
                break;
            case 2:
                if($game_type == -1 && empty($start_id))
                {
                    $code = 10204;
                }
                else if($game_type == -1 && !empty($start_id))
                {
                    $code = 10202;
                }
                else if($game_type != -1 && empty($start_id))
                {
                    $code = 10203;
                }
                else if($game_type != -1 && !empty($start_id))
                {
                    $code = 10201;
                }
                else
                {
                    return false;
                }
                $sql = $this->getSql($code,[
                    'game_type' => $game_type,
                    'uid1' => $uid,
                    'uid2' => $uid,
                    'uid3' => $uid,
                    'uid4' => $uid,
                    'uid5' => $uid,
                    'uid6' => $uid,
                    'uid7' => $uid,
                    'time' => $time,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                ]);
                break;
            case 4:
                if(empty($start_id))
                {
                    $code = 10404;
                }
                else
                {
                    $code = 10405;
                }
                $sql = $this->getSql($code,[
                    'uid' => $uid,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                    'time' => $time,
                ]);
                break;
            case 5:
                if(empty($start_id))
                {
                    $code = 10504;
                }
                else
                {
                    $code = 10502;
                }
                $sql = $this->getSql($code,[
                    'game_type' => $game_type,
                    'uid1' => $uid,
                    'uid2' => $uid,
                    'time' => $time,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                ]);
                break;
            case 6:
                if($game_type == -1 && empty($start_id))
                {
                    $code = 10604;
                }
                else if($game_type == -1 && !empty($start_id))
                {
                    $code = 10602;
                }
                else if($game_type != -1 && empty($start_id))
                {
                    $code = 10603;
                }
                else if($game_type != -1 && !empty($start_id))
                {
                    $code = 10601;
                }
                else
                {
                    return false;
                }
                $sql = $this->getSql($code,[
                    'game_type' => $game_type,
                    'uid1' => $uid,
                    'uid2' => $uid,
                    'uid3' => $uid,
                    'uid4' => $uid,
                    'time' => $time,
                    'start_id' => $start_id,
                    'page_num' => $page_num,
                ]);
                break;
            default:
                return false;
                break;
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
     * 根据qh和uid获取百家乐游戏记录
     * @param $qh
     * @param $uid
     */
    public function getUserBjlInfo($qh,$uid,$game_id)
    {
        switch ($game_id)
        {
            case 0:
                $sql = $this->getSql(10300,[
                    'qh' => $qh,
                    'uid' => $uid,
                ]);
                break;
            case 4:
                $sql = $this->getSql(10400,[
                    'qh' => $qh,
                    'uid' => $uid,
                ]);
                break;
            default:
                return false;
                break;
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
     * 根据qh获取百家乐游戏记录
     * @param $qh
     */
    public function getBjlInfoByQh($qh,$game_id)
    {
        switch ($game_id)
        {
            case 0:
                $sql = $this->getSql(10306,[
                    'qh' => $qh,
                ]);
                break;
            case 4:
                $sql = $this->getSql(10406,[
                    'qh' => $qh,
                ]);
                break;
            default:
                return false;
                break;
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

    /* 牛牛牌型活动开始 */

    /**
     * 获取用户牌型最后一次的兑换时间
     * @param $uid
     * @param $check_type
     * @return bool|mixed
     */
    public function getOxActivityTime($uid,$check_type)
    {
        $sql = $this->getSql(20000,[
            'uid' => $uid,
            'check_type' => $check_type,
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
     * 获取用户牌型在兑换后底注最大的一次
     * @param $uid
     * @param $check_type
     * @return bool|mixed
     */
    public function getOxActivityBottom($uid,$check_type,$create_time)
    {
        $sql = $this->getSql(20001,[
            'uid' => $uid,
            'check_type' => $check_type,
            'create_time' => $create_time,
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
     * 增加用户领取牌型记录
     * @param $uid
     * @param $prize_id
     * @param $check_type
     * @param $gold
     * @param $create_time
     */
    public function addOxActivityRecord($uid,$prize_id,$check_type,$create_time)
    {
            $sql = $this->getSql(20002,[
                'uid' => $uid,
                'prize_id' => $prize_id,
                'check_type' => $check_type,
                'create_time' => $create_time,
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
     * 完善用户领取牌型记录
     * @param $uid
     * @param $prize_id
     * @param $check_type
     * @param $gold
     * @param $create_time
     */
    public function addOxActivityRecordGold($uid,$gold,$create_time)
    {
        $sql = $this->getSql(20003,[
            'uid' => $uid,
            'gold' => $gold,
            'create_time' => $create_time,
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

    /* 牛牛牌型活动结束 */
}
