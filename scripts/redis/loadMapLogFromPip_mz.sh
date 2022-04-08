#!/bin/sh
log_path="/data/mapping_logs"
php_path="/data/modules/php-5.4.34/bin"
run_path="/data/modules/soft/cron"
Date="`date -d "-1 day" +%Y%m%d`"
cd $log_path

#miaozhen redis
for Hour in {00..23}
	do  
		cat $Date$Hour.log |grep 'source\":6'|$php_path/php $run_path/loadMapLogFromPip_mz.php
done
