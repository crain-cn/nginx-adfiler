:<<!
    {2, "tencent"},
    {3, "iqiyi"},
    {5, "sohu"},
    {6, "miaozhen"},
    {8, "youku"},
    {10, "ku6"},
    {11, "fengxing"},
    {21, "letv"},
    {26, "pptv"},
    {27, "yinyuetai"},
    {29, "mgtv"}
!

log_path="/data/mapping_logs_rsync"
log_name="`date -d "-1 hour" +%Y%m%d%H`.log"
log_file=$log_path/$log_name

if [ -f $log_file ]
then
cd /data/modules/ngx_mapping/scripts/redis
#cat $log_file | awk '/"ext":26/'  | awk '/"m":0/' | php loadMapLogFromPip_pptv.php
#cat $log_file | awk '/"ext":21/' | awk '/"m":0/' | php loadMapLogFromPip_letv.php

#cat $log_file | awk '/"ext":5/'  | awk '/"m":0/' | php loadMapLogFromPip_sohu.php
#cat $log_file | awk '/"ext":3/' | awk '/"m":0/' | php loadMapLogFromPip_iqiyi.php
#cat $log_file | awk '/"ext":8/' | awk '/"m":0/' | php loadMapLogFromPip_youku.php

#cat $log_file | awk '/"ext":2/'  | awk '/"m":0/' | php loadMapLogFromPip_tencent.php
cat $log_file | awk '/"ext":6/' | awk '/"m":0/' | php loadMapLogFromPip_mz.php

#cat $log_file | awk '/"ext":3/' | awk '/"m":0/' |php loadMapLogFromPip_iqiyidmp.php
#cat $log_file | awk '/"ext":8/' | awk '/"m":0/' | php loadMapLogFromPip_youkudmp.php
#cat $log_file | awk '/"ext":27/' | awk '/"m":0/' | php loadMapLogFromPip_yinyt.php
#cat $log_file | awk '/"ext":10/'| awk '/"m":0/' | php loadMapLogFromPip_ku6.php
#cat $log_file | awk '/"ext":11/'  | awk '/"m":0/' | php loadMapLogFromPip_fengxing.php

fi
