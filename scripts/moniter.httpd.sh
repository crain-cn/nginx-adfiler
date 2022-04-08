#!/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/local/go/bin:/data/module/go/bin

IP=`/sbin/ifconfig  eth1| grep inet|awk -F: '{print $2}'| awk '{print $1}'`
ps -ef|grep nginx | grep master | grep -v grep
if [ $? -ne 0 ]; then
  echo "HOST $IP nginx service failed" | mail -s "nginx service failed" mayishan@addnewer.com zhangshenwang@addnewer.com
  service nginx restart
  sleep 2
  ps -ef|grep nginx | grep -v grep
  if [ $? -eq 0 ]; then
        echo "HOST $IP nginx service recovered" | mail -s "nginx service recovered" mayishan@addnewer.com zhangshenwang@addnewer.com
  fi
fi

Maintainer="zhangshenwang@addnewer.com"
IP=`ifconfig eth1 | grep "inet addr" | awk -F: '{print $2}' | awk '{print $1}'`
#if [ -z `ss -tnl | grep 10001` ]; then
#  sleep 1
#  if [ -z `ss -tnl | grep 10001` ]; then
#       echo "HOST $IP httpd service error" |mail -s "httpd service error" $Maintainer
#  fi
#fi
STATUS=`/usr/bin/curl --connect-timeout 1 -m 2 http://127.0.0.1:10001/`
if [[ $STATUS != "\"hello reachmax\"" ]]; then
    sleep 2
    STATUS=`/usr/bin/curl --connect-timeout 1 -m 5 http://127.0.0.1:10001/`
    if [[ $STATUS != "\"hello reachmax\"" ]]; then
        echo "HOST $IP http status code is not hello reachmax" | mail -s "http status code error" $Maintainer
        service supervisor restart
    fi
fi