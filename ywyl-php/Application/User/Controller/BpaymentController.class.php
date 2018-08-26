<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/6/25
 * Time: 14:48
 */
namespace User\Controller;

use Think\Log;

class BpaymentController
{
    //接口API 正式环境域名
    const API_URL_PREFIX = 'http://bpayment.maiduopay.com';

    //接口API 正式环境ip
    const API_IP_PREFIX = 'http://47.106.19.205';

    //支付宝接口URL
    const ALIPAY_URL = "/Pay/GateWayAliPay.aspx";

    //微信，QQ钱包，京东钱包接口URL
    const GATEWAY_URL = "/Pay/GateWayTencent.aspx";

    //公众账号ID
    private $appid;

    /**
     * 支付方式
     * 微信：
     * 1=PC扫码支付，2=手机WAP支付,3=手机公众号支付（支付宝网关），4=QQ钱包 5=京东钱包
     * 支付宝：
     * 1=PC扫码支付，2=手机WAP支付,3=支付宝网关
    */
    private $pay_type;

    //商户订单号
    private $order_id;

    //订单金额
    private $order_amt;

    //支付结果异步通知URL
    private $notify_url;

    //支付返回URL
    private $return_url;

    //商品名称
    private $goods_name;

    //时间戳
    private $time_stamp;

    //用户ip
    private $user_ip;

    //签名
    private $sign;

    //用户密钥
    private $key;

    //所有参数
    private $params = array();

    public function __construct($appid,$notify_url,$return_url,$key)
    {
        $this->appid = $appid;
        $this->notify_url = $notify_url;
        $this->return_url = $return_url;
        $this->key = $key;
    }

    /**
     * 支付宝下单
     * @param $params
     * @return array|bool|mixed
     */
    public function aliPayUnifiedOrder($params)
    {
        $this->pay_type = isset($params['pay_type']) ? $params['pay_type'] : 2 ;
        $this->order_id = $params['order_id'];
        $this->order_amt = $params['order_amt'];
        $this->goods_name = $params['goods_name'];

        $this->time_stamp = date('YmdHis');
        $this->user_ip = $_SERVER['REMOTE_ADDR'];
        $this->params['app_id'] = $this->appid;
        $this->params['pay_type'] = $this->pay_type;
        $this->params['order_id'] = $this->order_id;
        $this->params['order_amt'] = $this->order_amt;
        $this->params['notify_url'] = $this->notify_url;
        $this->params['return_url'] = $this->return_url;
        $this->params['goods_name'] = $this->goods_name;
        $this->params['time_stamp'] = $this->time_stamp;
        $this->params['user_ip'] = $this->user_ip;

        $this->sign = $this->getPaySign($this->params);
        $this->params['sign'] = $this->sign;
        $this->params['notify_url'] = urlencode(mb_convert_encoding($this->notify_url, 'utf-8', 'gb2312'));
        $this->params['return_url'] = urlencode(mb_convert_encoding($this->return_url, 'utf-8', 'gb2312'));

        $response = $this->post_http_curl($this->params,self::API_URL_PREFIX.self::ALIPAY_URL);
        if( !$response ){
            Log::write('Bpayment访问失败','ERROR');
            return false;
        }

        $result = json_decode($response);
        $result = $this->object_to_array($result);

        if($result['status_code'] == -1)
        {
            Log::write(json_encode($result,JSON_UNESCAPED_UNICODE),'ERROR');
            return false;
        }
        return $result;
    }

    /**
     * 获取支付签名
     * @param $params
     * @return string
     */
    public function getPaySign($params)
    {
        //组装字符串
        $str = 'app_id='.$params['app_id'].'&pay_type='.$params['pay_type'].'&order_id='.$params['order_id'].'&order_amt='.$params['order_amt'].'&notify_url='.$params['notify_url'].'&return_url='.$params['return_url'].'&time_stamp='.$params['time_stamp'].'&key='.md5($this->key);

        //MD5加密
        $string = md5($str);

        //所有字符转小写
        $result = strtolower($string);
        return $result;
    }

    /**
     * 通知处理
     * @return bool
     */
    public function notify()
    {
        $data = $_POST;
        if($data['app_id'] != $this->appid)
        {
            return false;
        }

        $sign = $this->getNotifySign($data);
        if($sign != $data['sign'])
        {
            return false;
        }

        return $data;
    }

    /**
     * 获取通知签名
     * @param $data
     * @return string
     */
    public function getNotifySign($data)
    {
        //组装字符串
        $str = 'app_id='.$data['app_id'].'&order_id='.$data['order_id'].'&pay_seq='.$data['pay_seq'].'&pay_amt='.$data['pay_amt'].'&pay_result='.$data['pay_result'].'&key='.md5($this->key);

        //MD5加密
        $string = md5($str);

        //所有字符转小写
        $result = strtolower($string);
        return $result;
    }

    /**
     * post请求
     * @param $post
     * @param $url
     * @return bool|mixed
     */
    public function post_http_curl($post,$url)
    {
        $options = array(
            CURLOPT_RETURNTRANSFER =>true,
            CURLOPT_HEADER =>false,
            CURLOPT_POST =>true,
            CURLOPT_POSTFIELDS => $post,
        );
        $ch = curl_init($url);
        curl_setopt_array($ch, $options);
        $result = curl_exec($ch);
        if($result){
            curl_close($ch);
            return $result;
        } else {
            $error = curl_errno($ch);
            curl_close($ch);
            return false;
        }
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
}