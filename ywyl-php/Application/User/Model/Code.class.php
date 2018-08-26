<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/10/12
 * Time: 16:29
 */

namespace User\Model;

ini_set("display_errors", "on");

vendor('aliyunSmS.api_sdk.vendor.autoload');

use Aliyun\Api\Sms\Request\V20170525\SendSmsRequest;
use Aliyun\Core\DefaultAcsClient;
use Aliyun\Core\Profile\DefaultProfile;
use Aliyun\Core\Config;
use Think\Log;

// 加载区域结点配置
Config::load();


class Code
{
    static $acsClient = null;

    /**
     * 生成验证码
     * @param int $mobile
     * @param string $scene
     * @return mixed
     */
    public function createCode($mobile,$scene){
        $code = rand(1000,9999);
        $redisModel = new CodeRedis();

        //获取当前ip请求发送短信次数
//        $ip = $_SERVER['REMOTE_ADDR'];
//        $createCodeNum = $redisModel->getRedis(1,$ip);
//        if($createCodeNum === false){
//            $createCodeNum = 0;
//            $code_num_res = $redisModel->setRedis(1,$createCodeNum,C("CODE.CODE_RECODE_TIME"),$ip);
//            if($code_num_res === false){
//                return -2;
//            }
//        }
//        if(empty($createCodeNum)){
//            $createCodeNum = 0;
//        }
//
//        //判断发送短信次数
//        if($createCodeNum >= C("CODE.CODE_IP_DAY_NUM")){
//            return -15;
//        }

        //验证码创建与否验证
        $mod_res = $redisModel->getRedis(2,'',$mobile,$scene);
        if(!empty($mod_res)){
            return -10016;
        }

        //请求短信发送次数加1
//        $res = $redisModel->incRedis(1,1,$ip);
//        if($res === false){
//            return $this->returnDate("false",['message'=>"发送记录存储失败"]);
//        }

        //获取验证码有效期
        $time = C('SMS_CODE.EXPIRATION_TIME');

        //将验证码信息和手机号写入redis缓存
        $code_res = $redisModel->setRedis(2,$code,$time,'',$mobile,$scene);
        if($code_res === false){
            return -2;
        }
        //发送短信
        $res = $this->smsCode($mobile,['code' => $code]);
        Log::write('发送手机号'.$mobile.',返回的状态码是:'.$res->Code.',发送时间是:'.date('Y/m/d H:i:s'),'INFO');
        if($res->Code == "OK"){
            return 1;
        }else{
            if($res->Code == "isv.BUSINESS_LIMIT_CONTROL"){
                return -10016;
            }else{
                Log::write("发送短信失败,错误提示为:$res->Code",'EMERG');
                return -105;
            }
        }
    }

    /**
     * 发送验证短信
     * @param $mobile
     * @param $code
     * @param $signName
     * @param $templateCode
     * @param $phoneNumbers
     * @param null $templateParam
     * @param null $outId
     * @param null $smsUpExtendCode
     * @return mixed
     */
    public function smsCode($mobile,$templateParam = null, $outId = null, $smsUpExtendCode = null){
        $config = C('ALIYUN_SMS');
        // 初始化SendSmsRequest实例用于设置发送短信的参数
        $request = new SendSmsRequest();

        // 必填，设置雉短信接收号码
        $request->setPhoneNumbers($mobile);

        // 必填，设置签名名称
        $request->setSignName($config['signName']);

        // 必填，设置模板CODE
        $request->setTemplateCode($config['templateCode']);

        // 可选，设置模板参数
        if($templateParam) {
            $request->setTemplateParam(json_encode($templateParam));
        }

        // 可选，设置流水号
        if($outId) {
            $request->setOutId($outId);
        }

        // 选填，上行短信扩展码
        if($smsUpExtendCode) {
            $request->setSmsUpExtendCode($smsUpExtendCode);
        }

        // 发起访问请求
        $acsResponse = static::sendPost()->getAcsResponse($request);

        return $acsResponse;
    }

    /**
     * 验证码对比
     * @param $key
     * @param $user_code
     * @return mixed
     */
    public function conCode($mobile,$scene,$user_code){
        $redisModel = new CodeRedis();
        //链接redis 获取验证码
        $code = $redisModel->getRedis(2,'',$mobile,$scene);
        if(empty($code))
        {
                return -10014;
            }
            else
            {
                if($code == (int)$user_code)
                {
                    $redisModel->delRedis(2,'',$mobile,$scene);
                    return 1;
                }
                else
                {
                    return -10013;
                }
        }
    }

    /**
     * 请求发送
     * @return null
     */
    public static function sendPost() {
        $config = C('ALIYUN_SMS');

        //产品名称:云通信流量服务API产品,开发者无需替换
        $product = $config['product'];

        //产品域名,开发者无需替换
        $domain = $config['domain'];

        $accessKeyId = $config['accessKeyId'];

        $accessKeySecret = $config['accessKeySecret'];

        // 暂时不支持多Region
        $region = $config['region'];

        // 服务结点
        $endPointName = $config['endPointName'];

        if(static::$acsClient == null) {
            //初始化acsClient,暂不支持region化
            $profile = DefaultProfile::getProfile($region, $accessKeyId, $accessKeySecret);

            // 增加服务结点
            DefaultProfile::addEndpoint($endPointName, $region, $product, $domain);

            // 初始化AcsClient用于发起请求
            static::$acsClient = new DefaultAcsClient($profile);
        }
        return static::$acsClient;
    }
}