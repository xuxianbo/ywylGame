<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:37
 */

namespace User\Model;


use Think\Exception;

class SscConfigModel extends BaseModel
{
    //获取时时彩开启状态
    public function getIsSsc(){
        $sql = "select is_start from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return false;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取开出豹子所需局数
    public function getBaoZiJu(){
        $sql = "select baozi_k from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取新的开豹子所需局数并写入配置
    public function setBaoZiJu(){
        $sql = "select baozi_k_x,baozi_k_y from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }

        $baozi_k = rand($res[0]['baozi_k_x'],$res[0]['baozi_k_y']);
        $sql = "update config_ssc set baozi_k = $baozi_k where id = 1";
        try{
            $res = $this->execute($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }

        return $baozi_k;
    }

    //获取随机三条A的概率
    public function getAaaPro(){
        $sql = "select aaa_pro from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取牌型赔率
    public function getRatio(){
        $sql = "select aaa_ratio,aaa_proportion,baozi_ratio,baozi_proportion,shunjin_ratio,jinhua_ratio,shunzi_ratio,duizi_ratio,danpai_ratio from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取抽水比例和返池比例
    public function getWater(){
        $sql = "select water_proportion,water_fh_proportion from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取奖金与下注总额比上限
    public function getAwareBetgoldPro(){
        $sql = "select aware_betgold_pro from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }

    //获取彩金池上限
    public function getPoolMax(){
        $sql = "select pool_max from config_ssc where id = 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        return $res;
    }
}