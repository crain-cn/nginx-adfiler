#!/bin/bash
IP=`/sbin/ifconfig  eth1| grep inet|awk -F: '{print $2}'| awk '{print $1}'`
ps -ef|grep nginx | grep master | grep -v grep
if [ $? -ne 0 ]; then
  echo "HOST $IP nginx service failed" | mail -s "nginx service failed" mayishan@addnewer.com zhangshenwang@addnewer.com
  systemctl restart ngx_mapping.service
  sleep 2
  ps -ef|grep nginx | grep -v grep
  if [ $? -eq 0 ]; then
        echo "HOST $IP nginx service recovered" | mail -s "nginx service recovered" mayishan@addnewer.com zhangshenwang@addnewer.com
  fi
fi

