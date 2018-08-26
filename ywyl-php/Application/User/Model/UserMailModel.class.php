<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/6/15
 * Time: 17:45
 */

namespace User\Model;


use Think\Exception;

class UserMailModel extends BaseModel
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
                $sql = 'select id,`type`,title,status,is_give,create_time from user_mail where uid = :uid and status != 3 and create_time <= :time order by create_time desc';
                break;
            case 2:
                $sql = 'select uid,`type`,send_name,`identity`,title,`desc`,giving,giving_num,status,is_give,create_time from user_mail where id = :mail_id and status != 3 and create_time <= :time';
                break;
            case 3:
                $sql = 'update user_mail set status = 2 where id = :mail_id';
                break;
            case 4:
                $sql = 'update user_mail set is_give = 1 where id = :mail_id';
                break;
            case 5:
                $sql = 'update user_mail set status = 3 where id = :mail_id';
                break;
            case 6:
                $sql = 'select count(id) as `count` from user_mail where uid = :uid and status = 1 and create_time <= :time';
                break;
            case 7:
                $sql = 'select uid,`type`,send_name,`identity`,title,`desc`,giving,giving_num,status,is_give,create_time from user_mail where id = :mail_id and status != 3 and create_time <= :time for update';
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
     * 获取用户邮件列表
     */
    public function getList($uid,$time)
    {
        $sql = $this->getSql(1,[
            'uid' => $uid,
            'time' => $time,
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
     * 获取邮件内容
     * @param $mail_id
     * @return bool|mixed
     */
    public function getInfoById($mail_id,$time)
    {
        $sql = $this->getSql(2,[
            'mail_id' => $mail_id,
            'time' => $time,
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
     * 查看邮件
     * @param $mail_id
     * @return bool
     */
    public function lookStatus($mail_id)
    {
        $sql = $this->getSql(3,[
            'mail_id' => $mail_id
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
     * 删除邮件的附件
     * @param $mail_id
     * @return bool
     */
    public function delMailGiv($mail_id)
    {
        $sql = $this->getSql(4,[
            'mail_id' => $mail_id
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
     * 删除邮件
     * @param $mail_id
     * @return bool
     */
    public function delMailById($mail_id)
    {
        $sql = $this->getSql(5,[
            'mail_id' => $mail_id
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
     * 获取用户未读邮件的数量
     * @param $uid
     * @return bool|mixed
     */
    public function getNoLookNumByUid($uid,$time)
    {
        $sql = $this->getSql(6,[
            'uid' => $uid,
            'time' => $time,
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
     * 邮件加锁
     * @param $mail_id
     * @return bool|mixed
     */
    public function lockMailById($mail_id,$time)
    {
        $sql = $this->getSql(7,[
            'mail_id' => $mail_id,
            'time' => $time,
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