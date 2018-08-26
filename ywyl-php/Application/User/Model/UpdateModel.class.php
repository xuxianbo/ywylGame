<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 16:23
 */

namespace User\Model;


use Think\Exception;

class UpdateModel extends BaseModel
{
    /**
     * 获取服务器版本号
     * @param $app_id
     * @return int|mixed|string
     */
    public function getInfoByStore($app_id){
        $sql = "select version_1,version_2 from config_update where app_id = $app_id order by id desc limit 1";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        if(empty($res)){
            return 0;
        }
        $res[0]['version_2'] = sprintf("%03d", $res[0]['version_2']);
        $res = $res[0]['version_1'].'.'.$res[0]['version_2'];
        return $res;
    }

    /**
     * 版本号对比
     * @param $versionA
     * @param $versionB
     * @param $app_id
     * @return int|mixed
     */
    public function versionCompare($versionA,$versionB,$app_id){
        $dm = '.';
        $verListA = explode($dm, (string)$versionA);
        $verListB = explode($dm, (string)$versionB);

        if(strlen($verListA[1]) < 3){
            $verListA[1]=str_pad($verListA[1],3,"0",STR_PAD_RIGHT );
        }
        if(strlen($verListA[1]) != 3 && $verListA[1] != 0){
            return -103;
        }

        //客户端大版本大于服务端
        if($verListA[0] > $verListB[0]){
            return -5;
        }

        //这里的代码是为了强行执行app_id为2的包的热更新
        if($app_id == 2){
            $verListA[0] = $verListB[0];
        }

        //客户端大版本等于服务端
        if($verListA[0] == $verListB[0]){
            if($verListA[1] > $verListB[1]){
                return -5;
            }
            if($verListA[1] < $verListB[1]){
                $res['type'] = 2;
                $res['data'] = $this->getVersionDownUrl($verListA,$app_id);
                return $res;
            }
            return 1;
        }

        //客户端大版本小于服务端
        if($verListA[0] < $verListB[0]){
            $res['type'] = 1;
            $res['data'] = $this->getUpdateUrl($verListB,$app_id);
            return $res;
        }
    }

    /**
     * 获取大版本更新地址
     * @param $type
     * @param $versionB  服务器当前版本号
     * @param $app_id
     * @return int
     */
    private function getUpdateUrl($versionB,$app_id){
        $updatesUrl = C('app_id');
        $downUrl[$versionB[0].'.000'] = $updatesUrl[$app_id]['apple_update'];
        return $downUrl;
    }

    /**
     * 获取版本下载地址
     * @param $versionA   客户端版本号
     * @param $app_id
     * @return int|mixed
     */
    private function getVersionDownUrl($versionA,$app_id){
        $sql = "select version_1,version_2 from config_update where app_id = $app_id and version_1 = $versionA[0] and version_2 > $versionA[1]";
        try{
            $res = $this->query($sql);
        }catch (Exception $e){
            $this->logError($e,'EMERG');
            return -1;
        }
        $downUrl = [];
        foreach ($res as $val){
            $val['version_2'] = sprintf("%03d", $val['version_2']);
            $downUrl[$val['version_1'].'.'.$val['version_2']] = C('update_config.version_url').$app_id.$val['version_1'].$val['version_2'].'.zip';
        }
        return $downUrl;
    }
}