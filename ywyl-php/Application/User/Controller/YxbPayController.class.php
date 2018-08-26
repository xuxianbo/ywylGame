<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/8/14
 * Time: 14:48
 */
namespace User\Controller;

class YxbPayController
{
    private $pay_config = [
        'pay_memberid' => '',   //商户ID
        'pay_orderid' => '',   //订单号
        'pay_amount' => '',    //交易金额
        'pay_applydate' => '',  //订单时间
        'pay_bankcode' => '',  //银行编码
        'pay_notifyurl' => '', //服务端返回地址
        'pay_callbackurl' => '',   //页面跳转返回地址
    ];

    public function __construct($config,$md5key)
    {
        $this->pay_config['pay_memberid'] = $config['pay_memberid'];
        $this->pay_config['pay_orderid'] = $config['pay_orderid'];
        $this->pay_config['pay_amount'] = $config['pay_amount'];
        $this->pay_config['pay_applydate'] = $config['pay_applydate'];
        $this->pay_config['pay_bankcode'] = $config['pay_bankcode'];
        $this->pay_config['pay_notifyurl'] = $config['pay_notifyurl'];
        $this->pay_config['pay_callbackurl'] = $config['pay_callbackurl'];
        $this->md5key = $md5key;
        ksort($this->pay_config);
        reset($this->pay_config);
    }

    //md5签名
    public function getMd5Str()
    {
        $md5str = "";
        foreach ($this->pay_config as $key => $val) {
            $md5str = $md5str . $key . "=>" . $val . "&";
        }
        $sign = strtoupper(md5($md5str . "key=" . $this->md5key));
        return $sign;
    }

    //回调签名
    public function getNotifySign($returnConfig)
    {
        ksort($returnConfig);
        reset($returnConfig);
        $md5str = "";
        foreach ($returnConfig as $key => $val) {
            if($key != 'sign' && $key != 'reserved1' && $key != 'reserved2' && $key != 'reserved3')
            {
                $md5str = $md5str . $key . "=>" . $val . "&";
            }
        }
        $sign = strtoupper(md5($md5str . "key=" . $this->md5key));
        return $sign;
    }
}