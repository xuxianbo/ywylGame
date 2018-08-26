<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/19
 * Time: 17:55
 */

namespace User\Controller;


use Think\Log;
use User\Model\Code;
use User\Model\RaffleModel;
use User\Model\ServerModel;
use User\Model\SignInModel;
use User\Model\UserActiveModel;
use User\Model\UserBaseModel;
use User\Model\UserMailModel;
use User\Model\UserRedis;

class UserController extends BaseController
{
    /**
     * 游客登录
     * @param string $data
     */
    public function visitorLogin($data = ""){
        //获取用户数据
        if(empty($data))
        {
            $data = I('');
        }

        //数据验证
        if(!isset($data['app_id']) || empty($data['app_id']))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要app_id'],'statusCode'=>-101]);
        }
        if(!isset($data['phone_type']) || empty($data['phone_type']))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_type'],'statusCode'=>-101]);
        }
        if(!isset($data['phone_name']) || empty($data['phone_name']))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_name'],'statusCode'=>-101]);
        }
        if(!isset($data['phone_str']) || empty($data['phone_str']))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_str'],'statusCode'=>-101]);
        }

        $data['regis_ip'] = $_SERVER["REMOTE_ADDR"];
        $data['app_id'] = str_replace(C('User.Regular_data'),"",$data['app_id']);
        $data['phone_type'] = str_replace(C('User.Regular_data'),"",$data['phone_type']);
        $data['phone_name'] = str_replace(C('User.Regular_data'),"",$data['phone_name']);
        $data['phone_str'] = str_replace(C('User.Regular_data'),"",$data['phone_str']);

        //验证是否有该游客账号
        $userBaseModel = new UserBaseModel();
        $user_base = $userBaseModel->getInfoByIpPhStr($data['phone_str']);
        if($user_base === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
        }
        else if(!empty($user_base))
        {
            //判断如果用户的状态为1 说明被封号
            if($user_base[0]['status'] == 1)
            {
                $this->error_return(-20004);
            }

            $user_base = $user_base[0];
            $uid = $user_base['uid'];
        }
        else
        {
            //游客注册ip限制
            $res = $this->Iplimit($data['regis_ip']);
            if($res === false)
            {
                $this->error_return(-1001002);
            }

            $user_data['create_time']= $user_data['last_time'] = date(time());
            $user_data['last_ip'] = $user_data['regis_ip'] = $data['regis_ip'];
            $user_data['app_id'] = str_replace(C('User.Regular_data'),"",$data['app_id']);
            $user_data['phone_type'] = str_replace(C('User.Regular_data'),"",$data['phone_type']);
            $user_data['phone_name'] = str_replace(C('User.Regular_data'),"",$data['phone_name']);
            $user_data['phone_str'] = str_replace(C('User.Regular_data'),"",$data['phone_str']);

            //没有找到则需要注册游客账号
            $userBaseModel->startTrans();
            $uid = $userBaseModel->addUser($user_data);
            if($uid === false || empty($uid))
            {
                $userBaseModel->rollback();
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'注册失败'],'statusCode'=>-20001]);
            }

            $userActiveModel = new UserActiveModel();
            $res = $userActiveModel->addUser($uid);
            if($res === false)
            {
                $userBaseModel->rollback();
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'注册失败'],'statusCode'=>-20001]);
            }
            $userBaseModel->commit();
        }

        //更新用户token
        $last_time = time();
        $last_ip = $_SERVER["REMOTE_ADDR"];
        $user_token = $userBaseModel->updateTokenById($uid,$data['phone_str'],$last_time,$last_ip);
        if($user_token === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户token更新失败'],'statusCode'=>-20002]);
        }

        //把用户token写入redis
        $redisModel = new UserRedis();
        $redis_res = $redisModel->delRedis(1,'',$user_base['user_token']);
        if($redis_res === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
        }

        $redis_res = $redisModel->setRedis(1,$uid,C('REDIS_TIMEOUT'),'',$user_token);
        if($redis_res === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
        }

        $user_active = $this->getUserActiveRedis($uid,1);
        if($user_active == -2)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
        }

        $res = [
            "user_token" => $user_token,
            "uid" => $uid,
        ];
        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
    }

    /**
     * 微信 注册 | 登录
     * @param string $code
     * @param string $app_id
     * @param string $phone_type
     * @param string $phone_name
     * @param string $phone_str
     * @param string $wechar_id
     */
    public function wecharLogin($code = "", $app_id = "", $phone_type = "", $phone_name = "", $phone_str = "",$wechar_id = "")
    {
        if(empty($app_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要app_id'],'statusCode'=>-101]);
        }
        if(empty($phone_type))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_type'],'statusCode'=>-101]);
        }
        if(empty($phone_name))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_name'],'statusCode'=>-101]);
        }
        if(empty($phone_str))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_str'],'statusCode'=>-101]);
        }
        if(empty($wechar_id))
        {
            $wechar_id = 0;
        }

        //获取前端传递的code
        $config = C('new_app_wechar_config');

        if(!isset($config[$wechar_id]))
        {
            $this->error_return(-1001051);
        }

        $config = $config[$wechar_id];

        //通过code获取access_token
        $url = $config['url_1'].'appid='.$config['appid'].'&secret='.$config['secret'].'&code='.$code.'&grant_type=authorization_code';
        $res = $this->curl_get_https($url);
        if($res === false)
        {
            $this->error_return(-151);
        }
        else if(isset($res['errcode']))
        {
            $this->wechar_http_error_log($res,$url);
        }

        //整理返回的信息
        $access_token = $res['access_token'];
        $openid = $res['openid'];

        //根据access_token 获取用户信息
        $url = $config['url_4'].'access_token='.$access_token.'&openid='.$openid;
        $res = $this->curl_get_https($url);
        if($res === false)
        {
            $this->error_return(-151);
        }
        else if(isset($res['errcode']))
        {
            $this->wechar_http_error_log($res,$url);
        }

        //整理返回的信息
        $nickname = $res['nickname'];
        $nickname = mb_substr($nickname,0,16,'utf-8');
        $sex = $res['sex'];
        $headimgurl = $res['headimgurl'];
        $unionid = $res['unionid'];

        if(strlen($headimgurl) > 150)
        {
            $headimgurl = '';
        }

        $data = [];
        $data['app_id'] = str_replace(C('User.Regular_data'),"",$app_id);
        $data['phone_type'] = str_replace(C('User.Regular_data'),"",$phone_type);
        $data['phone_name'] = str_replace(C('User.Regular_data'),"",$phone_name);
        $data['phone_str'] = str_replace(C('User.Regular_data'),"",$phone_str);


        //根据unionid查询数据库中是否存在账号
        $userBaseModel = new UserBaseModel();
        $userActiveModel = new UserActiveModel();
        $user_info = $userBaseModel->getUidByUnionid($unionid);
        $userBaseModel->startTrans();
        if($user_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($user_info))
        {
            $data['regis_ip'] = $data['last_ip'] = $_SERVER["REMOTE_ADDR"];
            $data['create_time'] = $data['last_time'] = time();
            $data['openid'] = $openid;
            $data['unionid'] = $unionid;

            //判断ip一小时内注册了多少账号
            $res = $this->getWecharIplimit($data['regis_ip']);
            if($res === false)
            {
                $this->error_return(-1001004);
            }

            //创建账户
            $uid = $userBaseModel->addUser($data);
            if($uid === false || empty($uid))
            {
                $userBaseModel->rollback();
                $this->error_return(-20001);
            }

            $res = $userActiveModel->addUser($uid,$nickname,$sex,$headimgurl);
            if($res === false)
            {
                Log::write("用户名称是:","ERR");
                $this->error_return(-20001);
            }
        }
        else
        {
            //判断如果用户的状态为1 说明被封号
            if($user_info[0]['status'] == 1)
            {
                $this->error_return(-20004);
            }

            $uid = $user_info[0]['uid'];

            //更新用户的姓名和头像性别
            $res = $userActiveModel->setWecherLoginUserInfo($uid,$nickname,$headimgurl,$sex);
            if($res === false)
            {
                $this->error_return(-1);
            }

            //如果手机识别码为数字补全用户信息
            if(is_numeric($user_info[0]['phone_str']))
            {
                //补全用户的基本信息
                $res = $userBaseModel->updateUserInfo($uid,$data);
                if($res === false)
                {
                    $userBaseModel->rollback();
                    $this->error_return(-1);
                }

                //补全用户的默认信息
                $res = $userActiveModel->updateUserInfo($uid);
                if($res === false)
                {
                    $userBaseModel->rollback();
                    $this->error_return(-1);
                }

                //构建用户代理信息
                $agent_id_1 = $user_info[0]['invita_uid'];
                $agent_id_2 = 1;
                $agent_id_3 = 1;
                $res = $userBaseModel->getUserRelation($uid);
                if($res === false)
                {
                    $userBaseModel->rollback();
                    $this->error_return(-1);
                }
                else if(empty($res))
                {
                    $res = $userBaseModel->getInfoById($agent_id_1);
                    if($res === false)
                    {
                        $userBaseModel->rollback();
                        $this->error_return(-1);
                    }
                    if($res[0]['invita_uid'] > 1)
                    {
                        $agent_id_2 = $res[0]['invita_uid'];
                        $res = $userBaseModel->getInfoById($agent_id_2);
                        if($res === false)
                        {
                            $userBaseModel->rollback();
                            $this->error_return(-1);
                        }
                        if($res[0]['invita_uid'] > 1)
                        {
                            $agent_id_3 = $res[0]['invita_uid'];
                        }
                    }

                    $res = $userBaseModel->addUserRelation($uid,$agent_id_1,$agent_id_2,$agent_id_3);
                    if($res === false)
                    {
                        $userBaseModel->rollback();
                        $this->error_return(-1);
                    }
                }
            }
        }

        //更新用户token
        $last_time = time();
        $last_ip = $_SERVER["REMOTE_ADDR"];
        $user_token = $userBaseModel->updateTokenById($uid,$data['phone_str'],$last_time,$last_ip);
        if($user_token === false)
        {
            $userBaseModel->rollback();
            $this->error_return(-20002);
        }

        //把用户token写入redis
        $redisModel = new UserRedis();
        if(!is_numeric($user_info[0]['user_token']))
        {
            $redis_res = $redisModel->delRedis(1,'',$user_info[0]['user_token']);
            if($redis_res === false)
            {
                $userBaseModel->rollback();
                $this->error_return(-2);
            }
        }

        $redis_res = $redisModel->setRedis(1,$uid,C('REDIS_TIMEOUT'),'',$user_token);
        if($redis_res === false)
        {
            $userBaseModel->rollback();
            $this->error_return(-2);
        }

        $user_active = $this->getUserActiveRedis($uid,1);
        if($user_active == -2)
        {
            $userBaseModel->rollback();
            $this->error_return(-2);
        }

        $userBaseModel->commit();

        $res = [
            "user_token" => $user_token,
            "uid" => $uid,
        ];
        $this->error_return(1,$res);
    }

    /**
     * 测试登录方法
     */
    public function testLogin(){

        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'访问成功'],'statusCode'=>1]);
    }

    /**
     * 创建用户每日表
     * @param string $user_token
     * @param string $uid
     */
    public function createByDay($user_token = "",$uid = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
        }

        $time = time();
        $userBaseModel = new UserBaseModel();

        //添加用户登录操作
        $ip = get_client_ip_new();
        $res = $userBaseModel->addLoginRe($uid,$time,$ip);
        if($res === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'添加用户登录操作失败'],'statusCode'=>-1]);
        }

        //更新用户redis
        $this->getUserActiveRedis($uid);
        $userRedis = new UserRedis();
        $res = $userRedis->setRedis(1,$uid,C('REDIS_TIMEOUT'),'',$user_token);
        if($res === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'更新用户redis失败'],'statusCode'=>-2]);
        }

        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'创建成功','server_time' => time()],'statusCode'=>1]);
    }

    //显示免责声明
    public function disclaimer(){
        $this->display('disclaimer');
    }

    /**
     * 获取用户信息
     * @param string $user_token
     * @param string $uid
     * @param string $game_id
     */
    public function getUserInfo($user_token = "",$uid = ""){
        //验证token是否成效
        $user_base = $this->getUserBaseByToken($user_token);

        $userBaseModel = new UserBaseModel();
        if($uid == $user_base['uid'])
        {
            $user_base = $this->getUserBaseByToken($user_token);
        }
        else
        {
            $user_base = $userBaseModel->getInfoById($uid);
            if($user_base === false)
            {
                $this->error_return(-1);
            }
            else if(empty($user_base))
            {
                $this->error_return(-10008);
            }
            else
            {
                $user_base = $user_base[0];
            }
        }

        $user_active = $this->getUserActiveRedis($uid);
        $info = [];
        foreach ($user_base as $k => $v)
        {
            $info[$k] = $v;
        }
        foreach ($user_active as $k => $v)
        {
            $info[$k] = $v;
        }

        //获取用户未读邮件数
        $userMailModel = new UserMailModel();

        $mail_info = $userMailModel->getNoLookNumByUid($uid,time());
        if($mail_info === false)
        {
            $this->error_return(-1);
        }

        //获取用户邮件数
        $mail_info = $mail_info[0];
        if(empty($mail_info['count']))
        {
            $info['mail_count'] = 0;
        }
        else
        {
            $info['mail_count'] = $mail_info['count'];
        }

        $res = $userBaseModel->getUserIp($uid);
        if($res === false)
        {
            $this->error_return(-1);
        }
        else if(empty($res))
        {
            $info['user_ip'] = '0.0.0.0';
        }
        else
        {
            $info['user_ip'] = $res[0]['ip'];
        }

        $this->ajaxReturn(['status' => 'true','data' => $info,'statusCode' => 1]);
    }

    /**
     * 获取排行榜数据
     * @param string $user_token
     * @param string $uid
     */
    public function getRank($user_token = "",$uid = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        //将数据存入redis
        $userBaseRedis = new UserRedis();
        $goldRank = $userBaseRedis->getRedis(5);
        if(!$goldRank)
        {
            $goldRank = [];
        }
        else if(empty($goldRank))
        {
            $goldRank = [];
        }

        $res = [];
        $res['moneyRank'] = $goldRank;
        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
    }

    /**
     * 更新个人性别和签名
     * @param string $user_token
     * @param int $uid
     * @param string $sex
     * @param string $sign
     */
    public function updateUserSexSign($user_token = "",$uid = "",$sex = "",$sign = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $userActiveModel = new UserActiveModel();
        $user_active = $userActiveModel->getInfoById($user_base['uid']);
        if($user_active === false)
        {
            $this->error_return(-1);
        }
        else if ($user_active == 0)
        {
            $this->error_return(-10008);
        }

        //验证签名
        if($sign != $user_active['sign'] && $sign != "")
        {
            //验证是否包含非法字符
//            foreach (C('User.Regular_data') as $val)
//            {
//                if(strpos($sign,$val) !== false){
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'签名包含非法字符'],'statusCode'=>-10003]);
//                }
//            }

            //验证签名是否有屏蔽词
            $res = $this->validationString($sign);
            if($res === false)
            {
                $this->error_return(-10009);
            }

            $sign = base64_encode($sign);
        }
        else
        {
            $sign = "";
        }

        if(!is_numeric($sex) | $sex < 0 || $sex > 1)
        {
            $this->error_return(-10011);
        }

        if($sex == '' && $sign == '')
        {
            $this->error_return(-1);
        }

        //判断用户头像是否为默认值
        if(!is_numeric($user_active['pic_head']))
        {
            $res = $userActiveModel->updateSexSignByUid($user_base['uid'],$sex,$sign,false);
        }
        else
        {
            $res = $userActiveModel->updateSexSignByUid($user_base['uid'],$sex,$sign,true);
        }
        if($res === false)
        {
            $this->error_return(-1);
        }

        //更新用户redis
        $user_active = $this->getUserActiveRedis($user_base['uid']);

        $this->ajaxReturn(['status' => 'true','data' => [
            'message'=>'更新成功',
            'sex' => $user_active['sex'],
            'sign' => $user_active['sign']
        ],'statusCode' => 1]);
    }

    /**
     * 用户抽奖
     * @param string $user_token
     * @param string $uid
     */
    public function raffle($user_token = "",$uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        //检测用户是否拥有抽奖次数
        $userActiveModel = new UserActiveModel();
        $user_active = $userActiveModel->getInfoById($uid);
        if ($user_active == -1)
        {
            $this->error_return(-1);
        }
        if ($user_active == 0)
        {
            $this->error_return(-10008);
        }
        if ($user_active['reffle_num'] == 0)
        {
            $this->error_return(-10012);
        }

        //获取奖品信息
        $raffleModel = new RaffleModel();
        $raffle_info = $raffleModel->getAwardInfo();
        if ($raffle_info == false)
        {
            $this->error_return(-1);
        }
        if (empty($raffle_info))
        {
            $this->error_return(-10013);
        }

        //随机获取奖品
        $award = $raffleModel->getRandAward($raffle_info);
        if($award === false)
        {
            $this->error_return(-1);
        }

        $raffle_info = $raffle_info[$award];
        $award_type = $raffle_info['award_type'];
        $award_num = $raffle_info['award_num'];

        //开始给奖
        $userActiveModel->startTrans();
        $res = $userActiveModel->updateIsReffleByUid($uid);
        if ($res === false)
        {
            $userActiveModel->rollback();
            $this->error_return(-10014);
        }

        switch ($award_type)
        {
            case 0:
                $res = $userActiveModel->updateGoldByUid($uid, $award_num);
                if ($res === false)
                {
                    $userActiveModel->rollback();
                    $this->error_return(-10015);
                }
                else
                {
                    $award_num = $award_num.'金币';
                }
                break;
            case 1:
                $res = $userActiveModel->updateMoneyByUid($uid, $award_num);
                if ($res === false)
                {
                    $userActiveModel->rollback();
                    $this->error_return(-10015);
                }
                else
                {
                    $award_num = $award_num.'money';
                }
                break;
            default:
                $userActiveModel->rollback();
                $this->error_return(-10015);
                break;
        }

        $userActiveModel->commit();

        //更新玩家redis
        $user_active = $this->getUserActiveRedis($uid);

        $this->ajaxReturn(['status' => 'true', 'data' =>
            [
            'message' => '抽奖完成',
            'raffle_info' => $raffle_info,
            'user_gold' => $user_active['gold'],
            'user_money' => $user_active['money'],
            'reffle_num' => $user_active['reffle_num'],
        ], 'statusCode' => 1]);
    }

    /**
     * 登出游戏
     * @param string $user_token
     * @param string $uid
     */
    public function loginOut($user_token = "",$uid = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        $userRedis = new UserRedis();

        //删除用户redis数据
        $res = $userRedis->delRedis(1,'',$user_token);
        if($res === true)
        {
            //清空用户token
            $userBaseModel = new UserBaseModel();
            $userBaseModel->delTokenByUid($user_base['uid']);
        }

        $userRedis->delRedis(2,$user_base['uid']);
        $userRedis->delRedis(3,$user_base['uid']);
        $userRedis->delRedis(4,$user_base['uid']);

        $this->ajaxReturn(['status'=>'false','data'=>['message'=>'退出成功'],'statusCode'=>-1]);
    }

    /**
     * 获取服务器信息
     * @param string $user_token 用户密钥
     * @param int $uid 用户密钥对应的用户id
     * ajax return mixed message 状态说明
     * ajax return array server_info 服务器信息
     */
    public function getServerInfo($user_token ="",$uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        //获取服务器信息
        $serverModel = new ServerModel();
        $serverInfo = $serverModel->getInfo();

        if($serverInfo === false)
        {
            $this->error_return(-1);
        }
        else if($serverInfo == 0)
        {
            $this->error_return(-10016);
        }
        else
        {
            $this->success_return(['server_info'=>$serverInfo]);
        }

    }

    /**
     * 获取抽奖奖品列表
     * @param string $user_token   用户密钥
     * @param int $uid   用户id
     */
    public function getRaffleInfo($user_token = "",$uid = ""){
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        //获取奖品信息
        $raffleModel = new RaffleModel();
        $raffle_info = $raffleModel->getAwardInfo();
        if ($raffle_info == false)
        {
            $this->error_return(-1);
        }
        if (empty($raffle_info))
        {
            $this->error_return(-10013);
        }

        $this->success_return(['raffle_info' => $raffle_info]);
    }

    /**
     * 实名认证
     * @param string $user_token
     * @param string $uid
     * @param string $real_name
     * @param string $card_id
     */
    public function realNameAuthentication($user_token = "",$uid = "",$real_name = "",$card_id = "")
    {
        if($real_name == "") $this->error_return(-10017);
        if($card_id == "") $this->error_return(-10018);

        //判断用户密钥是否正确
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'])
        {
            $this->error_return(-150);
        }

        //判断是否已经实名认证过
        if(!is_numeric($user_base['real_name']))
        {
            $this->error_return(-10019);
        }

        //判断实名认证的姓名是否合法
        if(!preg_match('/^[\x{4e00}-\x{9fa5}]{2,6}+$/u', $real_name))
        {
            $this->error_return(-10020);
        }

        //判断身份证是否合法
        if($this->validation_filter_id_card($card_id) === false)
        {
            $this->error_return(-10021);
        }

        $userBaseModel = new UserBaseModel();

        //将身份证和真实姓名添加到对应的用户记录中
        $res = $userBaseModel->setUserRealName($uid,$real_name,$card_id);
        if($res === false)
        {
            $this->error_return(-1);
        }
        else
        {
            $this->success_return('','实名认证成功');
        }
    }

    /**
     * 判断身份证是否正确
     * @param $id_card
     * @return bool
     */
    private function validation_filter_id_card($id_card){
        if(strlen($id_card)==18){
            return $this->idcard_checksum18($id_card);
        }elseif((strlen($id_card)==15)){
            $id_card=$this->idcard_15to18($id_card);
            return $this->idcard_checksum18($id_card);
        }else{
            return false;
        }
    }

