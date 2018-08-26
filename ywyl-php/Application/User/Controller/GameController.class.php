<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2018/3/22
 * Time: 11:58
 */

namespace User\Controller;


use User\Model\GameModel;
use User\Model\UserActiveModel;

class GameController extends BaseController
{
    /**
     * 获取游戏房间配置
     * @param string $user_token
     * @param string $uid
     * @param string $game_id
     */
    public function getGameHoustConfig($user_token = "", $uid = "", $game_id = "")
    {
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        if(empty($game_id) || !is_numeric($game_id))
        {
            $this->error_return(-3001002);
        }

        $gameModel = new GameModel();
        $house_info = $gameModel->getGameHouseConfig($game_id);
        if($house_info === false)
        {
            $this->error_return(-1);
        }
        else
        {
            $max_type = end($house_info)['type'];
            $this->success_return(['house_config' => $house_info,'max_type' => $max_type]);
        }
    }

    /**
     * 获取玩家游戏记录
     * @param string $user_token
     * @param string $uid
     * @param string $game_id
     * @param string $game_type
     * @param string $page
     */
    public function getUserGameRecord($user_token = "", $uid = "", $game_id = "", $game_type = "", $start_id = "", $page_num = "")
    {
        //给与的数量条数
        if(empty($page_num) || $page_num > C('game_config.user_game_record_num'))
        {
            $page_num = C('game_config.user_game_record_num');
        }
        if(!empty($start_id) && !is_numeric($start_id))
        {
            $this->error_return(-3001006);
        }

        //处理传递数据
        $user_max_num = 0;
        switch ($game_id)
        {
            case 1:
                $user_max_num = 7;
                break;
            case 2:
                $user_max_num = 7;
                break;
            case 5:
                $user_max_num = 2;
                break;
            case 6:
                $user_max_num = 4;
                break;
            default:
                $this->error_return(-3001002);
                break;
        }
        if($game_type == "" || $game_type < 1)
        {
            $game_type = -1;
        }

        //验证用户token
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $gameModel = new GameModel();

        //获取玩家游戏记录
        $info = $gameModel->getUserGameRecordByUidGid($uid,$game_id,$game_type,$start_id,$page_num);
        if($info === false)
        {
            $this->error_return(-1);
        }

        $this->success_return(['user_game_record' => $info, 'game_id' => $game_id, 'game_type' => $game_type, 'user_max_num' => $user_max_num]);
    }

    /**
     * 获取玩家百人场游戏记录
     * @param string $user_token
     * @param string $uid
     * @param string $game_id
     * @param string $game_type
     * @param string $page
     */
    public function getUserBjlGameRecord($user_token = "", $uid = "", $game_id = "", $game_type = "", $start_id = "", $page_num = "")
    {
        //给与的数量条数
        if(empty($page_num) || $page_num > C('game_config.user_game_record_num'))
        {
            $page_num = C('game_config.user_game_record_num');
        }
        if(!empty($start_id) && !is_numeric($start_id))
        {
            $this->error_return(-3001006);
        }

        //处理传递数据
        switch ($game_id)
        {
            case 0:
                $game_id = 0;
                break;
            case 4:
                $game_id = 4;
                break;
            default:
                $this->error_return(-3001002);
                break;
        }
        if($game_type == "" || $game_type < 1)
        {
            $game_type = -1;
        }

        //验证用户token
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $gameModel = new GameModel();

        $info = [];

        //获取玩家
        $qh = $gameModel->getUserGameRecordByUidGid($uid,$game_id,$game_type,$start_id,$page_num);
        if($qh === false)
        {
            $this->error_return(-1);
        }
        else if(!empty($qh))
        {
            $i = 0;
            $userModel = new UserActiveModel();

            //遍历期号获取到需要的游戏记录数据
            foreach ($qh as $k => $v)
            {
                //获取玩家的游戏信息
                $user_record = $gameModel->getUserBjlInfo($v['qh'],$uid,$game_id);
                if($user_record === false)
                {
                    $this->error_return(-1);
                }
                else if(!empty($user_record[0]['uid']))
                {
                    $user_record[0]['id'] = $v['id'];
                    $info[$i][] = $user_record[0];

                    //庄家信息和前5名信息
                    $res = $gameModel->getBjlInfoByQh($v['qh'],$game_id);
                    if($res === false)
                    {
                        $this->error_return(-1);
                    }
                    else if(!empty($res))
                    {
                        $j = 1;
                        $res = $res[0];
                        //庄家
                        $info[$i][$j]['uid'] = $res['banker'];
                        $info[$i][$j]['name'] = $res['banker_name'];
                        $info[$i][$j]['gain'] = $res['banker_gain'];
                        $info[$i][$j]['create_time'] = $info[$i][0]['create_time'];
                        $info[$i][$j]['id'] = $v['id'];

                        //前5名
                        while ($j < 6)
                        {
                            $info[$i][$j+1]['uid'] = $res['uid'.$j];
                            $info[$i][$j+1]['name'] = $res['name'.$j];
                            $info[$i][$j+1]['gain'] = $res['gain'.$j];
                            $info[$i][$j+1]['create_time'] = $info[$i][0]['create_time'];
                            $info[$i][$j+1]['id'] = $v['id'];
                            $j++;
                        }


                    }
                }
                $i++;
            }
        }

        $this->success_return(['user_game_record' => $info, 'game_id' => $game_id, 'game_type' => $game_type]);
    }

