#!/bin/bash
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games"
IP=`ifconfig eth1 | grep "inet addr" | awk -F: '{print $2}'| awk '{print $1}'`
Maintainer="mayishan@addnewer.com zhangshenwang@addnewer.com"
/data/modules/redis-3.2.6/bin/redis-cli  ping | grep -qE '^PONG$' || (/data1/modules/redis-3.2.6/bin/redis-server /data1/modules/redis-3.2.6/etc/redis-6379.conf && echo "Host ${IP} redis service failed" | mail -s "redis service failed" $Maintainer)

