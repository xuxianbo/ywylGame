<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/4/13
 * Time: 11:17
 */

namespace User\Controller;


use User\Model\PropsModel;
use User\Model\UserActiveModel;

class PropsController extends BaseController
{
    /**
     * 获取玩家拥有的道具
     * @param string $user_token   用户密钥
     * @param string $uid   用户id
     * @param string $props_id   道具id
     */
    public function getUserProps($user_token = "",$uid = "",$props_id = "")
    {
        if(is_numeric($props_id) === false && $props_id != "")
        {
            $this->error_return(-12001001);
        }

        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $propsModel = new PropsModel();
        $props_info = $propsModel->getPropsInfo($uid,$props_id);
        if($props_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($props_info))
        {
            $props_info = [];
        }
        else
        {
            foreach ($props_info as $k => $v)
            {
                if($v['props_id'] == 300001)
                {
                    $userActiveModel = new UserActiveModel();
                    $info = $userActiveModel->getInfoById($uid);
                    if($info === false)
                    {
                        $this->error_return(-1);
                    }
                    if($info['broadcast_num'] == 0)
                    {
                        unset($props_info[$k]);
                    }
                    else
                    {
                        $props_info[$k]['props_num'] = $info['broadcast_num'];
                    }
                }
            }
        }

        $props_info = array_values($props_info);
        $this->success_return(['props_info' => $props_info],'玩家拥有的道具');
    }

    /**
     * 玩家使用道具
     * @param string $user_token   用户密钥
     * @param string $uid   用户id
     * @param string $props_id   道具id
     * @param string $props_use_num   道具使用次数
     */
    public function useUserProps($user_token = "",$uid = "",$props_id = "",$props_use_num = "",$add_parameters = "")
    {
        //判断使用次数
        if($props_use_num == "")
        {
            $props_use_num = 1;
        }

        //判断使用道具id
        if(is_numeric($props_id) === false)
        {
            $this->error_return(-12001001);
        }

        $user_base = $this->getUserBaseByToken($user_token);

        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }
        $propsModel = new PropsModel();

