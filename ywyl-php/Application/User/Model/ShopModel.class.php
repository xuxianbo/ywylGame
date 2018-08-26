<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:43
 */

namespace User\Model;


use Think\Exception;

class ShopModel extends BaseModel
{
    //根据商城id获取商城信息
    public function getInfoByShopId($shop_id){
        $sql = "select goods_id from shop where shop_id = $shop_id and status = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $res;
    }
}