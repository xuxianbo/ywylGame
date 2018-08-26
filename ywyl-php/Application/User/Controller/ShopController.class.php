<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:37
 */

namespace User\Controller;


use Think\Log;
use User\Model\ApplePayModel;
use User\Model\GoodsModel;
use User\Model\OrderModel;
use User\Model\PayModel;
use User\Model\UserActiveModel;
use User\Model\UserCardsModel;

class ShopController extends BaseController
{
    /**
     * 获取商城商品信息
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     */
    public function getShopInfo($user_token = "", $uid = "" ,$app_id = ""){
        if(empty($app_id))
        {
            $this->error_return(-10001001);
        }

        $app_config = C('app_id');
        if(empty($app_config))
        {
            $this->error_return(-10001004);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $goodsModel = new GoodsModel();

        //获取商品信息
        $goods_info = $goodsModel->getInfoByAppId($app_id);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $this->error_return(-10001002);
        }

        //获取包对应的支付方式
        $buy_type = $app_config[$app_id]['pay_type'];

        $this->success_return(['goods_info' => $goods_info, 'pay_type' => $buy_type]);
    }

    /**
     * 获取商品信息
     * @param string $user_token
     * @param string $uid
     * @param string $goods_id
     */
    public function getGoodsInfo($user_token = "", $uid = "", $goods_id = "", $app_id = "")
    {
        if(empty($goods_id)){
            $this->error_return(-149);
        }

        if(empty($app_id))
        {
            $this->error_return(-10001001);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $goodsModel = new GoodsModel();
        $goods_info = $goodsModel->getInfoById($goods_id);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $this->error_return(-148);
        }

        if($goods_info[0]['app_id'] != $app_id)
        {
            $this->error_return(-10001003);
        }
        $this->success_return($goods_info[0]);

    }

    /**
     * 支付订单状态确认
     * @param string $user_token
     * @param string $uid
     * @param string $order_id
     */
    public function userBuyStatus($user_token = "",$uid = "",$order_id = ""){
        if(empty($order_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递order_id'],'statusCode'=>-101]);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
        }

        //根据订单id和用户id查询订单信息
        $orderModel = new OrderModel();
        $orderInfo = $orderModel->getInfoByOrderId($order_id);
        if($orderInfo === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'获取订单信息失败'],'statusCode'=>-1]);
        }
        if($orderInfo == 0)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'未找到该订单'],'statusCode'=>0]);
        }

        $this->ajaxReturn(['status'=>'true','data'=>['order_status'=>$orderInfo['status']],'statusCode'=>1]);
    }

    /**
     * 苹果支付购买商品
     * @param string $user_token
     * @param string $uid
     * @param string $goods_id
     * @param string $buy_type
     */
    public function userBuyShopApple($user_token = "",$uid = "",$goods_id = "",$buy_type = ""){
        if(empty($goods_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递goods_id'],'statusCode'=>-101]);
        }
        if(empty($buy_type))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递buy_type'],'statusCode'=>-101]);
        }
        if($buy_type != 3)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付方式错误'],'statusCode'=>-603]);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }


        //获取订单号
        $orderModel = new OrderModel();
        $res = $orderModel->createOrder($uid,$goods_id,$buy_type);
        if($res === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'订单创建失败'],'statusCode'=>-1]);
        }
        $order_id = $orderModel->getLastInsID();

        $this->ajaxReturn(['status'=>'true','data'=>['order_id'=>$order_id],'statusCode'=>1]);
    }

    /**
     * 苹果支付确认
     * @param string $user_token
     * @param string $uid
     * @param string $order_id
     * @param string $apple_receipt
     * @param string $app_id
     */
    public function applePayVerify($user_token = "",$uid = "",$order_id = "",$apple_receipt = "",$app_id = ""){
        if(empty($order_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递order_id'],'statusCode'=>-101]);
        }
        if(empty($apple_receipt))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递apple_receipt'],'statusCode'=>-101]);
        }
        if(empty($app_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递app_id'],'statusCode'=>-101]);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
        }

        //确认苹果支付
        $applePayModel = new ApplePayModel();
        $res = $applePayModel->apple_pay($apple_receipt,$app_id);
        if($res !== true)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'确认支付失败'],'statusCode'=>$res]);
        }

        //修改订单状态完成订单
        $orderModel = new OrderModel();

        $order_info = $orderModel->getInfoByOrderId($order_id);
        if($order_info === false)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'获取订单信息失败'],'statusCode'=>-1]);
        }
        if($order_info == 0)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'未找到该订单'],'statusCode'=>0]);
        }
        if($order_info['status'] == 0)
        {
            $res = $orderModel->updateStatusByOrderUId($order_id,4);
            if($res !== true)
            {
                $log = "订单完成失败,订单号是$order_id";
                Log::write($log,'EMERG');
                $this->ajaxReturn(['status'=>'false','data'=>['message'=>'完成订单失败'],'statusCode'=>-1]);
            }
            else
            {
                $res= $orderModel->finishOrder($order_id,$apple_receipt);
                if($res !== true)
                {
                    $log = "订单完成失败,订单号是$order_id";
                    Log::write($log,'EMERG');
                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'完成订单失败'],'statusCode'=>-1]);
                }
            }
        }
        $this->getUserActiveRedis($user_base['uid']);
        $this->ajaxReturn(['status'=>'true','data'=>['message'=>'苹果支付成功'],'statusCode'=>1]);
    }

    /**
     * 第三方购买商品
     * @param string $user_token
     * @param string $uid
     * @param string $goods_id
     * @param string $buy_type
     */
    public function userBuyShop($user_token = "",$uid = "",$goods_id = "",$buy_type = "")
    {
        if(empty($goods_id))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递goods_id'],'statusCode'=>-101]);
        }
        if(empty($buy_type))
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递buy_type'],'statusCode'=>-101]);
        }
        if($buy_type != 1 && $buy_type != 2)
        {
            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付方式错误'],'statusCode'=>-603]);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        //获取商品信息
        $goodsModel = new GoodsModel();
        $goods_info = $goodsModel->getInfoById($goods_id);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $this->error_return(-148);
        }
        else
        {
            $orderModel = new OrderModel();

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
                //创建订单
                $res = $orderModel->createOrder($uid,$goods_id,$buy_type);
                if($res === false)
                {
                    $this->error_return(-1);
                }
                else
                {
                    $order_id = $orderModel->getLastInsID();
                    //构建支付链接
                    $pay_url = $_SERVER['SERVER_NAME'];
                    $pay_url .= U('buyGoods',['user_token' => $user_token,'uid' => $uid, 'order_id' => $order_id]);

                    //返回支付链接
                    $function_res['order_id'] = $order_id;
                    $function_res['pay_url'] = $pay_url;
                    $this->ajaxReturn(['status'=>'true','data'=>$function_res,'statusCode'=>1]);
                }
            }
        }
    }

    /**
     * 第三方支付确认页面
     * @param string $user_token
     * @param string $uid
     * @param string $order_id
     */
    public function buyGoods($user_token = "",$uid = "",$order_id = "")
    {
        if(empty($order_id))
        {
            $this->error_return(-10001006);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $orderModel = new OrderModel();

        //获取订单详情
        $order_info = $orderModel->getInfoByOrderId($order_id);
        if($order_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($order_info))
        {
            $this->error_return(-10001007);
        }


        $goodsModel = new GoodsModel();

        //根据订单获取商品id
        $goods_info = $goodsModel->getInfoById($order_info['goods_id']);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $this->error_return(-148);
        }
        else
        {
            $goods_info = $goods_info[0];
        }

        //构建支付配置
        $pay_config = C('lai_dian_fu_config');
        switch ($order_info['type'])
        {
            case 1:
                $pay_config['paytype'] = 'alipaywap';
                break;
            case 2:
                $pay_config['paytype'] = 'wxh5';
                break;
            default:
                $this->error_return(-10001008);
                break;
        }

        if(!empty($total_fee))
        {
            $pay_config['total_fee'] = $goods_info['price'];
        }

        $pay_config['sdorderno'] = $order_id;
        //构建签名
        $pay_config['sign'] = md5('version='.$pay_config['version'].'&customerid='.$pay_config['customerid'].'&total_fee='.$pay_config['total_fee'].'&sdorderno='.$pay_config['sdorderno'].'&notifyurl='.$pay_config['notifyurl'].'&returnurl='.$pay_config['returnurl'].'&'.$pay_config['app_key']);

        $this->assign('pay_config',$pay_config);
        $this->assign('goods_info',$goods_info);
        $this->assign('order_info',$order_info);

        $this->display('pay');
    }

