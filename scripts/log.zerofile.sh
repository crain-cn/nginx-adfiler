#!/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/go/bin:/data/module/go/bin
Maintainer="zhangshenwang@addnewer.com"
IP=`ifconfig eth1 | grep "inet addr" | awk -F: '{print $2}' | awk '{print $1}'`
DIR1=/tmp/zerofile.sh
/usr/bin/find /data/mapping_logs_rsync/ -size 0k -exec echo {} > $DIR1 \;
if [ -s $DIR1 ]; then
    cat $DIR1 | mail -s "HOST $IP /data/mapping_logs_rsync dir find zero file" $Maintainer
    cat /dev/null > $DIR1
fi
