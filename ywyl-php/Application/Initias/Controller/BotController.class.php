<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/5/24
 * Time: 10:06
 */

namespace Initias\Controller;


use Initias\Model\InitiaRedis;
use User\Model\GameModel;
use User\Model\GameRedis;
use User\Model\UserActiveModel;

class BotController extends InitiaController
{

    private $bot_config,$bot_time_out = 86400;

    private $game_rank_time_out = 300;

    protected function _initialize()
    {
        $this->bot_config = C('bot_config');
    }

    /**
     * 初始化机器人的初始金额 并存入对应的redis里面 每天零点执行
     * @param string $token   initia密钥
     * @return bool
     */
    public function initiaBotGold($token = "")
    {
        $this->tokenVerify($token);

        //获取今天的五十个机器人
        $day = date('d');

        $userModel = new UserActiveModel();
        $bjl_bot = [];
        $ssc_bot = [];
        $a = 0;
        $start_uid = $this->bot_config['bot_min_id'] + (($day - 1) % 20) * 50;
        $bjl_max_rand = 9;
        $ssc_max_rand = 9;
        for($i = 0; $i < 30; $i++)
        {
            $bjl_bot[$i]['uid'] = $start_uid + $i;
            $res = $userModel->getInfoById($bjl_bot[$i]['uid']);
            if($res === false)
            {
                $this->ajaxReturn(false);
            }
            else
            {
                $bjl_bot[$i]['name'] = $res['name'];
                $bjl_bot[$i]['pic_head'] = $res['pic_head'];
            }

            $ssc_bot[$i]['uid'] = $start_uid + $i + 20;
            $res = $userModel->getInfoById($ssc_bot[$i]['uid']);
            if($res === false)
            {
                $this->ajaxReturn(false);
            }
            else
            {
                $ssc_bot[$i]['name'] = $res['name'];
                $ssc_bot[$i]['pic_head'] = $res['pic_head'];
            }

            $bjl_gold_tmp = "";
            $ssc_gold_tmp = "";

            for($j = 0; $j < 6; $j++)
            {
                $bjl_gold_tmp .= rand(0,9);
            }
            for($j = 0; $j < 5; $j++)
            {
                $ssc_gold_tmp .= rand(0,9);
            }

            if($a > 0)
            {
                $bjl_max_rand = $bjl_max_rand - rand(1,2);
                if($bjl_max_rand < 2)
                {
                    $bjl_max_rand = 9 + $bjl_max_rand;
                }

                $ssc_max_rand = $ssc_max_rand - rand(1,2);
                if($ssc_max_rand < 3)
                {
                    $ssc_max_rand = 9 + $ssc_max_rand;
                }
            }

            if($a < 5)
            {
                $bjl_gold = '1'.$bjl_max_rand.$bjl_gold_tmp;
                $ssc_gold = '3'.$ssc_max_rand.$ssc_gold_tmp;
            }
            else if($a < 25)
            {
                $bjl_gold = $bjl_max_rand.$bjl_gold_tmp;
                $ssc_gold = $ssc_max_rand.$ssc_gold_tmp;
            }
            else
            {
                if($bjl_gold_tmp[0] == 0)
                {
                    $bjl_gold_tmp[0] = 1;
                }
                $bjl_gold = ''.$bjl_gold_tmp;

                if($ssc_gold_tmp[0] == 0)
                {
                    $ssc_gold_tmp[0] = 1;
                }
                $ssc_gold = ''.$ssc_gold_tmp;
            }
            $bjl_bot[$i]['gold'] = $bjl_gold;
            $ssc_bot[$i]['gold'] = $ssc_gold;
            $a++;
        }

        $initiasRedis = new InitiaRedis();

        $res = $initiasRedis->setInitiaRedis(0,$bjl_bot,$this->bot_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $res = $initiasRedis->setInitiaRedis(1,$ssc_bot,$this->bot_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $this->ajaxReturn(true);
    }

    /**
     * 初始化百家乐赢金榜机器人初始金额 五分钟更新一次
     * @param string $token
     */
    public function initiaBjlRank($token = "")
    {
        $this->tokenVerify($token);

        $gameRedis = new GameRedis();
        $initiasRedis = new InitiaRedis();
        $gameModel = new GameModel();

        //获取百家乐赢金榜
        $rank_info = $gameModel->getBjlWinGoldRank();
        if($rank_info === false)
        {
            $this->ajaxReturn(false);
        }
        else if(empty($rank_info))
        {
            $rank_info = [];
        }

        //获取百家乐机器人数组
        $bot_info = $initiasRedis->getInitiaRedis(0);

        //获取五分钟内的百家乐赢金数据
        $day = date('d');
        $start_uid = $this->bot_config['bot_min_id'] + (($day - 1) % 20) * 50;
        $time = time();
        $bjl_aaa_info = $gameModel->getBjlInfoByTimeUid($time-300,$time,$start_uid,$start_uid+30);
        if($bjl_aaa_info === false)
        {
            $this->ajaxReturn(false);
        }
        $tmp_arr = [];
        if(!empty($bjl_aaa_info))
        {
            foreach ($bjl_aaa_info as $k=>$v)
            {
                $tmp_arr[$v['uid']] = $v['gold_win'];
            }
        }

        //加百家乐机器人数组写入百家乐赢金榜中并更新百家乐机器人数据
        $i = count($rank_info);
        foreach ($bot_info as $k=>$v)
        {
            $rank_info[$i]['uid'] = $v['uid'];
            if(isset($tmp_arr[$v['uid']]))
            {
                $v['gold'] = $v['gold'] + $tmp_arr[$v['uid']];
                $bot_info[$k]['gold'] = $v['gold'];
            }
            $rank_info[$i]['gold_win'] = $v['gold'];
            $rank_info[$i]['name'] = $v['name'];
            $rank_info[$i]['pic_head'] = $v['pic_head'];

            $bot_info[$k]['gold'] = floor($v['gold'] + $v['gold'] * (rand($this->bot_config['min_level_gold'],$this->bot_config['max_level_gold']) / $this->bot_config['proportion']));
            $i++;
        }

        //赢金榜排序
        $rank_info = $this->arraySequence($rank_info,'gold_win');

        //删除多余的数据
        for ($i = 30; $i < 60; $i ++)
        {
            unset($rank_info[$i]);
        }

        //讲数据写入redis
        $res = $initiasRedis->setInitiaRedis(0,$bot_info,$this->bot_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $res = $gameRedis->setRedis(1,$rank_info,1,$this->game_rank_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $this->ajaxReturn(true);
    }

    /**
     * 初始化时时彩赢金榜机器人初始金额 五分钟更新一次
     * @param string $token
     */
    public function initiaSscRank($token = "")
    {
        $this->tokenVerify($token);

        $gameRedis = new GameRedis();
        $initiasRedis = new InitiaRedis();
        $gameModel = new GameModel();

        //获取时时彩赢金榜
        $rank_info = $gameModel->getSscWinGoldRank();
        if($rank_info === false)
        {
            return $this->error_return(-1);
        }
        else if(empty($rank_info))
        {
            $rank_info = [];
        }

        //获取时时彩机器人数组
        $bot_info = $initiasRedis->getInitiaRedis(1);

        //获取五分钟内的百家乐赢金数据
        $day = date('d');
        $start_uid = $this->bot_config['bot_min_id'] + (($day - 1) % 20) * 50;
        $time = time();
        $bjl_aaa_info = $gameModel->getSscInfoByTimeUid($time-300,$time,$start_uid+20,$start_uid+50);
        if($bjl_aaa_info === false)
        {
            $this->ajaxReturn(false);
        }
        $tmp_arr = [];
        if(!empty($bjl_aaa_info))
        {
            foreach ($bjl_aaa_info as $k=>$v)
            {
                $tmp_arr[$v['uid']] = $v['gold_win'];
            }
        }

        //添加时时彩机器人数组写入百家乐赢金榜中并更新百家乐机器人数据
        $i = count($rank_info);
        foreach ($bot_info as $k=>$v)
        {
            $rank_info[$i]['uid'] = $v['uid'];
            if(isset($tmp_arr[$v['uid']]))
            {
                $v['gold'] = $v['gold'] + $tmp_arr[$v['uid']];
                $bot_info[$k]['gold'] = $v['gold'];
            }
            $rank_info[$i]['gold_win'] = $v['gold'];
            $rank_info[$i]['name'] = $v['name'];
            $rank_info[$i]['pic_head'] = $v['pic_head'];

            $bot_info[$k]['gold'] = floor($v['gold'] + $v['gold'] * (rand($this->bot_config['min_level_gold'],$this->bot_config['max_level_gold']) / $this->bot_config['proportion']));
            $i++;
        }

        //赢金榜排序
        $rank_info = $this->arraySequence($rank_info,'gold_win');

        //删除多余的数据
        for ($i = 30; $i < 60; $i ++)
        {
            unset($rank_info[$i]);
        }

        //讲数据写入redis
        $res = $initiasRedis->setInitiaRedis(1,$bot_info,$this->bot_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $res = $gameRedis->setRedis(1,$rank_info,2,$this->game_rank_time_out);
        if($res === false)
        {
            $this->ajaxReturn(false);
        }

        $this->ajaxReturn(true);
    }
}