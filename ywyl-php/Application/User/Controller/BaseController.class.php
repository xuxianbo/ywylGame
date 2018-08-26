<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/19
 * Time: 17:55
 */

namespace User\Controller;


use Think\Cache\Driver\Redis;
use Think\Log;
use User\Model\UserActiveModel;
use User\Model\UserBaseModel;
use User\Model\UserRedis;
use Think\Controller;

class BaseController extends Controller
{
    /**
     * 首页
     */
    public function index()
    {
        header("Location:http://www.qka888.com");
    }

    /**
     * 根据用户密钥获取用户基础信息
     * @param string $user_token  用户密钥
     * @param bool $is_update  是否获取最新数据
     * @return bool|mixed
     */
    public function getUserBaseByToken($user_token = ""){
        if(empty($user_token))
        {
            $this->error_return(-103);
        }

        $pattern = "/[a-z0-9]/";
        if(!preg_match($pattern, $user_token))
        {
            $this->error_return(-103);
        }

        $userRedis = new UserRedis();
        $userBaseModel = new UserBaseModel();

        $user_base = $userBaseModel->getInfoByToken($user_token);
        if($user_base === false)
        {
            $this->error_return(-1);
        }
        else if(is_numeric($user_base))
        {
            switch ($user_base)
            {
                case 0:
                    $this->error_return(-103);
                    break;
                case -20003:
                    $this->error_return(-20003);
                    break;
                case -20004:
                    $this->error_return(-20004);
                    break;
                case -20005:
                    $this->error_return(-20005);
                    break;
                default:
                    $this->error_return(-20003);
                    break;
            }
        }
        else
        {
            //判断如果用户的状态为1 说明被封号
            if($user_base['status'] == 1)
            {
                $this->error_return(-20004);
            }

            //更新用户tokenRedis
            $res = $userRedis->setRedis(1,$user_base['uid'],C('REDIS_TIMEOUT'),'',$user_token);
            if($res === false)
            {
                $this->error_return(-2);
            }
        }

        return $user_base;
    }

    /**
     * 更新userActiveRedis
     * @param string $uid
     * @param int $is_redis 是否判断redis存储成功
     * @return bool|int
     */
    public function getUserActiveRedis($uid = "",$is_redis = 0,$is_redis_set = true){
        if(empty($uid))
        {
            $this->error_return(-1001001);
        }

        $userActiveModel = new UserActiveModel();
        $user_active = $userActiveModel->getInfoById($uid);
        if($user_active === false)
        {
            $this->error_return(-1);
        }
        else if($user_active === 0)
        {
            $this->error_return(-10008);
        }

        if(empty($user_active['pic_head']))
        {
            $user_active['pic_head'] = 0;
        }

        if($is_redis_set === true)
        {
            $redis = new UserRedis();

            $gold_name = $user_active['gold'].','.$user_active['money'].','.$user_active['house'].','.$user_active['name'].','.$user_active['pic_head'].','.$user_active['sex'];

            $res = $redis->setRedis(4,$gold_name,'',$uid,'');
            if($res === false)
            {
                $this->error_return(-2);
            }

            $res = $redis->setRedis(3,$user_active,'',$uid,'');
            if($is_redis == 1 && $res === false)
            {
                $this->error_return(-2);
            }
        }

        return $user_active;
    }

