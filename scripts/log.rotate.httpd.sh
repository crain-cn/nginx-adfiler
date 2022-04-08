#!/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/go/bin:/data/module/go/bin
##日志切割
oldlog_path="/data2/old_logs"
nginx_log_path="/data1/logs"
Date="`date +%F-%H`"

cd $nginx_log_path
for log_name in `ls *.log`
do
mv $log_name  ${Date}-$log_name
done

service nginx reload

sleep 200
for log_name in `ls ${Date}*.log`
do
mv $log_name  $oldlog_path/nginx/$log_name
done