// 计算身份证校验码，根据国家标准GB 11643-1999
    private function idcard_verify_number($idcard_base){
        if(strlen($idcard_base)!=17){
            return false;
        }
        //加权因子
        $factor=array(7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2);
        //校验码对应值
        $verify_number_list=array('1','0','X','9','8','7','6','5','4','3','2');
        $checksum=0;
        for($i=0;$i<strlen($idcard_base);$i++){
            $checksum += substr($idcard_base,$i,1) * $factor[$i];
        }
        $mod=$checksum % 11;
        $verify_number=$verify_number_list[$mod];
        return $verify_number;
    }
// 将15位身份证升级到18位
    private function idcard_15to18($idcard){
        if(strlen($idcard)!=15){
            return false;
        }else{
            // 如果身份证顺序码是996 997 998 999，这些是为百岁以上老人的特殊编码
            if(array_search(substr($idcard,12,3),array('996','997','998','999')) !== false){
                $idcard=substr($idcard,0,6).'18'.substr($idcard,6,9);
            }else{
                $idcard=substr($idcard,0,6).'19'.substr($idcard,6,9);
            }
        }
        $idcard=$idcard.$this->idcard_verify_number($idcard);
        return $idcard;
    }
// 18位身份证校验码有效性检查
    private function idcard_checksum18($idcard){
        if(strlen($idcard)!=18){
            return false;
        }
        $idcard_base=substr($idcard,0,17);
        if($this->idcard_verify_number($idcard_base)!=strtoupper(substr($idcard,17,1))){
            return false;
        }else{
            return true;
        }
    }

    /**
     * 加入我们
     * @param string $user_token
     * @param string $uid
     */
    public function join_us($user_token = "", $uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }


        $data['message'] = '加入我们，搜索官方微信公众号：百赢娱乐';
        $data['url'] = C("the_agent_url");

        $this->error_return(1,$data);
    }

    /**
     * 获取短信验证码
     * @param int $mobile 需要传递的手机号
     * @param int $type 需要获取的短信类型
     */
    public function getSmsCode($mobile = "",$sms_type = ""){
        //手机号验证
        if(empty($mobile))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
        }
        else
        {
            $Regular_phone = C('User.Regular_phone');
            if(!preg_match($Regular_phone,$mobile))
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
            }
        }
        $codeModel = new Code();
        switch ($sms_type){
            case 1:
                $sms_res = $codeModel->createCode($mobile,"register");
                break;
            case 2:
                $sms_res = $codeModel->createCode($mobile,"updatePass");
                break;
            case 3:
                $sms_res = $codeModel->createCode($mobile,"old_changePhone");
                break;
            case 4:
                $sms_res = $codeModel->createCode($mobile,"new_changePhone");
                break;
            default:
                $sms_res = -101;
                break;
        }

        switch ($sms_res){
            case 1:
                $this->ajaxReturn(['status'=>'true','data'=>['message'=>'短信发送成功'],'statusCode'=>1]);
                break;
            case -15:
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送已达到上限'],'statusCode'=>-15]);
                break;
            case -101:
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信类型sms_type必须发送'],'statusCode'=>-15]);
                break;
            case -105:
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
                break;
            case -10016:
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送太过频繁请稍后在试'],'statusCode'=>-10016]);
                break;
            default:
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
                break;
        }
    }

    /*以下方法可能不需要*/

