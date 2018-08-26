<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:00
 */

namespace User\Model;


use Think\Log;

class ApplePayModel
{
    //苹果支付接口
    public function apple_pay($apple_receipt,$app_id){
        $jsonData = array('receipt-data'=>$apple_receipt);//这里本来是需要base64加密的，我这里没有加密的原因是客户端返回服务器端之前，已经作加密处理
        $jsonData = json_encode($jsonData);

        $apple_pay_config = C('Apple_Pay');
        if($app_id == "")
        {
            $url = $apple_pay_config['url'];
        }
        else
        {
            //获取包id情况
            $app = C("app_id");
            $app_config = $app[$app_id];
            if($app_config['is_ceshi'])
            {
                $url = $apple_pay_config['ceshi_url'];
            }
            else
            {
                $url = $apple_pay_config['url'];
            }
        }

        $response = $this->http_post_data_apple_validation($url,$jsonData);

        Log::write("苹果支付信息,时间为:".date('Y-m-d H:i:s').",苹果返回信息状态:".$response->{'status'},'INFO');
        if(is_int($response) && $response == -9)
        {
            //curl链接失败无法验证商户收据
            return -9;
        }
        if($response->{'status'} != 0)
        {
            return $response->{'status'};
        }
        //对应的金币或者钻石
        return true;
    }

    //post请求苹果
    private function http_post_data_apple_validation($url,$data_string){
        $curl_handle=curl_init();
        curl_setopt($curl_handle,CURLOPT_URL, $url);
        curl_setopt($curl_handle,CURLOPT_RETURNTRANSFER, true);
        curl_setopt($curl_handle,CURLOPT_HEADER, 0);
        curl_setopt($curl_handle,CURLOPT_POST, true);
        curl_setopt($curl_handle,CURLOPT_POSTFIELDS, $data_string);
        curl_setopt($curl_handle,CURLOPT_SSL_VERIFYHOST, 0);
        curl_setopt($curl_handle,CURLOPT_SSL_VERIFYPEER, 0);
        $response_json = curl_exec($curl_handle);
        if($response_json === false){
            return -9;
        }
        $response = json_decode($response_json);
        curl_close($curl_handle);
        return $response;
    }
}