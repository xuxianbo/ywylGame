<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:20
 */

namespace User\Controller;


use Think\Log;
use User\Model\GoodsModel;
use User\Model\OrderModel;
use User\Model\OrderRedis;
use User\Model\PayModel;

class PayController extends BaseController
{
    /**
     * 创建订单
     * @param $uid 用户id
     * @param $goods_id 商品id
     * @param $buy_type 购买方式  1微信  2支付宝  3苹果
     * @return array  成功返回订单信息
     */
    protected function create_order($uid,$goods_id,$buy_type)
    {

        $orderModel = new OrderModel();

        //获取商品信息
        $goodsModel = new GoodsModel();
        $goods_info = $goodsModel->getInfoById($goods_id);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $this->error_return(-12001008);
        }
        else
        {
            $goods_info = $goods_info[0];
        }


        $user_active = $this->getUserActiveRedis($uid);
        if($user_active['is_buy_money'] - $goods_info['price'] < 0)
        {
            $this->error_return(-10001011);
        }

        //获取用户五分钟内的订单数
        $user_order_config = C('user_order');
        $time = time();
        $res = $orderModel->getOrderNotCountByTime($uid,$time - $user_order_config['order_time'],$time);
        if($res === false)
        {
            $this->error_return(-1);
        }
        else if($res[0]['count_num'] >= $user_order_config['order_num'])
        {
            $this->error_return(-10001005);
        }
        else
        {
            $goods_num = $goods_info['goods_num'] + $goods_info['give_num'];
            //创建订单
            $res = $orderModel->createOrder($uid, $goods_id, $buy_type, $goods_info['price'], $goods_info['type'], $goods_num);
            if ($res === false)
            {
                $this->error_return(-1);
            }
            else
            {
                $order_id = $orderModel->getLastInsID();

                //创建redis
                $orderRedis = new OrderRedis();
                $res = $orderRedis->setRedis(1,1,300,$order_id);

                $order_info = [];
                $order_info['id'] = $order_id;
                $order_info['goods_name'] = $goods_info['name'];
                $order_info['goods_price'] = $goods_info['price'];
                return $order_info;
            }
        }
    }

    /**
     * 完成订单
     * @param string $order_id
     * @param string $dingdan_id
     */
    protected function fishOrder($order_id = "",$dingdan_id = "")
    {
        //确认订单是否存在
        $orderModel = new OrderModel();
        $order_info = $orderModel->getInfoByOrderId($order_id);
        if($order_info === false || $order_info == 0)
        {
            Log::write("订单未找到",'EMERG');
            return false;
        }
        else
        {
            //修改订单状态
            if($order_info['status'] == 0)
            {
                $res = $orderModel->updateStatusByOrderUId($order_id,4);
                if($res === false)
                {
                    Log::write("修改订单状态失败",'EMERG');
                    return false;
                }
                else
                {
                    //完成订单
                    $res= $orderModel->finishOrder($order_id,$dingdan_id);
                    if($res !== true)
                    {
                        Log::write("完成订单失败",'EMERG');
                        return false;
                    }
                    else
                    {
                        $orderRedis = new OrderRedis();
                        $res = $orderRedis->setRedis(1,2,60,$order_id);
                        $this->getUserActiveRedis($order_info['uid']);
                        return true;
                    }
                }
            }
        }
    }

    /**
     * 获取支付流程
     */
    public function getPayProcess()
    {
        $pay_config = C('PAY_CONFIG');
        $res_data['wecharPayProcess'] = $pay_config['wecharPayProcess'];
        $res_data['aliPayProcess'] = $pay_config['aliPayProcess'];
        $this->success_return($res_data);
    }

    //---微信官方支付接口开始---

    /**
     * 微信官方购买
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     */
    public function wecharPay($user_token = '',$uid = '',$app_id = "",$goods_id = "")
    {
        if(empty($app_id))
        {
            $this->error_return(-1001003);
        }
        if(empty($goods_id))
        {
            $this->error_return(-1001006);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $config = C('wechar_pay_config');
        $wechar_pay_config = $config[0];

        //创建订单
        $order_info = $this->create_order($uid,$goods_id,1);

        if($goods_id == 20001)
        {
            $order_info['goods_price'] = 0.01;
        }

        //1.统一下单方法
        $wechatAppPay = new WechatAppPayController($wechar_pay_config['appid'], $wechar_pay_config['mch_id'], $config['notify_url'], $wechar_pay_config['key']);
        $params['body'] = $order_info['goods_name']; //商品名称
        $params['out_trade_no'] = $order_info['id']; //自定义的订单号
        $params['total_fee'] = $order_info['goods_price'] * 100; //订单金额 只能为整数 单位为分
        $params['trade_type'] = 'APP'; //交易类型 JSAPI | NATIVE | APP | WAP
        $result = $wechatAppPay->unifiedOrder( $params );
        if($result === false)
        {
            Log::write('微信官方支付失败，错误为:链接访问失败','ERROR');
            $this->error_return(-10001012);
        }
        else if($result['result_code'] != 'SUCCESS')
        {
            Log::write('微信官方支付失败，错误码为:'.$result['err_code'].'错误为:'.$result['return_msg'],'ERROR');
            $this->error_return(-10001009);
        }

        //2.创建APP端预支付参数
        /** @var TYPE_NAME $result */
        $data = @$wechatAppPay->getAppPayParams( $result['prepay_id'] );

        $data['order_id'] = $order_info['id'];
        //3根据上行取得的支付参数请求支付即可
        $this->success_return($data);
    }

    /**
     * 微信h5支付接口
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     * @param string $goods_id
     */
    public function wecharH5Pay($user_token = '',$uid = '',$app_id = "",$goods_id = "")
    {
        if(empty($app_id))
        {
            $this->error_return(-1001003);
        }
        if(empty($goods_id))
        {
            $this->error_return(-1001006);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $config = C('wechar_pay_config');
        $wechar_pay_config = $config[0];

        //创建订单
        $order_info = $this->create_order($uid,$goods_id,1);

        if($goods_id == 20001)
        {
            $order_info['goods_price'] = 0.01;
        }

        //1.统一下单方法
        $wechatAppPay = new WechatAppPayController($wechar_pay_config['appid'], $wechar_pay_config['mch_id'], $config['notify_url'], $wechar_pay_config['key']);
        $params['body'] = $order_info['goods_name']; //商品名称
        $params['out_trade_no'] = $order_info['id']; //自定义的订单号
        $params['total_fee'] = $order_info['goods_price'] * 100; //订单金额 只能为整数 单位为分
        $params['trade_type'] = 'WAP'; //交易类型 JSAPI | NATIVE | APP | WAP
        $result = $wechatAppPay->unifiedOrder( $params );
        if($result === false)
        {
            Log::write('微信官方支付失败，错误为:链接访问失败','ERROR');
            $this->error_return(-10001012);
        }
        else if($result['result_code'] != 'SUCCESS')
        {
            Log::write('微信官方支付失败，错误码为:'.$result['err_code'].'错误为:'.$result['return_msg'],'ERROR');
            $this->error_return(-10001009);
        }

        $data['order_id'] = $order_info['id'];
        //3根据上行取得的支付参数请求支付即可
        $this->success_return($data);
    }

    /**
     * 微信官方支付回调
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     */
    public function wecharNotify()
    {
        $config = C('wechar_pay_config');
        $wechar_pay_config = $config[0];
        $wechatAppPay = new WechatAppPayController($wechar_pay_config['appid'], $wechar_pay_config['mch_id'], $config['notify_url'], $wechar_pay_config['key']);
        $result = $wechatAppPay->getNotifyData();
        if($result)
        {
            //获取到订单号
            $order_id = $result['out_trade_no'];

            //订单处理
            $res = $this->fishOrder($order_id);

            //返回success
            if($res === true)
            {
                $wechatAppPay->replyNotify();
            }
            else
            {
                Log::write('微信官方支付回调,订单处理错误','ERROR');
            }
        }
        else
        {
            Log::write('微信官方支付回调错误','ERROR');
        }
    }

    //---微信官方支付接口结束---

    //---支付宝第三方支付Bpay接口开始---

    /**
     * 购买
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     * @param string $goods_id
     */
    public function bpayMentAliPayBuy($user_token = '',$uid = '',$app_id = "",$goods_id = "")
    {
        $this->error_return(-10001010);

        if(empty($app_id))
        {
            $this->error_return(-1001003);
        }
        if(empty($goods_id))
        {
            $this->error_return(-1001006);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $config = C('bpayment_config');

        //创建订单
        $order_info = $this->create_order($uid,$goods_id,1);

        if($goods_id == 20001)
        {
            $order_info['goods_price'] = 2;
        }

        $bpayment = new BpaymentController($config['app_id'],$config['notify_url'],$config['return_url'],$config['app_key']);

        //获取支付配置
        $params = [];
        $params['order_id'] = $order_info['id'];
        $params['order_amt'] = $order_info['goods_price'];
        $params['goods_name'] = $order_info['goods_name'];

        $res = $bpayment->aliPayUnifiedOrder($params);
        if($res === false)
        {
            $this->error_return(-10001009);
        }
        else
        {
            $this->success_return(['order_id' => $order_info['id'], 'pay_url' => $res['pay_url']]);
        }
    }

    /**
     * 支付回调
     */
    public function bpayMentNotify()
    {

        $config = C('bpayment_config');
        $bpayment = new BpaymentController($config['app_id'],$config['notify_url'],$config['return_url'],$config['app_key']);
        $data = $bpayment->notify();
        $data['test_time'] = date('Y-m-d H:i:s');
        if($data === false)
        {
            $data['title'] = 'bpayMent支付回调失败';
            $data['type'] = 'FAIL';
            Log::write(json_encode($data,JSON_UNESCAPED_UNICODE),'ERROR');
        }
        else
        {
            //获取到订单号
            $order_id = $data['order_id'];

            //订单处理
            $res = $this->fishOrder($order_id);

            //返回success
            if($res === true)
            {
                echo 'ok';
            }
            else
            {
                Log::write(json_encode($data,JSON_UNESCAPED_UNICODE),'ERROR');
            }

        }
    }

    /**
     * 支付同步返回
     */
    public function bpayMentReturn()
    {
        $this->display('return_buy');
    }

    //---支付宝第三方支付Bpay接口结束---

    //---支付宝第三方支付YxbPay接口开始---

    /**
     * 购买
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     * @param string $goods_id
     */
    public function YxbPayAliPayBuy($user_token = '',$uid = '',$app_id = "",$goods_id = "")
    {
        if(empty($app_id))
        {
            $this->error_return(-1001003);
        }
        if(empty($goods_id))
        {
            $this->error_return(-1001006);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        //创建订单
        $order_info = $this->create_order($uid,$goods_id,2);

        $pay_url = C('YxbConfig.payurl')."/uid/$uid/order_id/".$order_info['id'];
        $this->success_return(['pay_url' => $pay_url ,'order_id' => $order_info['id']]);
    }

    /**
     * 打开购买确认页面
     */
    public function YxbPayPage($uid = '',$order_id = '')
    {
        if(!is_numeric($order_id))
        {
            $this->error_return(-10001006);
        }
        $orderModel = new OrderModel();
        $order_info = $orderModel->getInfoByOrderId($order_id);
        if($order_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($order_info))
        {
            $this->error_return(-10001007);
        }
        else if($order_info['uid'] != $uid)
        {
            $this->error_return(-10001007);
        }
        else
        {

            $YxbConfig = C('YxbConfig');


            $config = [
                'pay_memberid' => $YxbConfig['pay_memberid'],  //商户ID
                'pay_orderid' => $order_id,   //订单号
                'pay_amount' => $order_info['goods_price'],    //交易金额
                'pay_applydate' => date("Y-m-d H:i:s",$order_info['create_time']),  //订单时间
                'pay_bankcode' => $YxbConfig['pay_bankcode'],  //银行编码
                'pay_notifyurl' => $YxbConfig['pay_notifyurl'], //服务端返回地址
                'pay_callbackurl' => $YxbConfig['pay_callbackurl'],   //页面跳转返回地址
                'tongdao' => $YxbConfig['tongdao'],  //支付通道
            ];

            $pay = new YxbPayController($config,$YxbConfig['Md5key']);

            $config['pay_md5sign'] = $pay->getMd5Str();
            $this->assign('YxbConfig',$config);
            $this->assign('goods_price',$order_info['goods_price']);
            $this->assign('goods_num',$order_info['goods_num']);
            $this->assign('action_url',$YxbConfig['tjurl']);
            $this->display('YxbBuy');
        }
    }

    /**
     * 支付回调
     */
    public function YxbPayNotify()
    {
        $data = I('');

        $YxbConfig = C('YxbConfig');

        $config = [
            'pay_memberid' => $YxbConfig['pay_memberid'],  //商户ID
            'pay_orderid' => '',   //订单号
            'pay_amount' => '',    //交易金额
            'pay_applydate' => '',  //订单时间
            'pay_bankcode' => $YxbConfig['pay_bankcode'],  //银行编码
            'pay_notifyurl' => $YxbConfig['pay_bankcode'], //服务端返回地址
            'pay_callbackurl' => $YxbConfig['pay_bankcode'],   //页面跳转返回地址
            'tongdao' => $YxbConfig['tongdao'],  //支付通道
        ];

        $pay = new YxbPayController($config,$YxbConfig['Md5key']);

        $sign_data = [
            "memberid" => $data["memberid"], // 商户ID
            "orderid" =>  $data["orderid"], // 订单号
            "amount" =>  $data["amount"], // 交易金额
            "datetime" =>  $data["datetime"], // 交易时间
            "returncode" => $data["returncode"]
        ];
        $sign = $pay->getNotifySign($sign_data);
        if($sign == $data['sign'])
        {
            if ($data["returncode"] == "00") {
                //获取到订单号
                $order_id = $data['orderid'];

                //订单处理
                $res = $this->fishOrder($order_id);

                //返回success
                if($res === true)
                {
                    echo 'ok';
                }
                else
                {
                    Log::write(json_encode($data,JSON_UNESCAPED_UNICODE),'ERROR');
                }
            }
            else
            {
                Log::write(json_encode($data,JSON_UNESCAPED_UNICODE),'ERROR');
            }
        }
        else
        {
            Log::write(json_encode($data,JSON_UNESCAPED_UNICODE),'ERROR');
        }
    }

    /**
     * 支付同步返回
     */
    public function YxbPayReturn()
    {
        $this->display('return_buy');
    }

    //---支付宝第三方支付YxbPay接口结束---

    /**
     * 测试支付回调
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     */
    public function TestNotify()
    {
        $model = new OrderModel();
        $order_list = $model->query('select * from user_order where status = 0');
        if($order_list === false)
        {
            echo 'false';
        }
        else if(empty($order_list))
        {
            echo 'null';
        }
        else
        {
            foreach ($order_list as $k => $v)
            {
                //订单处理
                $res = $this->fishOrder($v['order_id']);

                //返回success
                if($res === true)
                {
                    echo $v['order_id'].'_true';
                    echo "</br>";
                    $orderRedis = new OrderRedis();
                    $res = $orderRedis->getRedis(1,$v['order_id']);
                    echo $res;
                }
                else
                {
                    echo $v['order_id'].'_false';
                }

                echo "</br>";
            }
        }
    }

    /**
     * 订单号测试创建
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     * @param string $goods_id
     */
    public function TestCreateOrder($user_token = '',$uid = '',$app_id = "",$goods_id = "",$buy_type)
    {
        $orderModel = new OrderModel();

        //创建订单号
        $order_id = time().$buy_type.$app_id.$uid;
        dump($order_id);
    }
}