//    /**
//     * 用户反馈
//     */
//    private function userFeedback(){
//        $sub_url = "/index.php/User/User/userFeedBackReturn.html";
//        $this->assign('sub_url',$sub_url);
//        $this->display('feedback');
//    }
//
//    /**
//     * 用户反馈返回
//     */
//    private function userFeedBackReturn(){
//        $this->display("feedbackreturn");
//    }
//
//    /**
//     * 获取用户签到信息
//     * @param string $user_token   用户密钥
//     * @param int $uid   用户id
//     */
//    private function getUserSignInfo($user_token = "",$uid = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'] || $uid == ""){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//        $sign_in_model = new SignInModel();
//        $sign_info = $sign_in_model->getInfo();
//        if($sign_info === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        else if(empty($sign_info))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'没有签到信息'],'statusCode'=>-13002]);
//        }
//        else
//        {
//            //获取用户信息
//            $userActiveModel = new UserActiveModel();
//            $user_active = $userActiveModel->getInfoById($uid);
//            if($user_active === false)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }
//            else if($user_active == 0)
//            {
//                $this->error_return(-10008);
//            }
//            //获取vip配置信息
//            $vipModel = new VipModel();
//            $vip_info = $vipModel->getInfoByLevel($user_active['vip']);
//            if($vip_info === false)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }else if($vip_info == 0)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'没有找到vip信息'],'statusCode'=>-10010]);
//            }
//            foreach ($sign_info as $key => $val)
//            {
//                $sign_info[$key]['give_points_num'] = $sign_info[$key]['give_points_num'] * $vip_info['sign_in_ratio'];
//                $sign_info[$key]['give_gold_num'] = $sign_info[$key]['give_gold_num'] * $vip_info['sign_in_ratio'];
//                $sign_info[$key]['give_diamond_num'] = $sign_info[$key]['give_diamond_num'] * $vip_info['sign_in_ratio'];
//            }
//
//            //获取用户的签到信息
//            $user_info = $userActiveModel->getUserSignInInfo($uid);
//            if($user_info === false)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }
//            else if(empty($user_info))
//            {
//                $res = $userActiveModel->addUserSignIn($uid);
//                if($res === false)
//                {
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//                }
//                $user_info = [];
//                $user_info['now_sign_in'] = 1;
//                $user_info['is_sign_in'] = 0;
//                $user_info['sign_in_time'] = 0;
//            }
//            else
//            {
//                $user_info = $user_info[0];
//            }
//            $this->ajaxReturn(['status'=>'true','data'=>['sign_info'=>$sign_info,'user_info'=>$user_info],'statusCode'=>1]);
//        }
//    }
//
//    /**
//     * 用户签到
//     * @param string $user_token   用户密钥
//     * @param int $uid   用户id
//     */
//    private function userSignIn($user_token = "",$uid = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'] || $uid == "")
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//
//        //获取用户的签到信息
//        $userActiveModel = new UserActiveModel();
//        $user_info = $userActiveModel->getUserSignInInfo($uid);
//        if($user_info === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        else if(empty($user_info))
//        {
//            $res = $userActiveModel->addUserSignIn($uid);
//            if($res === false)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }
//            $user_info = [];
//            $user_info['now_sign_in'] = 1;
//            $user_info['is_sign_in'] = 0;
//            $user_info['sign_in_time'] = 0;
//        }
//        else
//        {
//            $user_info = $user_info[0];
//        }
//
//        if($user_info['is_sign_in'] != 0)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户今日已经签到过了'],'statusCode'=>-1]);
//        }
//
//        //获取签到配置信息
//        $sign_in_model = new SignInModel();
//        $config_info = $sign_in_model->getInfo($user_info['now_sign_in']);
//        if($config_info === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        else if(empty($config_info))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'没有签到信息'],'statusCode'=>-13002]);
//        }
//
//        //获取用户信息
//        $userActiveModel = new UserActiveModel();
//        $user_active = $userActiveModel->getInfoById($uid);
//        if($user_active === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        else if($user_active == 0)
//        {
//            $this->error_return(-10008);
//        }
//
//
//        //给与用户对应的金钱
//        $give_points_num = $config_info[0]['give_points_num'];
//        $give_gold_num = $config_info[0]['give_gold_num'];
//        $give_diamond_num = $config_info[0]['give_diamond_num'];
//
//        //给与奖励
//        $userActiveModel->startTrans();
//        if($give_points_num > 0)
//        {
//            $res = $userActiveModel->updatePointsByUid($uid,$give_points_num);
//            if($res !== true)
//            {
//                $userActiveModel->rollback();
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
//            }
//        }
//        if($give_gold_num > 0)
//        {
//            $res = $userActiveModel->updateGoldByUid($uid,$give_gold_num);
//            if($res !== true)
//            {
//                $userActiveModel->rollback();
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
//            }
//        }
//        if($give_diamond_num > 0)
//        {
//            $res = $userActiveModel->updateDiamondByUid($uid,$give_diamond_num);
//            if($res !== true)
//            {
//                $userActiveModel->rollback();
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
//            }
//        }
//
//        //更新签到次数
//        if($user_info['now_sign_in'] < 7)
//        {
//            $now_sign_in = $user_info['now_sign_in'] + 1;
//        }
//        else
//        {
//            $now_sign_in = 1;
//        }
//
//        $res = $userActiveModel->setUserSignIn($uid,$now_sign_in,1,time());
//        if($res === false)
//        {
//            $userActiveModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接出错'],'statusCode'=>-1]);
//        }
//        else
//        {
//            $userActiveModel->commit();
//            //更新用户redis
//            $user_active = $this->getUserActiveRedis($uid);
//            $this->ajaxReturn(['status'=>'true','data'=>[
//                'message'=>'签到成功',
//                'user_gold' => $user_active['gold'],
//                'user_diamond' => $user_active['diamond'],
//                'user_points' => $user_active['points'],
//            ],'statusCode'=>1]);
//        }
//    }
//    /**
//     * 手机号登录
//     * @param string $phone
//     * @param string $passwd
//     */
//    public function userLogin($phone = "",$passwd = ""){
//        //获取用户数据
//        if(empty($phone))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-10005]);
//        }
//        if(empty($passwd))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码不能为空'],'statusCode'=>-10002]);
//        }
//
//        //获取用户的登录redis
//        $userRedis = new UserRedis();
//        $login_num = $userRedis->getRedis(10,$phone);
//        if($login_num == false)
//        {
//            $login_num = 0;
//        }
//        else if($login_num > C('user_login_max_num'))
//        {
//            $time = $userRedis->ttlSscRedis(10,$phone);
//            if($time === false)
//            {
//                $this->error_return(-2);
//            }
//            else
//            {
//                if((int)$time < 60)
//                {
//                    $date = $time.'秒';
//                }
//                else
//                {
//                    $date = ceil(($time / 60)).'分钟';
//                }
//                $this->error_return(-104,'',[ 'date' => $date]);
//            }
//
//        }
//
//        //数据验证
//        $data['phone'] =  $phone;
//        $data['passwd'] =  $passwd;
//        $Regular_phone = C('USER_DEFAULT.REGULAR_PHONE');
//        if(!preg_match($Regular_phone,$data['phone']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//        }
//        $Regular_passwd = C('USER_DEFAULT.REGULAR_PASSWD');
//        if(!preg_match($Regular_passwd,$data['passwd']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式错误'],'statusCode'=>-10003]);
//        }
//
//        //验证用户密码和token
//        $userBaseModel = new UserBaseModel();
//        $user_base = $userBaseModel->getMd5PwdByPhone($data['phone']);
//        if($user_base == -1)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'获取用户数据失败'],'statusCode'=>-1]);
//        }
//        if($user_base == 0)
//        {
//            $this->error_return(-10008);
//        }
//        $md5 = $user_base['md5'];
//        $passwd = $user_base['passwd'];
//        $uid = $user_base['uid'];
//        if((md5($data['passwd'].$md5)) != $passwd)
//        {
//            $login_num++;
//            $res = $userRedis->setRedis(10,$login_num,C('user_login_false_time'),$phone);
//            if($res === false)
//            {
//                $this->ajaxReturn(-2);
//            }
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码错误,请重新输入'],'statusCode'=>-10001]);
//        }
//
//        //更新用户token
//        $last_time = time();
//        $last_ip = $_SERVER["REMOTE_ADDR"];
//        $user_token = $userBaseModel->updateTokenById($uid,$user_base['phone_str'],$last_time,$last_ip);
//        if($user_token === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户token更新失败'],'statusCode'=>-20002]);
//        }
//
//        //把用户token写入redis
//        $redisModel = new UserRedis();
//        $redis_res = $redisModel->delRedis(1,'',$user_base['user_token']);
//        if($redis_res === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
//        }
//        $redis_res = $redisModel->setRedis(1,$uid,C('REDIS_TIMEOUT'),'',$user_token);
//        if($redis_res === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
//        }
//        $user_active = $this->getUserActiveRedis($uid,1);
//        if($user_active == -2)
//        {
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
//        }
//
//        //获取服务器信息
//        $serverModel = new ServerModel();
//        $serverInfo = $serverModel->getInfo();
//        if($serverInfo === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        if($serverInfo == 0)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'没有账号可以进入的服务器'],'statusCode'=>-11001]);
//        }
//        $server_ip = $serverInfo[0]['ip'];
//        $server_port = $serverInfo[0]['port'];
//
//        $res = [
//            "user_token" => $user_token,
//            "uid" => $uid,
//            "server_ip" => $server_ip,
//            "server_port" => $server_port,
//        ];
//        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
//    }
//    /**
//     * 手机号注册
//     * @param string $data
//     */
//    public function userRegister($data = ""){
//        //获取用户数据
//        if(empty($data))
//        {
//            $data = I('');
//        }
//        $userBaseModel = new UserBaseModel();
//
//        //数据验证
//        if(!isset($data['app_id']) || empty($data['app_id'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要app_id'],'statusCode'=>-101]);
//        }
//        if(!isset($data['phone_type']) || empty($data['phone_type'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_type'],'statusCode'=>-101]);
//        }
//        if(!isset($data['phone_name']) || empty($data['phone_name'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_name'],'statusCode'=>-101]);
//        }
//        if(!isset($data['phone_str']) || empty($data['phone_str'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要phone_str'],'statusCode'=>-101]);
//        }
//        if(empty($data['passwd'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码不能为空'],'statusCode'=>-101]);
//        }else{
//            $Regular_passwd = C('User.Regular_passwd');
//            if(!preg_match($Regular_passwd,$data['passwd'])){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式不对'],'statusCode'=>-10003]);
//            }
//            foreach (C('User.Regular_data') as $val){
//                if(strpos($data['passwd'],$val) !== false){
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式不对,包含非法字符'],'statusCode'=>-10003]);
//                }
//            }
//        }
//        if(empty($data['phone'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }else{
//            $this->getPhoneIsOne($data['phone']);
//        }
//        if(empty($data['code'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码不能为空'],'statusCode'=>-10006]);
//        }else{
//            //验证验证码
//            $cordModel = new Code();
//            $code_res = $cordModel->conCode($data['phone'],'register',$data['code']);
//            if($code_res != 1){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//            }else if($code_res == -10014){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//            }else if($code_res == -10013){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//            }
//        }
//
//        //获取md5 组装密码和数据
//        $md5 = rand(1000,9999);
//
//        $user_base['phone'] = $data['phone'];
//        $user_base['passwd'] = md5($data['passwd'].$md5);
//        $user_base['md5'] = $md5;
//        $user_base['create_time']= $user_base['last_time'] = date(time());
//        $user_base['regis_ip']= $user_base['last_ip'] = $_SERVER["REMOTE_ADDR"];
//
//        $user_base['app_id'] = str_replace(C('User.Regular_data'),"",$data['app_id']);
//        $user_base['phone_type'] = str_replace(C('User.Regular_data'),"",$data['phone_type']);
//        $user_base['phone_name'] = str_replace(C('User.Regular_data'),"",$data['phone_name']);
//        $user_base['phone_str'] = str_replace(C('User.Regular_data'),"",$data['phone_str']);
//
//        //注册账号
//        $userBaseModel->startTrans();
//        $uid = $userBaseModel->addUser($user_base);
//        if($uid === false || empty($uid)){
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'注册失败'],'statusCode'=>-20001]);
//        }
//
//        $userActiveModel = new UserActiveModel();
//        $res = $userActiveModel->addUser($uid);
//        if($res === false){
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'注册失败'],'statusCode'=>-20001]);
//        }
//
//        //更新用户token
//        $last_time = time();
//        $last_ip = $_SERVER["REMOTE_ADDR"];
//        $user_token = $userBaseModel->updateTokenById($uid,$user_base['phone_str'],$last_time,$last_ip);
//        if($user_token === false){
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户token更新失败'],'statusCode'=>-20002]);
//        }
//
//        //把用户token写入redis
//        $redisModel = new UserRedis();
//        $redis_res = $redisModel->setRedis(1,$uid,C('REDIS_TIMEOUT'),'',$user_token);
//        if($redis_res === false){
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
//        }
//        $user_active = $this->getUserActiveRedis($uid,1);
//        if($user_active == -2){
//            $userBaseModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis链接失败'],'statusCode'=>-2]);
//        }
//        $userBaseModel->commit();
//
//        //获取服务器信息
//        $serverModel = new ServerModel();
//        $serverInfo = $serverModel->getInfo();
//        if($serverInfo === false){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        if($serverInfo == 0){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'未账号可以进入的服务器'],'statusCode'=>-1]);
//        }
//        $server_ip = $serverInfo[0]['ip'];
//        $server_port = $serverInfo[0]['port'];
//        $res = [
//            "user_token" => $user_token,
//            "uid" => $uid,
//            "server_ip" => $server_ip,
//            "server_port" => $server_port,
//        ];
//        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
//    }
    //变更头像