    /**
     * 更新userBaseRedis
     * @param string $user_token
     * @return bool
     */
    public function getUserBaseRedis($user_token = ""){
        if(empty($user_token))
        {
            $this->error_return(-103);
        }
        $userBaseModel = new UserBaseModel();
        $user_base = $userBaseModel->getInfoByToken($user_token);
        if($user_base === false)
        {
            $this->error_return(-1);
        }
        else if(is_numeric($user_base))
        {
            switch ($user_base)
            {
                case 0:
                    $this->error_return(-103);
                    break;
                case -20003:
                    $this->error_return(-20003);
                    break;
                case -20004:
                    $this->error_return(-20004);
                    break;
                case -20005:
                    $this->error_return(-20005);
                    break;
                default:
                    $this->error_return(-20003);
                    break;
            }
        }
        else
        {
            $userRedis = new UserRedis();
            $res = $userRedis->setRedis(1,$user_base['uid'],C('REDIS_TIMEOUT'),'',$user_token);
            if($res === false)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis服务器链接失败'],'statusCode'=>-2]);
            }
            $res = $userRedis->setRedis(2,$user_base,C('UserBaseRedisOutTime'),$user_base['uid']);
            if($res === false)
            {
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'redis服务器链接失败'],'statusCode'=>-2]);
            }
            return $user_base;
        }
    }

    /**
     * 检测字符串是否包含屏蔽词
     * @param $str
     * return true代表没有包含，false代表包含
     */
    public function validationString($str)
    {
        $file = fopen(PUBLIC_PATH."badwords.txt","r") or die("文件打开失败");
        while (!feof($file)){
            $text = fgets($file);
            $text = substr($text,0,strlen($text)-2);
            if(strpos($str,$text) !== false)
            {
                return false;
                break;
            }
        }
        fclose($file);
        return true;
    }

    /**
     * curl https请求获取信息
     * @param $url
     * @return mixed
     */
    public function curl_get_https($url){
        $curl = curl_init(); // 启动一个CURL会话
        curl_setopt($curl, CURLOPT_URL, $url);
        curl_setopt($curl, CURLOPT_HEADER, 0);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false); // 跳过证书检查
        curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, 2);  // 从证书中检查SSL加密算法是否存在
        $tmpInfo = curl_exec($curl);     //返回api的json对象
        //关闭URL请求
        curl_close($curl);
        if($tmpInfo != false)
        {
            $tmpInfo = json_decode($tmpInfo);
            $tmpInfo = $this->object_to_array($tmpInfo);
        }
        else
        {
            Log::write('https请求信息失败,链接是:'.$url,"ERR");
        }
        return $tmpInfo;    //返回json对象
    }

    /**
     * 微信http请求错误日志写入
     * @param $http_res
     */
    public function wechar_http_error_log($http_res,$url)
    {
        Log::write('https请求信息失败,链接是:'.$url.';错误码是:'.$http_res['errcode'].';错误信息是:'.$http_res['errmsg'],"ERR");
        $this->error_return(-151);
    }

    /**
     * 数组 转 对象
     *
     * @param array $arr 数组
     * @return object
     */
    public function array_to_object($arr) {
        if (gettype($arr) != 'array') {
            return;
        }
        foreach ($arr as $k => $v) {
            if (gettype($v) == 'array' || getType($v) == 'object') {
                $arr[$k] = (object)$this->array_to_object($v);
            }
        }

        return (object)$arr;
    }

    /**
     * 对象 转 数组
     *
     * @param object $obj 对象
     * @return array
     */
    public function object_to_array($obj) {
        $obj = (array)$obj;
        foreach ($obj as $k => $v) {
            if (gettype($v) == 'resource') {
                return;
            }
            if (gettype($v) == 'object' || gettype($v) == 'array') {
                $obj[$k] = (array)$this->object_to_array($v);
            }
        }

        return $obj;
    }

    /**
     * 错误返回
     * @param $code
     */
    public function error_return($code,$data = "",$bind = "")
    {
        if(!isset($data['message']))
        {
            $data['message'] = "执行成功";
        }
        switch ($code)
        {
            case 1:
                $this->ajaxReturn(['status'=>'true', 'data' => $data,'statusCode'=> $code]);
                break;

            case -1:
                $this->ajaxReturn(['status'=>'false', 'data' => ['message' => '数据库链接错误'],'statusCode'=> $code]);
                break;
            case -2:
                $this->ajaxReturn(['status'=>'false', 'data' => ['message' => 'redis链接失败'],'statusCode'=> $code]);
                break;

            case -103:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'user_token错误'],'statusCode' => $code]);
                break;
            case -104:
                $message = "多次登录错误,还需要再过:date才能登录";
                $message = $this->bindStr($message,$bind);
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => $message],'statusCode' => $code]);
                break;
            case -145:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '加入我们，搜索官方微信公众号：百赢娱乐'],'statusCode' => $code]);
                break;
            case -146:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要绑定手机号'],'statusCode' => $code]);
                break;
            case -147:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要实名认证'],'statusCode' => $code]);
                break;
            case -148:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '该商品不存在或者以下架'],'statusCode' => $code]);
                break;
            case -149:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递goods_id'],'statusCode' => $code]);
                break;
            case -150:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递uid或者传递的uid与user_token不匹配'],'statusCode' => $code]);
                break;
            case -151:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '微信访问失败'],'statusCode' => $code]);
                break;

            case -10004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '手机号格式不正确'], 'statusCode' => $code]);
                break;

            case -10008:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有找到用户信息'], 'statusCode' => $code]);
                break;
            case -10009:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '输入的内容包含了违禁内容'], 'statusCode' => $code]);
                break;
            case -10010:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递sex或者签名'], 'statusCode' => $code]);
                break;
            case -10011:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '性别传递错误'], 'statusCode' => $code]);
                break;
            case -10012:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '用户抽奖次数已经使用完了'], 'statusCode' => $code]);
                break;
            case -10013:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有找到奖品信息'], 'statusCode' => $code]);
                break;
            case -10014:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '抽奖使用失败'], 'statusCode' => $code]);
                break;
            case -10015:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '奖品获取失败'], 'statusCode' => $code]);
                break;
            case -10016:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '未获取到可以进入的服务器'], 'statusCode' => $code]);
                break;
            case -10017:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递真实姓名'], 'statusCode' => $code]);
                break;
            case -10018:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递身份证号'], 'statusCode' => $code]);
                break;
            case -10019:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '该用户已经实名认证过了'], 'statusCode' => $code]);
                break;
            case -10020:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '请输入正确的名字,长度在2到6位之间'], 'statusCode' => $code]);
                break;
            case -10021:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '身份证号码不正确'], 'statusCode' => $code]);
                break;

            case -11001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有账号可以进入的服务器'], 'statusCode' => $code]);
                break;

            case -20001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '注册失败'], 'statusCode' => $code]);
                break;
            case -20002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '用户token更新失败'], 'statusCode' => $code]);
                break;
            case -20003:
                $this->ajaxReturn(['status' => 'false','data' => ['message' => '用户状态错误'], 'statusCode' => $code]);
                break;
            case -20004:
                $this->ajaxReturn(['status' => 'false','data' => ['message' => '账号已被冻结'], 'statusCode' => $code]);
                break;
            case -20005:
                $this->ajaxReturn(['status' => 'false','data' => ['message' => '该用户是机器人'], 'statusCode' => $code]);
                break;

            case -1001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递uid'], 'statusCode' => $code]);
                break;
            case -1001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '进入游戏失败,请稍后在试'], 'statusCode' => $code]);
                break;
            case -1001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'app_id不能为空'], 'statusCode' => $code]);
                break;
            case -1001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'ip注册被限制'], 'statusCode' => $code]);
                break;
            case -1001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'ip注册被限制'], 'statusCode' => $code]);
                break;

            case -1001006:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '商品id不能为空'], 'statusCode' => $code]);
                break;
            case -1001051:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '微信没有配置'], 'statusCode' => $code]);
                break;

            case -3001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'game_id未定义'], 'statusCode' => $code]);
                break;
            case -3001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '游戏id错误'], 'statusCode' => $code]);
                break;
            case -3001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递游戏类型'], 'statusCode' => $code]);
                break;
            case -3001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有找到庄家信息'], 'statusCode' => $code]);
                break;
            case -3001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '游戏没有记录'], 'statusCode' => $code]);
                break;
            case -3001006:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '起始id格式错误'], 'statusCode' => $code]);
                break;
            case -3001007:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '未达到领取要求'], 'statusCode' => $code]);
                break;

            case -10001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递app_id'], 'statusCode' => $code]);
                break;
            case -10001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有得到商品数据'], 'statusCode' => $code]);
                break;
            case -10001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '商品不属于该应用'], 'statusCode' => $code]);
                break;
            case -10001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => 'id所属包不存在'], 'statusCode' => $code]);
                break;
            case -10001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '创建的订单太多了，请稍后再试'], 'statusCode' => $code]);
                break;
            case -10001006:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递订单号'], 'statusCode' => $code]);
                break;
            case -10001007:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '订单不存在'], 'statusCode' => $code]);
                break;
            case -10001008:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '支付方式不支持'], 'statusCode' => $code]);
                break;
            case -10001009:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '支付失败'], 'statusCode' => $code]);
                break;
            case -10001010:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '努力开发中'], 'statusCode' => $code]);
                break;
            case -10001011:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '已达到每日消费限额'], 'statusCode' => $code]);
                break;
            case -10001012:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '微信访问失败'], 'statusCode' => $code]);
                break;

            case -11001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有得到活动数据'], 'statusCode' => $code]);
                break;
            case -11001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '活动已经关闭'], 'statusCode' => $code]);
                break;
            case -11001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '活动已经结束'], 'statusCode' => $code]);
                break;
            case -11001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '活动尚未开始'], 'statusCode' => $code]);
                break;
            case -11001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '传递的活动信息必须是数组格式'], 'statusCode' => $code]);
                break;

            case -12001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '道具id错误'], 'statusCode' => $code]);
                break;
            case -12001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递使用的道具的次数 '], 'statusCode' => $code]);
                break;
            case -12001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '没有找到该道具类型 '], 'statusCode' => $code]);
                break;
            case -12001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '玩家的道具数量不足 '], 'statusCode' => $code]);
                break;
            case -12001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '商品不能直接使用 '], 'statusCode' => $code]);
                break;
            case -12001006:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '道具暂时无法使用 '], 'statusCode' => $code]);
                break;
            case -12001007:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '道具商品id不能为空 '], 'statusCode' => $code]);
                break;
            case -12001008:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '该商品不存在 '], 'statusCode' => $code]);
                break;
            case -12001009:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '商品价格类型未定义 '], 'statusCode' => $code]);
                break;
            case -12001010:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '玩家的金币不足 '], 'statusCode' => $code]);
                break;
            case -12001011:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '玩家的钻石不足 '], 'statusCode' => $code]);
                break;
            case -12001012:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '道具不存在 '], 'statusCode' => $code]);
                break;
            case -12001013:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '玩家持有道具数已经达到上限 '], 'statusCode' => $code]);
                break;

            case -13001001:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '需要传递邮件id '], 'statusCode' => $code]);
                break;
            case -13001002:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '打开了错误的邮件 '], 'statusCode' => $code]);
                break;
            case -13001003:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '邮件没有携带附件 '], 'statusCode' => $code]);
                break;
            case -13001004:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '附件类型出错 '], 'statusCode' => $code]);
                break;
            case -13001005:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '邮件还有附件'], 'statusCode' => $code]);
                break;
            case -13001006:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '邮件附件已经领取'], 'statusCode' => $code]);
                break;
            case -13001007:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '邮件不存在'], 'statusCode' => $code]);
                break;

            default:
                $this->ajaxReturn(['status' => 'false', 'data' => ['message' => '数据库链接错误'], 'statusCode' => -1]);
                break;
        }
    }

    /**
     * 成功返回
     * @param $data
     * @param string $msg
     */
    public function success_return($data,$msg = "")
    {
        if($msg != "")
        {
            $data['message'] = $msg;
        }
        else
        {
            $data['message'] = "成功";
        }

        $this->ajaxReturn(['status'=>'true', 'data' => $data,'statusCode'=> 1]);
    }

    /**
     * 将对应的数据放入字符串的占位符中
     * @param $str
     * @param $bind
     * @return mixed
     */
    protected function bindStr($str,$bind)
    {
        if(is_array($bind))
        {
            foreach ($bind as $key => $value)
            {
                $str = str_replace(":$key",$value,$str);
            }
        }
        return $str;
    }

    /**
     * 二维数组根据字段排序
     * @array $array
     * @param $field
     * @param string $sort
     * @return mixed
     */
    public function arraySequence($array, $field, $sort = 'SORT_DESC')
    {
        $arrSort = array();
        foreach ($array as $uniqid => $row)
        {
            foreach ($row as $key => $value)
            {
                $arrSort[$key][$uniqid] = $value;
            }
        }
        array_multisort($arrSort[$field], constant($sort), $array);
        return $array;
    }

    /**
     * 游客注册ip限制
     * @param string $ip 游客注册ip
     * @return bool true 表示游客可以注册 false表示游客不可以注册新账号
     */
    protected function Iplimit($ip = '')
    {
        if(C('is_youke_regis') === false)
        {
            return false;
        }
        else if(C('is_youke_regis_ip_limit') === false)
        {
            return true;
        }

        if($ip == "")
        {
            $ip = $_SERVER["REMOTE_ADDR"];
        }

        $userRedis = new UserRedis();

        //获取数据
        $res = $userRedis->getRedis(9,$ip);
        if($res == false)
        {
            $i = 0;

        }
        else
        {
            $i = $res;

        }

        if($i > 5)
        {
            return false;
        }
        else
        {
            $i++;
        }

        $res = $userRedis->setRedis(9,$i,3600,$ip);
        if($res === false)
        {
            $this->error_return(-2);
        }

        return true;
    }

    /**
     * 微信注册ip限制
     * @param $ip
     * @return bool
     */
    public function getWecharIplimit($ip = '')
    {
        if($ip == "")
        {
            $ip = $_SERVER["REMOTE_ADDR"];
        }

        $userRedis = new UserRedis();

        //获取数据
        $res = $userRedis->getRedis(11,$ip);
        if($res == false)
        {
            $i = 0;

        }
        else
        {
            $i = $res;
        }

        if($i > 20)
        {
            return false;
        }
        else
        {
            $i++;
        }

        $res = $userRedis->setRedis(11,$i,3600,$ip);
        if($res === false)
        {
            $this->error_return(-2);
        }

        return true;
    }

    //    /**
//     * 验证手机号是否唯一
//     * @param $mobile
//     */
//    protected function getPhoneIsOne($mobile = "")
//    {
//        if(empty($mobile))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号不能为空'],'statusCode'=>-101]);
//        }
//        else
//        {
//            $userBaseModel = new UserBaseModel();
//            $Regular_phone = C('User.Regular_phone');
//            if(!preg_match($Regular_phone,$mobile)){
//                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'手机号格式不正确'],'statusCode'=>-10003]);
//            }
//            else
//            {
//                //手机号唯一性判断
//                $phone_res = $userBaseModel->getMd5PwdByPhone($mobile);
//                if($phone_res == -1)
//                {
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'数据库链接失败'],'statusCode'=>-1]);
//                }
//                else if($phone_res != 0)
//                {
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'该手机号已经存在'],'statusCode'=>-10005]);
//                }
//            }
//        }
//    }
}