        //获取道具信息
        $props_info = $propsModel->getPropsInfo($uid,$props_id);
        if($props_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($props_info))
        {
            $this->error_return(-12001003);
        }
        else
        {

            //判断道具是否可以直接使用
            if($props_info['is_use'] != 1)
            {
                $this->error_return(-12001005);
            }

            //判断用户道具数量是否足够
            $use_number = $props_use_num * $props_info['props_use_num'];
            if($use_number > $props_info["props_num"])
            {
                $this->error_return(-12001004);
            }

            //开始使用道具
            $propsModel->startTrans();

            //减少玩家持有道具
            $res = $propsModel->reduceProps($uid,$props_id,$use_number);
            if($res === false)
            {
                $propsModel->rollback();
                $this->error_return(-1);
            }
            else
            {
                //添加玩家道具使用记录
                $res = $propsModel->addPropsUseRecord($uid,$props_id,$use_number,time());
                if($res === false)
                {
                    $propsModel->rollback();
                    $this->error_return(-1);
                }
                else
                {
                    //计算给与玩家的数量
                    $give_num = $props_info['give_num'] * $props_use_num;

                    //根据道具类型使用道具
                    switch ($props_info['props_type'])
                    {
                        case 1:
                            $userActiveModel = new UserActiveModel();
                            $res = $userActiveModel->updateGoldByUid($uid,$give_num);
                            break; //给予玩家金币
                        case 2:
                            $userActiveModel = new UserActiveModel();
                            $res = $userActiveModel->updateDiamondByUid($uid,$give_num);
                            break; //给予玩家钻石
                        case 4:
                            //处理额外传输数据
                            $add_parameters = explode(',',$add_parameters);
                            $phone = $add_parameters[0];
                            //判断手机号是否符合
                            $Regular_phone = C('User.Regular_phone');
                            if(!preg_match($Regular_phone,$phone))
                            {
                                $propsModel->rollback();
                                $this->error_return(-10004);
                            }
                            //处理添加话费申请记录
                            $res = $propsModel->addPhoneMoneyRecord($uid,$phone,$give_num,time());
                            break; //给予玩家话费
                        default:
                            $propsModel->rollback();
                            $this->error_return(-12001006);
                            break;
                    }

                    if($res === false)
                    {
                        $propsModel->rollback();
                        $this->error_return(-1);
                    }
                    else
                    {
                        $propsModel->commit();
                        $this->getUserActiveRedis($uid);
                        $this->success_return(['props_num' => $props_info["props_num"] - $use_number, 'give_num' => $give_num, 'props_type' => $props_info['props_type'], 'props_id' => $props_info['props_id']],'道具使用成功');
                    }
                }
            }
        }
    }

    /**
     * 获取道具配置
     * @param string $user_token
     * @param string $uid
     */
    public function getPropsConfig($user_token = "",$uid = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);

        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }
        $propsModel = new PropsModel();

        $info = $propsModel->getPropsConfig();
        if($info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($info))
        {
            $info = [];
        }

        $this->success_return($info,'获取成功');
    }

    /**
     * 获取道具商城
     * @param string $user_token
     * @param string $uid
     * @param string $app_id
     */
    public function getPropsGoods($user_token = "",$uid = "",$app_id = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }
        $propsModel = new PropsModel();

        //获取道具商城
        $shop_info = $propsModel->getPropsShop($app_id);
        if($shop_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($shop_info))
        {
            $shop_info = [];
        }

        //获取超值特惠商品
        $goods_info = $propsModel->getPropsShopByStatus($app_id,2);
        if($goods_info === false)
        {
            $this->error_return(-1);
        }
        else if(empty($goods_info))
        {
            $goods_info = [];
        }

        $this->success_return(['shop_info' => $shop_info, 'goods_info' => $goods_info]);
    }

    /**
     * 购买道具商品
     * @param string $user_token
     * @param string $uid
     * @param string $goods_id
     */
    public function buyPropsGoods($user_token = "",$uid = "",$goods_id = "")
    {
        if(empty($goods_id))
        {
            $this->error_return(-12001007);
        }
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $propsModel = new PropsModel();
        $goods_info = $propsModel->getPropsGoodsInfoById($goods_id);
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
            $price = $goods_info['price'];
            $price_type = $goods_info['price_type'];

            //获取用户信息
            $userActiveModel = new UserActiveModel();
            $user_active = $userActiveModel->getInfoById($uid);
            if($user_active === false)
            {
                $this->error_return(-1);
            }
            else if($user_active == 0)
            {
                $this->error_return(-10008);
            }

            $props_id = $goods_info['props_id'];

            //获取道具配置信息
            $props_info = $propsModel->getPropsConfigInfo($props_id);
            if($props_info === false)
            {
                $this->error_return(-1);
            }
            else if(empty($props_info))
            {
                $this->error_return(-12001012);
            }
            else
            {
                $props_info = $props_info[0];
            }

            $give_num = $goods_info['give_num'];

            //获取用户是否拥有该道具
            $user_props = $propsModel->getPropsInfo($uid,$props_id);
            if($user_props === false)
            {
                $this->error_return(-1);
            }
            else if(!empty($user_props))
            {
                if($user_props[0]['props_num'] + $give_num >  $props_info['props_max_num'])
                {
                    $this->error_return(-12001013);
                }
            }

            $props_num = $give_num;

            $userActiveModel->startTrans();

            //扣除用户对应的货币
            $res = false;
            switch ($price_type)
            {
                case 1:
                    if($price > $user_active['gold'])
                    {
                        $this->error_return(-120010010);
                    }
                    $res = $userActiveModel->updateGoldByUid($uid,-$price);
                    break; //金币
                case 2:
                    if($price > $user_active['diamond'])
                    {
                        $this->error_return(-12001011);
                    }
                    $res = $userActiveModel->updateDiamondByUid($uid,-$price);
                    break; //钻石
                default:
                    $this->error_return(-12001009);
                    break;
            }
            if($res !== true){
                $userActiveModel->rollback();
                $this->error_return(-1);
            }

            if($props_id == 300001)
            {
                if($user_active['broadcast_num'] + $give_num >  $props_info['props_max_num'])
                {
                    $this->error_return(-12001013);
                }
                $res = $userActiveModel->setUserBroadcastNum($uid,$give_num);
                if($res === false)
                {
                    $this->error_return(-1);
                }
                $give_num = 1;
            }

            //给与用户对应的道具数
            if(empty($user_props))
            {
                $res = $propsModel->addUserProps($uid,$props_id,$give_num);
                if($res === false)
                {
                    $userActiveModel->rollback();
                    $this->error_return(-1);
                }
            }
            else
            {
                $res = $propsModel->setUserPropsNum($uid,$props_id,$give_num);
                if($res === false)
                {
                    $userActiveModel->rollback();
                    $this->error_return(-1);
                }
                $props_num = $user_props[0]['props_num'];
            }

            $res = $propsModel->addPropsGoodsNum($goods_id);
            if($res === false)
            {
                $userActiveModel->rollback();
                $this->error_return(-1);
            }

            $userActiveModel->commit();

            $user_active = $this->getUserActiveRedis($uid);

            if($props_id == 300001)
            {
                $props_num = $user_active['broadcast_num'];
            }

            $this->success_return(['gold' => $user_active['gold'], 'diamond'=> $user_active['diamond'], 'props_num' => $props_num, 'props_id' => $props_id],'购买成功');
        }
    }
}