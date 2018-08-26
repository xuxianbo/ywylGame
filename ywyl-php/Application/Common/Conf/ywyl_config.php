<?php
return array(
    //'配置项'=>'配置值'

    /*第三方配置*/

    //短信验证码配置
    'SMS_CODE'   =>   array(
        'EXPIRATION_TIME'   =>   120,    //验证码过期时间 单位秒
        'CODE_IP_DAY_NUM'   =>   3,   //同一个ip的短信发送次数限制
        'CODE_RECODE_TIME'   =>   120,   //限制时间段  单位秒
    ),

    //阿里云短信服务接口配置
    'ALIYUN_SMS'   =>   array(
        'product' => 'Dysmsapi',
        'domain' => 'dysmsapi.aliyuncs.com',
        'accessKeyId' => 'LTAIwJJRF9rBWCAp',   //账号键值id
        'accessKeySecret' => 'kOt8y95P64jPoYHAsAksn9ch0hhnoX',   //账号密钥
        'region' => 'cn-hangzhou',
        'endPointName' => 'cn-hangzhou',
        'signName' => '百赢科技',   //短信签名
        'templateCode' => 'SMS_113445363',   //模版code
    ),

    //新微信app应用微信登录配置
    'new_app_wechar_config' => [
        [
            'appid' => 'wx6b6a71ab5a20cfc0',   //应用唯一标识，在微信开放平台提交应用审核通过后获得
            'secret' => '484900d17fd61dfcfa517dbc152d4026',   //应用密钥AppSecret，在微信开放平台提交应用审核通过后获得
            'url_1' => 'https://api.weixin.qq.com/sns/oauth2/access_token?', //通过code获取access_token   https://api.weixin.qq.com/sns/oauth2/access_token?appid=APPID&secret=SECRET&code=CODE&grant_type=authorization_code
            'url_2' => 'https://api.weixin.qq.com/sns/oauth2/refresh_token?',  //刷新或续期access_token使用   https://api.weixin.qq.com/sns/oauth2/refresh_token?appid=APPID&grant_type=refresh_token&refresh_token=REFRESH_TOKEN
            'url_3' => 'https://api.weixin.qq.com/sns/auth?', //验证检验授权凭证（access_token）是否有效   https://api.weixin.qq.com/sns/auth?access_token=ACCESS_TOKEN&openid=OPENID
            'url_4' => 'https://api.weixin.qq.com/sns/userinfo?',   //获取用户个人信息（UnionID机制) https://api.weixin.qq.com/sns/userinfo?access_token=ACCESS_TOKEN&openid=OPENID
        ],  //易玩娱乐
    ],

    //--- 支付配置开始 ---

    //支付配置
    'PAY_CONFIG' => [
        'wecharPayProcess' => 1, //微信支付流程代码
        'aliPayProcess' => 3, //支付宝支付流程代码 1为支付宝原生支付(暂时没有) 2为bpayment 3为Yxb
    ],

    //苹果支付配置
    'Apple_Pay' =>array(
        'url' => 'https://buy.itunes.apple.com/verifyReceipt',//正式地址
        'ceshi_url' => 'https://sandbox.itunes.apple.com/verifyReceipt',//测试地址
    ),

    //微信支付配置
    'wechar_pay_config' => [
        0 => [
            'appid' => 'wx6b6a71ab5a20cfc0', //应用ID 微信开放平台审核通过的应用APPID   必填
            'mch_id' => '1508429461', //商户号 微信支付分配的商户号   必填
            'key' => 'fc9b02ab2967cceec0c363813899c6f4', //支付密钥   必填
        ],
        'nonce_str' => '', //随机字符串 不长于32位。推荐随机数生成算法   必填
        'sign' => '', //签名 详见微信支付文档的签名生成算法   必填
        'body' => '', //商品描述 商品描述交易字段格式根据不同的应用场景按照以下格式：APP——需传入应用市场上的APP名字-实际商品名称，天天爱消除-游戏充值。   必填
        'out_trade_no' => '', //商户订单号 商户系统内部订单号，要求32个字符内，只能是数字、大小写字母_-|*且在同一个商户号下唯一。详见商户订单号   必填
        'total_fee' => '', //总金额 订单总金额，单位为分，详见支付金额   必填
        'spbill_create_ip' => '', //终端IP 用户端实际ip   必填
        'notify_url' => 'http://api.88008088.com/index.php/user/pay/wecharNotify', //	接收微信支付异步通知回调地址，通知url必须为直接可访问的url，不能携带参数。   必填
        'trade_type' => 'APP', //	支付类型   必填
    ],

    //BpayMent配置
    'bpayment_config' => [
        'app_id' => 3831,
        'app_key' => '6a6b435c528e4a8db470c2bb7969b6bd',
        'notify_url' => 'http://api.88008088.com/index.php/user/pay/bpayNotify',
        'return_url' => 'http://api.88008088.com/index.php/user/pay/bpayMentReturn',
    ],

    //Yxb支付配置
    'YxbConfig' => [
        'pay_memberid' => '10297',   //商户ID
        'pay_orderid' => '',   //订单号
        'pay_amount' => '',    //交易金额
        'pay_applydate' => '',  //订单时间
        'pay_bankcode' => 'ALIPAY',  //银行编码
        'pay_notifyurl' => 'http://api.88008088.com/index.php/user/pay/YxbPayNotify', //服务端返回地址
        'pay_callbackurl' => 'http://api.88008088.com/index.php/user/pay/YxbPayReturn',   //页面跳转返回地址
        'tongdao' => 'AlipayWap',  //支付通道
        'Md5key' => 'rEtAvkFufq2symWScEml6cnZ5ySldx', //密钥
        'tjurl' => 'http://www.yxbpay.com/Pay_Index',  //提交地址
        'payurl' => 'http://testapi.18168168.com/index.php/user/pay/YxbPayPage',  //支付地址
    ],

    //--- 支付配置结束 ---

    //用户信息默认配置
    'USER_DEFAULT' => array(
        'DEFAULT_GOLD' => 50000,  //默认铜币数
        'DEFAULT_MONEY' => 0,  //默认金币数
        'DEFAULT_REFFLE_NUM' => 1,  //默认抽奖次数
        'DEFAULT_BUY_NUM' => 100,  //默认每日可消费额度
        'REGULAR_PHONE'=>'/^[1-2][0-9]{10}$/',   //用户手机号正则验证
        'REGULAR_PASSWD'=>'/[\w\d]{8,20}/',   //用户密码正则验证
    ),

    /*app_id对应配置*/
    'app_id' => [
        1 => [
            'pay_type'   =>  [
                1,2,
            ],  //支持的支付方式  0现金 1微信 2支付宝 3苹果
            'is_ceshi'   =>   true, //是否开启测试支付
        ],
        1000 => [
            'pay_type'   =>  [
                1,2,
            ],  //支持的支付方式  0现金 1微信 2支付宝 3苹果
            'is_ceshi'   =>   true, //是否开启测试支付
        ],
        1003 => [
            'pay_type'   =>  [
                1,2,
            ],  //支持的支付方式  0现金 1微信 2支付宝 3苹果
            'is_ceshi'   =>   true, //是否开启测试支付
        ],

        //0到999 是苹果
        //1000到1999 是安卓
        //100000到199999 是测试

    ],

    /*代理后台配置*/
    "the_agent_url" => [
        "http://testagentywyl1.18168168.com/index.php/home/index/login.html",
        "http://testagentywyl.18168168.com/index.php/home/index/login.html",
        "http://testagent1ywyl.18168168.com/index.php/home/index/login.html",
        "http://testagent2ywyl.18168168.com/index.php/home/index/login.html",
    ],

    //是否开启游客注册
    'is_youke_regis' => true,

    //是否开启游客注册ip限制
    'is_youke_regis_ip_limit' => true,

    //用户登录错误最大次数
    'user_login_max_num' => 5,

    //用户登录错误限制时间
    'user_login_false_time' => 120,

    //自动方法的密钥
    'INITIA_TOKEN' => 'by_admin_token_123456',

    //榜单机器人配置
    'bot_config' => [
        'bot_min_id' => 9000, //机器人最小id
        'bot_max_id' => 9999, //机器人最大id
        'bot_bjl_gold_min' => 1200000, //机器人百家乐最小初始金币
        'bot_bjl_gold_max' => 12000000, //机器人百家乐最大初始金币
        'bot_ssc_gold_min' => 300000, //机器人时时彩最小初始金币
        'bot_ssc_gold_max' => 3000000, //机器人时时彩最大初始金币
        'get_bot_num' => 50, //每日使用的机器人数量
        'min_level_gold' => 0,  //最小机器人金币增长率  万分比
        'max_level_gold' => 300,  //最小机器人金币增长率  万分比
        'min_gold_rank_uid' => 9000,  //财富榜最小机器人id
        'max_gold_rank_uid' => 9029,  //财富榜最大机器人id
        'min_gold_rank' => -1000,  //最大机器人金币增长率  万分比
        'max_gold_rank' => 1000,  //最大机器人金币增长率  万分比
        'proportion' => 10000, //增长比例
    ],

    //游戏数据配置
    'record_num' => 30, //榜单显示数据条数

    /*game控制器配置*/
    'game_config' => [
        'user_game_record_num' => 10, //玩家游戏记录展示条数
    ],

    //玩家订单限制
    'user_order' => [
        'order_time' => 300, //限制时间多少秒内
        'order_num' => 5, //限制时间内最多存在多少条状态为0的订单
    ],

	//牛牛活动配置
    'ox_activity_start_time' => 0,
    'ox_activity_type_ratio' => [
        100, //大四喜
        200, //惊天动地
        500, //五福临门
        300, //锦上添花
        '苹果X', //十全十美
    ],
);