    /* 牛牛牌型活动开始 */

    /**
     * 获取玩家在牛牛中获得彩金的情况
     * @param string $user_token
     * @param string $uid
     * 牌型对应 11五花 12顺子 13同花 14葫芦 15同花顺 16炸弹 17五小
     */
    public function getActivityOxUserPool($user_token = "", $uid = "")
    {
        //验证用户token
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $start_time = C('ox_activity_start_time');

        $gameModel = new GameModel();
        $check_type_pool = [
            ['check_type' => 11,'start_time' => ''],
            ['check_type' => 12,'start_time' => ''],
            ['check_type' => 13,'start_time' => ''],
            ['check_type' => 14,'start_time' => ''],
            ['check_type' => 15,'start_time' => ''],
            ['check_type' => 16,'start_time' => ''],
            ['check_type' => 17,'start_time' => ''],
        ];

        foreach ($check_type_pool as $k => $v)
        {
            //获取用户兑换情况
            $res = $gameModel->getOxActivityTime($uid,$v['check_type']);
            if($res === false)
            {
                $this->error_return(-1);
            }
            else if(empty($res))
            {
                $check_type_pool[$k]['start_time'] = $start_time;
            }
            else
            {
                $check_type_pool[$k]['start_time'] = $res[0]['create_time'];
            }

            //获取用户牌型情况 11,12,13,14,15,16,17
            $res = $gameModel->getOxActivityBottom($uid,$v['check_type'],$check_type_pool[$k]['start_time']);
            if($res === false)
            {
                $this->error_return(-1);
            }
            else if(empty($res))
            {
                $check_type_pool[$k]['data'] = [];
            }
            else
            {
                $check_type_pool[$k]['data'] = $res[0];
            }
        }

        $this->success_return(['check_type_pool' => $check_type_pool]);
    }

