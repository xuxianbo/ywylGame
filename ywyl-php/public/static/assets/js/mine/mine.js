// JavaScript Document

//跳转到指定url
function get_href(url)
{
	window.location.href = url;
}

//判断是否null
function isNull(data)
{
	return (data == "" || data == undefined || data == null) ? true : false; 
}

//获取数据
function get_data(url,data,status)
{
    var res = "";
    var type = "post";

    if(isNull(url))
    {
        url = "";
        res = 0;
        return res;
    }

    if(isNull(data))
    {
        type = "get";
    }

    if(isNull(status))
    {
        status = false;
    }

    $.ajax({
        type:type,
        url:url,
        data:data,
        async:status,
        success:function(msg)
        {
            res = msg;
            if(res.code == 119)
            {
                window.location.href = res.data.url;
            }
        }
    });
    return res;
}

//获取字符串长度 中文字符算两个字符
function get_str_lenth(str)
{
    var l = str.length;
    var blen = 0;
    for(i=0; i<l; i++) {
        if ((str.charCodeAt(i) & 0xff00) != 0)
        {
            blen ++;
        }
        blen ++;
    }
    return blen;
}

//获取到时间戳
function get_unix(e) {
    var date = new Date(e);
    var time = Date.parse(date);
    return time / 1000;
}

//获取到时间
function get_time(e,format) {
    var date = new Date(e * 1000);//时间戳为10位需*1000，时间戳为13位的话不需乘1000
    var Y = date.getFullYear();
    var M = (date.getMonth()+1 < 10 ? '0'+(date.getMonth()+1) : date.getMonth()+1);
    var D = date.getDate();
    var h = date.getHours();
    var m = date.getMinutes();
    var s = date.getSeconds();

    if(isNull(format))
    {
        format = 'Y-m-d H:i:s';
    }

    format = format.replace("Y", Y);
    format = format.replace("m", M);
    format = format.replace("d", D);
    format = format.replace("H", h);
    format = format.replace("i", m);
    format = format.replace("s", s);

    return format;
}