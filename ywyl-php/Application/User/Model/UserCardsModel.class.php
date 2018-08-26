<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/3/22
 * Time: 14:34
 */

namespace User\Model;


use Think\Exception;

class UserCardsModel extends BaseModel
{

    /**
     * 获取玩家月卡配置信息
     * @param string $id
     * @return bool|mixed
     */
    public function getInfo($id = ""){
        if($id == ""){
            $sql = "select id as card_id,`desc`,goods_id,get_num,give_points_num,give_gold_num,give_diamond_num,pre_points_num,pre_gold_num,pre_diamond_num from config_user_cards";
        }else{
            $sql = "select id as card_id,`desc`,goods_id,get_num,give_points_num,give_gold_num,give_diamond_num,pre_points_num,pre_gold_num,pre_diamond_num from config_user_cards where id = $id";
        }
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $info;
    }

    /**
     * 根据商品id获取卡配置信息
     * @param $goods_id
     * @return bool|mixed
     */
    public function getInfoByGoodsId($goods_id){
        $sql = "select id as card_id,get_num,pre_points_num,pre_gold_num,pre_diamond_num from config_user_cards where goods_id = $goods_id";
        try{
            $info = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        return $info;
    }
}