//    public function changePicHead($user_token = "",$uid = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid']){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//        //验证手机号
//        if($user_base['phone'] == 0){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要绑定手机才能修改'],'statusCode'=>-10010]);
//        }
//        $file = $_FILES['file'];
//
//        //获取用户id
//        $uid = $user_base['uid'];
//        $time = time();
//
//        //数据验证
//        if(!isset($file) || empty($file))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要上传头像'],'statusCode'=>-101]);
//        }
//        $fileUpload = $file;
//        $type = strtolower(substr($fileUpload['name'],strrpos($fileUpload['name'],'.')+1));
//        $user_statuc = C("User.user_static");
//        $img_type = $user_statuc['user_pic_head_type'];
//
//        if(!in_array($type,$img_type))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'图片格式错误'],'statusCode'=>-10103]);
//        }
//
//        //组装头像路径 将头像放入路径中
//        $userImage = $user_statuc['image'];
//        $createUrl = $user_statuc['create_image'];
//        $userIdFile = ceil($uid / $user_statuc['userIdFile']);
//        $picHead = md5($uid.$time);
//        $user_pic_url = "$userImage/$userIdFile/$picHead.$type"; //图片读取路径
//        $create_pic_url = "$createUrl/$userIdFile/$picHead.$type"; //图片存储路径
//        //判断是否有该用户文件目录如果没有则创建
//        if(!file_exists("$createUrl/$userIdFile"))
//        {
//            if(!mkdir("$createUrl/$userIdFile",0777,true)){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'创建文件夹失败'],'statusCode'=>-10019]);
//            }
//        }
//
//        //根据用户id获取用户以前的头像路径
//        $userActiveModel = new UserActiveModel();
//        $user_active = $userActiveModel->getInfoById($uid);
//        $userPic = $user_active['pic_head'];
//
//        //更新用户头像
//        $userActiveModel->startTrans();
//        $res = $userActiveModel->updatePicHeadById($uid,$user_pic_url);
//        if($res === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户头像更新失败'],'statusCode'=>-1]);
//        }
//
//        //创建用户文件
//        if(!move_uploaded_file($fileUpload['tmp_name'],$create_pic_url))
//        {
//            $userActiveModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户头像保存失败'],'statusCode'=>-10020]);
//        }
//        $userActiveModel->commit();
//
//        $this->getUserActiveRedis($uid);
//        $res = [];
//        if(!empty($userPic))
//        {
//            $old_user_head_uid = explode('/',$userPic['pic_head']);
//            $old_img_url = array_pop($old_user_head_uid);
//            $old_user_url = array_pop($old_user_head_uid);
//            $old_url = $old_user_url.'/'.$old_img_url;
//            //检查旧头像路径是否在用户文件中如果在则删除
//            if(file_exists("$createUrl/$old_url"))
//            {
//                if(!unlink("$createUrl/$old_url"))
//                {
//                    $res['uid'] = $uid;
//                    $res['head_url'] = $user_pic_url;
//                    $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
//                }
//            }
//        }
//        $this->getUserActiveRedis($uid);
//        //返回新的头像路径
//        $res['uid'] = $uid;
//        $res['head_url'] = $user_pic_url;
//        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
//    }
//    //修改名字
//    public function updateUserName($user_token = "",$uid = "",$name = ""){
//        if(empty($name))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递name'],'statusCode'=>-101]);
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'])
//        {
//            $this->error_return(-150);
//        }
//        $this->getUserBaseRedis($user_token);
//
//        $userActiveModel = new UserActiveModel();
//
//        //验证姓名是否有屏蔽词
//        $res = $this->validationString($name);
//        if($res === false)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'输入的内容包含了违禁内容'],'statusCode'=>-10009]);
//        }
//
//        //验证手机号
//        if($user_base['phone'] == 0)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要绑定手机才能修改'],'statusCode'=>-10010]);
//        }
//        $user_active = $userActiveModel->getInfoById($user_base['uid']);
//        if($user_active == -1)
//        {
//            $this->error_return(-1);
//        }
//        if($user_active == 0)
//        {
//            $this->error_return(-10008);
//        }
//
//        //昵称校验
//        if($name == $user_active['name'])
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'新名字和旧名字一样'],'statusCode'=>-10011]);
//        }
//
//        if((strlen($name) + mb_strlen($name,'UTF8')) / 2 > 32)
//        {
//            $this->error_return(-10012);
//        }
//
//        $name = base64_encode($name);
//
////        foreach (C('User.Regular_data') as $val){
////            if(strpos($name,$val) !== false){
////                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'签名包含非法字符'],'statusCode'=>-10003]);
////            }
////        }
//
//        $userActiveModel->startTrans();
//        if($user_active['is_modname'] == '0')
//        {
//            //扣钱
//            $res = $userActiveModel->updateDiamondByUid($user_base['uid'],-C('User.update_name_diamond'));
//
//            if($res === false)
//            {
//                $userActiveModel->rollback();
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }
//            else if($res === 0)
//            {
//                $userActiveModel->rollback();
//                $this->error_return(-10008);
//            }
//            else if($res === -13)
//            {
//                $userActiveModel->rollback();
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'用户钻石不足'],'statusCode'=>-13]);
//            }
//        }
//
//        //修改名字
//        $res = $userActiveModel->updateNameByUid($user_base['uid'],$name,$user_active['is_modname']);
//        if($res === false)
//        {
//            $userActiveModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'昵称修改失败'],'statusCode'=>-10012]);
//        }
//
//        $userActiveModel->commit();
//
//        $user_active = $this->getUserActiveRedis($user_base['uid']);
//        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'昵称修改成功','name'=>$user_active['name'],'diamond'=>$user_active['diamond']],'statusCode'=>1]);
//    }
//    //获取Vip信息
//    public function getVipInfo($user_token = "",$uid = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'] || $uid == "")
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//
//        $vipModel = new VipModel();
//        $info = $vipModel->getInfo();
//        if($info === false){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }else if($info == 0){
//            $this->ajaxReturn(['status'=>'true','data'=>['info'=>0],'statusCode'=> 1]);
//        }else{
//            $this->ajaxReturn(['status'=>'true','data'=>['info'=>$info],'statusCode'=> 1]);
//        }
//    }
//    /**
//     * 获取用户信息
//     * @param string $user_token
//     * @param string $sel
//     * @param string $uid
//     * @param string $game_id
//     */
//    public function getUserById($user_token = "",$sel = "",$uid = "",$game_id = ""){
//        if(empty($sel)){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递sel'],'statusCode'=>-101]);
//        }
//
//        //验证token是否成效
//        $user_base = $this->getUserBaseByToken($user_token);
//        if(empty($uid))
//        {
//            $uid = $user_base['uid'];
//        }
//        else
//        {
//            $userBaseModel = new UserBaseModel();
//            $user_base = $userBaseModel->getInfoById($uid);
//            if($user_base === false)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//            }
//            else if($user_base == 0)
//            {
//                $this->error_return(-10008);
//            }
//        }
//
//        $sel = explode(',',$sel);
//        $redis = new UserRedis();
//        $user_active = $redis->getRedis(3,$uid);
//        if(!$user_active)
//        {
//            $user_active = $this->getUserActiveRedis($user_base['uid']);
//        }
//
//        $res = [];
//
//        $i = 0;
//        foreach ($sel as $v)
//        {
//            $res[$v] = 0;
//            if($i == 0 && ($v == "game_total" || $v == "game_win")){
//                $userBaseModel = new UserBaseModel();
//                $user_total = $userBaseModel->getUserTotalInfo($uid,$game_id);
//                if($user_total === false)
//                {
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//                }
//                else
//                {
//                    foreach ($user_total as $key => $val)
//                    {
//                        $user_active["game_total"] = $user_active["game_total"] + $val['game_total'];
//                        $user_active["game_win"] = $user_active["game_win"] + $val['game_win'];
//                    }
//                }
//                $i++;
//            }
//        }
//
//        foreach ($user_base as $k => $v)
//        {
//            if(isset($res[$k]))
//            {
//                $res[$k] = $v;
//            }
//        }
//
//        foreach ($user_active as $k => $v)
//        {
//            if(isset($res[$k]))
//            {
//                $res[$k] = $v;
//            }
//        }
//
//        if(isset($res['sex']) && $uid < 10000)
//        {
//            $res['sex'] = $uid % 2;
//        }
//
//
//        $this->ajaxReturn(['status'=>'true','data'=>$res,'statusCode'=>1]);
//    }
//    //修改密码
//    public function updatePassWd($passwd = "",$phone = "",$code = "",$user_token = "")
//    {
//        if(empty($passwd))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $Regular_passwd = C('User.Regular_passwd');
//            if(!preg_match($Regular_passwd,$passwd))
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式不对'],'statusCode'=>-10003]);
//            }
//            foreach (C('User.Regular_data') as $val)
//            {
//                if(strpos($passwd,$val) !== false)
//                {
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式不对,包含非法字符'],'statusCode'=>-10003]);
//                }
//            }
//        }
//        if(empty($phone))
//        {
//            $phone = I('param.phone');
//            if(empty($phone))
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递phone'],'statusCode'=>-101]);
//            }
//        }
//        if(empty($code))
//        {
//            $code = I('param.code');
//            if(empty($code))
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递code'],'statusCode'=>-101]);
//            }
//        }
//        if(empty($user_token))
//        {
//            $user_token = I('user_token');
//        }
//
//        //验证手机号
//        $Regular_phone = C('User.Regular_phone');
//        if(!preg_match($Regular_phone,$phone))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//        }
//
//        $Regular_passwd = C('User.Regular_passwd');
//        if(!preg_match($Regular_passwd,$passwd))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式错误'],'statusCode'=>-10003]);
//        }
//
//        //验证验证码
//        $cordModel = new Code();
//        $code_res = $cordModel->conCode($phone,'updatePass',$code);
//        if($code_res != 1)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10014)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10013)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//        }
//
//        //获取用户MD5
//        $userBaseModel = new UserBaseModel();
//        $user_info = $userBaseModel->getMd5PwdByPhone($phone);
//        if($user_info == -1){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//        }
//        if($user_info == 0){
//            $this->error_return(-10008);
//        }
//        $uid = $user_info['uid'];
//        $md5 = rand(1000,9999);
//        $passwd = md5($passwd.$md5);
//        if($user_info['user_token'] == $user_token) {
//            $res = $userBaseModel->updatePWMd5ById($uid, $passwd, $md5,true);
//        }else{
//            $userRedis = new UserRedis();
//            //删除用户redis数据
//            $userRedis->delRedis(1,'',$user_info['user_token']);
//            $res = $userBaseModel->updatePWMd5ById($uid,$passwd,$md5,false);
//        }
//
//        if($res === false){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码更新失败'],'statusCode'=>-10017]);
//        }else{
//            $this->ajaxReturn(['status'=>'true','data'=>['message'=>'新密码设置成功'],'statusCode'=>1]);
//        }
//    }
//
//    //修改密码获取短信
//    public function updatePassWdCode($mobile = ""){
//        //获取提交的手机号
//        if(empty($mobile)){
//            $mobile = I('param.phone');
//        }
//
//        //手机号验证
//        if(empty($mobile)){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }else{
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$mobile)){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//            }
//        }
//
//        $codeModel = new Code();
//        //创建并发送验证码
//        $sms_res = $codeModel->createCode($mobile,"updatePass");
//        switch ($sms_res){
//            case 1:
//                $this->ajaxReturn(['status'=>'true','data'=>['message'=>'短信发送成功'],'statusCode'=>1]);
//                break;
//            case -15:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送已达到上限'],'statusCode'=>-15]);
//                break;
//            case -105:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//            case -10016:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送太过频繁请稍后在试'],'statusCode'=>-10016]);
//                break;
//            default:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//        }
//    }
//
//    //绑定手机
//    public function bindPhone($user_token = "",$data = ""){
//        if(empty($user_token))
//        {
//            $user_token = I('param.user_token');
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        if(empty($data))
//        {
//            //获取用户数据
//            $data = I('');
//        }
//        $userBaseModel = new UserBaseModel();
//
//        //数据验证
//        if(empty($data['passwd']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $Regular_passwd = C('User.Regular_passwd');
//            if(!preg_match($Regular_passwd,$data['passwd']))
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'密码格式不对'],'statusCode'=>-10003]);
//            }
//        }
//
//        if(empty($data['code']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码不能为空'],'statusCode'=>-10006]);
//        }
//        else
//        {
//            //验证验证码
//            $cordModel = new Code();
//            $code_res = $cordModel->conCode($data['phone'],'register',$data['code']);
//            if($code_res != 1)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//            }
//            else if($code_res == -10014)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//            }
//            else if($code_res == -10013)
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//            }
//        }
//
//        if(empty($data['phone']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $this->getPhoneIsOne($data['phone']);
//        }
//
//        //获取md5 组装密码和数据
//        $md5 = rand(1000,9999);
//
//        $set['phone'] = $data['phone'];
//        $set['passwd'] = md5($data['passwd'].$md5);
//        $set['md5'] = $md5;
//
//        //更新用户数据
//        $res = $userBaseModel->updateUser($user_base['uid'],$set);
//        if($res === true)
//        {
//            $this->getUserActiveRedis($user_base['uid']);
//            $this->getUserBaseRedis($user_token);
//            $this->ajaxReturn(['status'=>'true','data'=>['message'=>'绑定手机成功','phone'=>$data['phone']],'statusCode'=>1]);
//        }
//        else
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'绑定手机失败'],'statusCode'=>-10018]);
//        }
//    }
//
//    //老手机号获取短信
//    public function oldPhone($user_token = '',$mobile = ''){
//        if(empty($user_token)){
//            $user_token = I('param.user_token');
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        //获取提交的手机号
//        if(empty($mobile)){
//            $mobile = I('param.phone');
//        }
//
//        //手机号验证
//        if(empty($mobile)){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }else{
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$mobile)){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//            }
//        }
//
//        $codeModel = new Code();
//        //创建并发送验证码
//        $sms_res = $codeModel->createCode($mobile,"old_changePhone");
//        switch ($sms_res){
//            case 1:
//                $this->ajaxReturn(['status'=>'true','data'=>['message'=>'短信发送成功'],'statusCode'=>1]);
//                break;
//            case -15:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送已达到上限'],'statusCode'=>-15]);
//                break;
//            case -105:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//            case -10016:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送太过频繁请稍后在试'],'statusCode'=>-10016]);
//                break;
//            default:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//        }
//    }
//
//    //新手机号获取短信
//    public function newPhone($user_token = '',$mobile = ''){
//        if(empty($user_token))
//        {
//            $user_token = I('param.user_token');
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        //获取提交的手机号
//        if(empty($mobile))
//        {
//            $mobile = I('param.phone');
//        }
//
//        //手机号验证
//        if(empty($mobile))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$mobile)){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//            }
//        }
//
//        $codeModel = new Code();
//        //创建并发送验证码
//        $sms_res = $codeModel->createCode($mobile,"new_changePhone");
//        switch ($sms_res)
//        {
//            case 1:
//                $this->ajaxReturn(['status'=>'true','data'=>['message'=>'短信发送成功'],'statusCode'=>1]);
//                break;
//            case -15:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送已达到上限'],'statusCode'=>-15]);
//                break;
//            case -105:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//            case -10016:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'今日短信发送太过频繁请稍后在试'],'statusCode'=>-10016]);
//                break;
//            default:
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信发送失败'],'statusCode'=>-105]);
//                break;
//        }
//    }
//
//    /**
//     * 变更手机之旧手机号
//     * @param string $user_token
//     * @param string $uid
//     * @param string $phone
//     * @param string $code
//     */
//    public function validateOldPhone($user_token = "",$uid = "",$phone = "",$code = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'] || $uid == "")
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//        if($phone != $user_base['phone'])
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递当前绑定的手机号'],'statusCode'=>-1500]);
//        }
//        if(empty($code))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码不能为空'],'statusCode'=>-10007]);
//        }
//        $codeModel = new Code();
//        $code_res = $codeModel->conCode($phone,'old_changePhone',$code);
//        if($code_res != 1){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10014)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10013)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//        }
//        else
//        {
//            $this->ajaxReturn(['status'=>'true','data'=>['message'=>'验证通过',"vali" => "true"],'statusCode'=>1]);
//        }
//
//    }
//
//    /**
//     * 变更手机号
//     * @param string $user_token
//     * @param string $uid
//     * @param string $phone
//     * @param string $code
//     * @param string $vali
//     */
//    public function changePhoneNew($user_token = "",$uid = "",$phone = "",$code = "",$vali = ""){
//        if($vali != "true")
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请先验证旧手机号'],'statusCode'=>-1510]);
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid']){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//        if(empty($code))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码不能为空'],'statusCode'=>-10007]);
//        }
//        if(empty($user_token))
//        {
//            $user_token = I('param.user_token');
//        }
//        $user_base = $this->getUserBaseByToken($user_token);
//        $userBaseModel = new UserBaseModel();
//
//        //数据验证
//        if(empty($phone))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }
//        else if($phone == $user_base['phone']){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'新手机号不能是旧手机号'],'statusCode'=>-10005]);
//        }
//        else
//        {
//            $this->getPhoneIsOne($phone);
//        }
//
//        //校验验证码
//        $codeModel = new Code();
//        $code_res = $codeModel->conCode($phone,'new_changePhone',$code);
//        if($code_res != 1)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10014)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//        }
//        else if($code_res == -10013)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//        }
//
//        //更新用户手机号
//        $res = $userBaseModel -> updatePhone($uid,$phone);
//        if(!$res){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'更换手机失败'],'statusCode'=>-10051]);
//        }
//        $this->getUserBaseRedis($user_token);
//        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'更换手机完成','phone'=>$phone],'statusCode'=>1]);
//
//    }
//
//    //变更手机
//    public function changePhone($user_token = "",$uid = ""){
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid']){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//
//        if(empty($data)){
//            //获取用户数据
//            $data = I('');
//        }
//        $userBaseModel = new UserBaseModel();
//
//        //数据验证
//        if($data['old_phone'] == $data['new_phone']){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'新手机号不能是旧手机号'],'statusCode'=>-10005]);
//        }
//        if(empty($data['old_phone']))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'旧手机号不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$data['old_phone']))
//            {
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10004]);
//            }
//        }
//
//        if(empty($data['new_phone'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }else{
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$data['new_phone'])){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10003]);
//            }else{
//                //手机号唯一性判断
//                $phone_res = $userBaseModel->getMd5PwdByPhone($data['new_phone']);
//                if($phone_res == -1){
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//                }
//                if($phone_res != 0){
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'该手机号已经存在'],'statusCode'=>-10005]);
//                }
//            }
//        }
//        if(empty($data['old_phone_code'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'旧手机验证码不能为空'],'statusCode'=>-101]);
//        }
//        if(empty($data['new_phone_code'])){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'新手机验证码不能为空'],'statusCode'=>-101]);
//        }
//
//        $old_phone = $data['old_phone'];
//        $old_phone_code = $data['old_phone_code'];
//        $new_phone = $data['new_phone'];
//        $new_phone_code = $data['new_phone_code'];
//
//        //校验验证码
//        $codeModel = new Code();
//        $code_res = $codeModel->conCode($old_phone,'old_changePhone',$old_phone_code);
//        if($code_res != 1){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//        }else if($code_res == -10014){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//        }else if($code_res == -10013){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//        }
//
//        $code_res = $codeModel->conCode($new_phone,'new_changePhone',$new_phone_code);
//        if($code_res != 1){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'验证码错误'],'statusCode'=>-10007]);
//        }else if($code_res == -10014){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'请获取短信验证码'],'statusCode'=>-10007]);
//        }else if($code_res == -10013){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'短信验证码错误'],'statusCode'=>-10007]);
//        }
//
//        //更新用户手机号
//        $uid = $user_base['uid'];
//        $res = $userBaseModel -> updatePhone($uid,$new_phone);
//        if(!$res){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'更换手机失败'],'statusCode'=>-10051]);
//        }
//        $this->getUserBaseRedis($user_token);
//        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'更换手机完成'],'statusCode'=>1]);
//    }
}