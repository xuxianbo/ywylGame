#!/bin/bash
rm /usr/local/src/hqyl_new/fin.log
netstat -nat | grep SYN_RECV > /usr/local/src/hqyl_new/fin.log
while read myline
do
 str=${myline%:*}
	#echo $str
 str1=${str##*\ }
	#echo $str1
 aa="0"
 bb="0"
 while read myip
 do
 if [ ${str1:0:8} = ${myip:0:8} ];then
    aa="1"
    break;
 fi
 done < /usr/local/src/hqyl_new/ip.white
 if [ $aa = $bb ];then
 str2="/usr/sbin/iptables -I INPUT -s "${str1}" -j DROP"
 echo $str2
 result=`$str2`
 echo $result
 fi
done < /usr/local/src/hqyl_new/fin.log
#done < /usr/local/src/hqyl_new/ip.black