//    /**
//     * 第三方购买商品
//     * @param string $user_token
//     * @param string $uid
//     * @param string $goods_id
//     * @param string $buy_type
//     * @param string $app_id
//     */
//    public function userBuyShop($user_token = "",$uid = "",$goods_id = "",$buy_type = "",$app_id =""){
//        if(empty($goods_id))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递goods_id'],'statusCode'=>-101]);
//        }
//        if(empty($buy_type)){
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递buy_type'],'statusCode'=>-101]);
//        }
//        if($buy_type != 1 && $buy_type != 2)
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付方式错误'],'statusCode'=>-603]);
//        }
//        if(empty($app_id))
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递app_id'],'statusCode'=>-101]);
//        }
//
//        $user_base = $this->getUserBaseByToken($user_token);
//        if($uid != $user_base['uid'] || $uid == "")
//        {
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'需要传递uid或者传递的uid与user_token不匹配'],'statusCode'=>-150]);
//        }
//
//        //获取订单号
//        $orderModel = new OrderModel();
//        $orderModel->startTrans();
//        $order_id = $orderModel->createOrder($uid,$goods_id,$buy_type);
//        if($order_id === false)
//        {
//            $orderModel->rollback();
//            $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付失败，请稍后在试'],'statusCode'=>-1]);
//        }
//
//        //获取支付链接
//        $pay = new PayModel();
//        $pay_url = $pay->pay($app_id,$buy_type,$order_id);
//        if(is_numeric($pay_url))
//        {
//            $orderModel->rollback();
//            switch ($pay_url)
//            {
//                case -20001:
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付失败，请稍后在试'],'statusCode'=>-20001]);
//                    break;
//                case -20002:
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付失败，请稍后在试'],'statusCode'=>-20002]);
//                    break;
//                case -20003:
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付失败，请稍后在试'],'statusCode'=>-20003]);
//                    break;
//                default:
//                    $this->ajaxReturn(['status'=>'false','data'=>['message'=>'支付失败，请稍后在试'],'statusCode'=>-20004]);
//                    break;
//            }
//        }
//        else
//        {
//            $orderModel->commit();
//        }
//
//        //返回支付链接
//        $function_res['order_id'] = $order_id;
//        $function_res['pay_url'] = $pay_url;
//        $this->ajaxReturn(['status'=>'true','data'=>$function_res,'statusCode'=>1]);
//    }
}