#!/bin/sh
log_path="/data/mapping_logs"
rsync_path="/data/mapping_logs_rsync"
old_path="/data2/old_logs/cookie_mapping"

log_name="`date -d "-1 hour" +%Y%m%d%H`.log"
cd $log_path
if [ -f $log_name ]
then
 mv $log_path/$log_name $rsync_path
fi


PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/go/bin:/data/module/go/bin
Maintainer="zhangshenwang@addnewer.com"
IP=`ifconfig eth1 | grep "inet addr" | awk -F: '{print $2}' | awk '{print $1}'`

log_name="`date -d "-1 hour" +%Y%m%d%H`.log"
log_file=$rsync_path/$log_name
if [ ! -s $log_file ]
then
  mail -s "HOST $IP $log_file file is empty" $Maintainer
fi


old_log="`date -d "-2 day" +%Y%m%d%H`.log"
cd $rsync_path
if [ -f $old_log ]
then
 mv $rsync_path/$old_log $old_path
fi

#cd $old_path
#if [ -f $old_log ]
#then
# tar -zcvf $old_log.tar.gz $old_log
# rm -rf $old_log
#fi

#old_tar="`date -d "-5 day" +%Y%m%d%H`.log.tar.gz"
#if [ -f $old_tar ]
#then
# rm -rf $old_tar
#fi

find /data/logs/test -mtime 2 -name '*.log' -exec rm -f {} \;

find /data2/old_logs/cookie_mapping -mtime 5 -name '*.log' -exec rm -f {} \;
#find /data2/old_logs/cookie_mapping -mtime 7 -name '*.tar.gz' -exec rm -f {} \;
find /data2/old_logs/nginx -mtime 2 -name '*.log' -exec rm -f {} \;

find /data/mapping_logs_rsync/ -mtime 2 -name '*.log' -exec mv {} /data2/old_logs/cookie_mapping \;
find /data/mapping_logs/ -cmin +120 -name '*.log' -exec mv {} /data/mapping_logs_rsync \;
