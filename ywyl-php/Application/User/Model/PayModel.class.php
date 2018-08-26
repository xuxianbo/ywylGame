<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2017/12/23
 * Time: 15:11
 */

namespace User\Model;


use Think\Log;

class PayModel extends BaseModel
{
    /**
     * curlPost提交
     * @param string $url 提交的地址
     * @param array $post 需要传递的数据数组
     * @return mixed
     */
    private function curl_post($url, $post){
        $options = array(
            CURLOPT_RETURNTRANSFER =>true,
            CURLOPT_HEADER =>false,
            CURLOPT_POST =>true,
            CURLOPT_POSTFIELDS => $post,
        );
        $ch = curl_init($url);
        curl_setopt_array($ch, $options);
        $result = curl_exec($ch);
        if($result === false)
        {
            Log::write("curl链接失败,信息是:".curl_error($ch).",时间为".date("Y-m-d H:i:s"),'ERR');
        }
        curl_close($ch);
        return $result;
    }
}