<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 14:10
 */

namespace User\Model;


use Think\Exception;

class ByDayModel extends BaseModel
{
    /**
     * 添加用户每日记录表
     * @param $uid
     * @param $date
     * @return int
     */
    public function addUserByDay($initia_num,$uid,$time){
        $this->startTrans();

        $date = date('Ymd',$time-86400);
        $sql = "select id from user_by_day where uid = $uid and `date` = $date";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->rollback();
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            $sql = "select gold_get from user_by_day where uid = $uid and `date` < $date order by `date` desc limit 1";
            try{
                $res = $this->query($sql);
            }catch (Exception $e){
                $this->rollback();
                $this->logError($e,'EMERG');
                return false;
            }
            if(empty($res)){
                $sql = "insert into user_by_day (uid,`date`) values ($uid,$date)";
                try{
                    $this->execute($sql);
                }catch (Exception $e){
                    $this->rollback();
                    $this->logError($e,'EMERG');
                    return false;
                }
            }else{
                $res = $res[0];
                $gold_get = $res['gold_get'];
                $sql = "insert into user_by_day (uid,gold_get,`date`) values ($uid,$gold_get,$date)";
                try{
                    $this->execute($sql);
                }catch (Exception $e){
                    $this->rollback();
                    $this->logError($e,'EMERG');
                    return false;
                }
            }
        }

        $i = 0;
        $date = date('Ymd',$time+(86400*$i));
        while ($i < $initia_num){
            $sql = "select id from user_by_day where uid = $uid and `date` = $date";
            try{
                $res = $this->query($sql);
            }catch (Exception $e){
                $this->rollback();
                $this->logError($e,'EMERG');
                return false;
            }
            if(empty($res)){
                $sql = "insert into user_by_day (uid,`date`) values ($uid,$date)";
                try{
                    $this->execute($sql);
                }catch (Exception $e){
                    $this->rollback();
                    $this->logError($e,'EMERG');
                    return false;
                }
            }
            $i = $i + 1;
            $date = date('Ymd',$time+(86400*$i));
        }
        $this->commit();
        return true;
    }

    /**
     * 初始化用户每日记录表
     * @param $initia_num
     * @param $uid
     * @return int
     */
    public function initalUserByday($initia_num,$uid,$time){
        $i = 0;
        $date = date('Ymd',$time+(86400*$i));
        $this->startTrans();
        while ($i < $initia_num){
//            //初始化用户炸金花普通玩法每日记录
//            $res = $this->addByDay(0,$uid,$date);
//            if($res != 1 && $res != 0){
//                $this->rollback();
//                return $res;
//            }

//            //初始化用户时时彩每日记录
//            $res = $this->addByDay(1,$uid,$date);
//            if($res != 1 && $res != 0){
//                $this->rollback();
//                return $res;
//            }

            $i = $i + 1;
            $date = date('Ymd',$time+(86400*$i));
        }
        $this->commit();
        return true;
    }

    /**
     * 添加用户每日记录表
     * @param $type
     * @param $uid
     * @param string $date
     * @return int
     */
    public function addByDay($type,$uid,$date = ""){
        if(empty($date)){
            $date = date('Ymd');
        }
        switch ($type){
//            case 0:
//                $res = $this->addUserZjhNormalByDay($uid,$date);
//                if($res === false || $res == 0){
//                    return false;
//                }
//                break;
//            case 1:
//                $res = $this->addUserSscByDay($uid,$date);
//                if($res === false || $res == 0){
//                    return false;
//                }
//                break;
            default:
                return false;
                break;
        }
        return true;
    }

//    /**
//     * 添加用户炸金花普通玩法每日记录
//     * @param $uid
//     * @param $date
//     * @return int
//     */
//    private function addUserZjhNormalByDay($uid,$date){
//        $sql = "select id from user_zjh_normal_by_day where uid = $uid and `date` = $date";
//        try{
//            $res = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($res)){
//            $sql = "insert into user_zjh_normal_by_day (uid,`date`) value ($uid,$date)";
//            try{
//                $this->execute($sql);
//            }catch (Exception $e){
//                $this->logError($e,'EMERG');
//                return false;
//            }
//            return false;
//        }else{
//            return 0;
//        }
//    }

//    /**
//     * 添加用户时时彩每日记录
//     * @param $uid
//     * @param $date
//     * @return int
//     */
//    private function addUserSscByDay($uid,$date){
//        $sql = "select id from user_ssc_by_day where uid = $uid and `date` = $date";
//        try{
//            $res = $this->query($sql);
//        }catch (Exception $e){
//            $this->logError($e,'EMERG');
//            return false;
//        }
//        if(empty($res)){
//            $sql = "insert into user_ssc_by_day (uid,`date`) value ($uid,$date)";
//            try{
//                $this->execute($sql);
//            }catch (Exception $e){
//                $this->logError($e,'EMERG');
//                return false;
//            }
//            return true;
//        }else{
//            return 0;
//        }
//    }
}