    /**
     * 玩家牛牛活动领取奖励
     * @param string $user_token
     * @param string $uid
     * 牌型对应 11五花 12顺子 13同花 14葫芦 15同花顺 16炸弹 17五小
     */
    public function getActivityOxDraw($user_token = "", $uid = "")
    {
        //验证用户token
        $user_base = $this->getUserBaseByToken($user_token);
        if($uid != $user_base['uid'] || $uid == "")
        {
            $this->error_return(-150);
        }

        $start_time = C('ox_activity_start_time');

        $gameModel = new GameModel();

        $check_type_pool = [
            ['check_type' => 11,'start_time' => ''],
            ['check_type' => 12,'start_time' => ''],
            ['check_type' => 13,'start_time' => ''],
            ['check_type' => 14,'start_time' => ''],
            ['check_type' => 15,'start_time' => ''],
            ['check_type' => 16,'start_time' => ''],
            ['check_type' => 17,'start_time' => ''],
        ];

        foreach ($check_type_pool as $k => $v)
        {
            //获取用户兑换情况
            $res = $gameModel->getOxActivityTime($uid,$v['check_type']);
            if($res === false)
            {
                $this->error_return(-1);
            }
            else if(empty($res))
            {
                $check_type_pool[$k]['start_time'] = $start_time;
            }
            else
            {
                $check_type_pool[$k]['start_time'] = $res[0]['create_time'];
            }

            //获取用户牌型情况 11,12,13,14,15,16,17
            $res = $gameModel->getOxActivityBottom($uid,$v['check_type'],$check_type_pool[$k]['start_time']);
            if($res === false)
            {
                $this->error_return(-1);
            }
            else if(empty($res))
            {
                $check_type_pool[$k]['data'] = [];
            }
            else
            {
                $check_type_pool[$k]['data'] = $res[0];
            }
        }

        $gold = 0;
        $max_bottom = 0;
        $time = time();

        $gameModel->startTrans();

        $is_shi = 1;
        //判断是否可以领取
        if($check_type_pool[0]['data'] && $check_type_pool[1]['data'] && $check_type_pool[2]['data'] && $check_type_pool[3]['data'])
        {
            for($i = 0;$i < 4; ++$i)
            {
                $res = $gameModel->addOxActivityRecord($uid,$check_type_pool[$i]['data']['id'],$check_type_pool[$i]['check_type'],$time);
                if($res === false)
                {
                    $gameModel->rollback();
                    $this->error_return(-1);
                }

                if($check_type_pool[$i]['data']['bottom'] < 50)
                {
                    $is_shi = 0;
                }

                if($max_bottom == 0)
                {
                    $max_bottom = $check_type_pool[$i]['data']['bottom'];
                }
                else if($max_bottom > $check_type_pool[$i]['data']['bottom'])
                {
                    $max_bottom = $check_type_pool[$i]['data']['bottom'];
                }



                $check_type_pool[$i]['data'] = [];
            }

            if($is_shi && $check_type_pool[4]['data'] && $check_type_pool[5]['data'] && $check_type_pool[6]['data'] && $check_type_pool[4]['data']['bottom'] > 50 && $check_type_pool[5]['data']['bottom'] > 50 && $check_type_pool[6]['data']['bottom'] > 50)
            {
                //十全十美
                //添加领取记录
                for($i = 4;$i < 7; ++$i)
                {
                    $res = $gameModel->addOxActivityRecord($uid,$check_type_pool[$i]['data']['id'],$check_type_pool[$i]['check_type'],$time);
                    if($res === false)
                    {
                        $gameModel->rollback();
                        $this->error_return(-1);
                    }

                    if($max_bottom < $check_type_pool[$i]['data']['bottom'])
                    {
                        $max_bottom = $check_type_pool[$i]['data']['bottom'];
                    }

                    $check_type_pool[$i]['data'] = [];
                }
            }
            else
            {
                $ox_activity_type_ratio = C('ox_activity_type_ratio');
                if($check_type_pool[6]['data']) //五福临门
                {
                    $res = $gameModel->addOxActivityRecord($uid,$check_type_pool[6]['data']['id'],$check_type_pool[6]['check_type'],$time);
                    if($res === false)
                    {
                        $gameModel->rollback();
                        $this->error_return(-1);
                    }

                    if($max_bottom > $check_type_pool[6]['data']['bottom'])
                    {
                        $max_bottom = $check_type_pool[6]['data']['bottom'];
                    }

                    $check_type_pool[6]['data'] = [];
                    $gold = $max_bottom * $ox_activity_type_ratio[2];
                }
                else if($check_type_pool[4]['data']) //锦上添花
                    {
                        $res = $gameModel->addOxActivityRecord($uid,$check_type_pool[4]['data']['id'],$check_type_pool[4]['check_type'],$time);
                        if($res === false)
                        {
                            $gameModel->rollback();
                            $this->error_return(-1);
                        }

                        if($max_bottom > $check_type_pool[4]['data']['bottom'])
                        {
                            $max_bottom = $check_type_pool[4]['data']['bottom'];
                        }
                        $check_type_pool[4]['data'] = [];

                        //计算奖金
                        $gold = $max_bottom * $ox_activity_type_ratio[3];
                    }
                else if($check_type_pool[5]['data']) //惊天动地
                    {
                        $res = $gameModel->addOxActivityRecord($uid,$check_type_pool[5]['data']['id'],$check_type_pool[5]['check_type'],$time);
                        if($res === false)
                        {
                            $gameModel->rollback();
                            $this->error_return(-1);
                        }

                        if($max_bottom > $check_type_pool[5]['data']['bottom'])
                        {
                            $max_bottom = $check_type_pool[5]['data']['bottom'];
                        }
                        $check_type_pool[5]['data'] = [];

                        $gold = $max_bottom * $ox_activity_type_ratio[1];
                    }
                else //大四喜
                {
                    $gold = $max_bottom * $ox_activity_type_ratio[0];
                }

                $res = $gameModel->addOxActivityRecordGold($uid,$gold,$time);
                if($res === false)
                {
                    $gameModel->rollback();
                    $this->error_return(-1);
                }

                //记录用户奖励
                $userActiveModel = new UserActiveModel();
                $res = $userActiveModel->updateMoneyByUid($uid,$gold);
                if($res === false)
                {
                    $gameModel->rollback();
                    $this->error_return(-1);
                }
            }

            $gameModel->commit();
            $user_active = $this->getUserActiveRedis($uid);
            $this->success_return([ 'message' => '领取成功', 'money' => $gold, 'user_money' => $user_active['money'], 'check_type_pool' => $check_type_pool ]);
        }
        else
        {
            //不可领取
            $this->error_return(-3001007);
        }
    }

    /* 牛牛牌型活动结束 */
}