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
    {31, "amnet"}
!

log_path="/data/mapping_logs_rsync"
log_name="`date -d "-1 hour" +%Y%m%d%H`.log"
log_file=$log_path/$log_name

if [ -f $log_file ]
thenhp
cd /data/modules/ngx_mapping/scripts/scylla
#cat $log_file | awk '/"ext":31/'  | awk '/"m":0/' | php cinlog.php -t amnet
#cat $log_file | awk '/"ext":26/'  | awk '/"m":0/' | php cinlog.php -t pptv
#cat $log_file | awk '/"ext":21/'  | awk '/"m":0/' | php cinlog.php -t letv
#cat $log_file | awk '/"ext":5/' | awk '/"m":0/' | php cinlog.php -t sohu
#cat $log_file | awk '/"ext":3/' | awk '/"m":0/' | php cinlog.php -t iqiyi -s 1
#cat $log_file | awk '/"ext":8/'  | awk '/"m":0/'  | php cinlog.php -t youku -s 1
#cat $log_file | awk '/"ext":29/' | awk '/"m":0/' | php cinlog.php -t mgtv

#cat $log_file | awk '/"ext":10/'| awk '/"m":0/' | php cinlog.php -t ku6
#cat $log_file | awk '/"ext":11/'  | awk '/"m":0/' | php cinlog.php -t fengxing
#cat $log_file | awk '/"ext":27/'  | awk '/"m":0/' | php cinlog.php -t yinyuetai 

#cat $log_file | awk '/"ext":6/'  | grep '"m":0' | php cinlog.php -t miaozhen 
#cat $log_file | awk '/"ext":2/' | grep '"f":1' | grep '"m":0'| php -t cinlog.php tencent 

fi