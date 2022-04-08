<?php
define('PACK_NUM', 200); //redis一次提交的数量

//predis配置
$redisConfig = array('tcp://10.51.120.96:6379',
                    'tcp://10.51.120.96:6380',
                    'tcp://10.51.120.96:6381',
                    'tcp://10.51.120.96:6382',
                    'tcp://10.51.120.96:6383',
                    'tcp://10.51.120.96:6384',
                    'tcp://10.51.120.96:6385',
                    'tcp://10.51.120.96:6386',
                    'tcp://10.51.120.96:6387',
                    'tcp://10.51.120.96:6388',
                    'tcp://10.51.120.86:6379',
                    'tcp://10.51.120.86:6380',
                    'tcp://10.51.120.86:6381',
                    'tcp://10.51.120.86:6382',
                    'tcp://10.51.120.86:6383',
                    'tcp://10.51.120.86:6384',
                    'tcp://10.51.120.86:6385',
                    'tcp://10.51.120.86:6386',
                    'tcp://10.51.120.86:6387',
                    'tcp://10.51.120.86:6388',
                    'tcp://10.51.120.72:6379',
                    'tcp://10.51.120.72:6380',
                    'tcp://10.51.120.72:6381',
                    'tcp://10.51.120.72:6382',
                    'tcp://10.51.120.72:6383',
                    'tcp://10.51.120.72:6384',
                    'tcp://10.51.120.72:6385',
                    'tcp://10.51.120.72:6386',
                    'tcp://10.51.120.72:6387',
                    'tcp://10.51.120.72:6388',
                    'tcp://10.51.120.93:6379',
                    'tcp://10.51.120.93:6380',
                    'tcp://10.51.120.93:6381',
                    'tcp://10.51.120.93:6382',
                    'tcp://10.51.120.93:6383',
                    'tcp://10.51.120.93:6384',
                    'tcp://10.51.120.93:6385',
                    'tcp://10.51.120.93:6386',
                    'tcp://10.51.120.93:6387',
                    'tcp://10.51.120.93:6388',
    				'tcp://10.51.119.53:6379',
					'tcp://10.51.119.53:6380',
					'tcp://10.51.119.53:6381',
					'tcp://10.51.119.53:6382',
                    'tcp://10.51.119.53:6383',
                    'tcp://10.51.119.53:6384',
                    'tcp://10.51.119.53:6385',
                    'tcp://10.51.119.53:6386',
                    'tcp://10.51.119.53:6387',
                    'tcp://10.51.119.53:6388',
                    'tcp://10.51.120.44:6379',
                    'tcp://10.51.120.44:6380',
                    'tcp://10.51.120.44:6381',
                    'tcp://10.51.120.44:6382',
                    'tcp://10.51.120.44:6383',
                    'tcp://10.51.120.44:6384',
                    'tcp://10.51.120.44:6385',
                    'tcp://10.51.120.44:6386',
                    'tcp://10.51.120.44:6387',
                    'tcp://10.51.120.44:6388',
                    'tcp://10.51.69.51:6379',
                    'tcp://10.51.69.51:6380',
                    'tcp://10.51.69.51:6381',
                    'tcp://10.51.69.51:6382',
                    'tcp://10.51.69.51:6383',
                    'tcp://10.51.69.51:6384',
                    'tcp://10.51.69.51:6385',
                    'tcp://10.51.69.51:6386',
                    'tcp://10.51.69.51:6387',
                    'tcp://10.51.69.51:6388',
                    'tcp://10.51.72.166:6379',
                    'tcp://10.51.72.166:6380',
                    'tcp://10.51.72.166:6381',
                    'tcp://10.51.72.166:6382',
                    'tcp://10.51.72.166:6383',
                    'tcp://10.51.72.166:6384',
                    'tcp://10.51.72.166:6385',
                    'tcp://10.51.72.166:6386',
                    'tcp://10.51.72.166:6387',
                    'tcp://10.51.72.166:6388',
                    'tcp://10.173.27.6:6379',
                    'tcp://10.173.27.6:6380',
                    'tcp://10.173.27.6:6381',
                    'tcp://10.173.27.6:6382',
                    'tcp://10.173.27.6:6383',
                    'tcp://10.173.27.6:6384',
                    'tcp://10.173.27.6:6385',
                    'tcp://10.173.27.6:6386',
                    'tcp://10.173.27.6:6387',
                    'tcp://10.173.27.6:6388');
$sourceNeed = array(6);  //需要存到redis的来源方id

function initRedis() {
   define('APP_PATH', dirname(__FILE__));
   require  APP_PATH.'/Predis/Autoloader.php'; 
   \Predis\Autoloader::register();
   $redis = new \Predis\Client($GLOBALS['redisConfig'], array('cluster'=>'redis'));
   $redis->set('fuck', 'you');  // 不可删
   return $redis;
}

function startReadLogToRedis() {
    $_redis = initRedis();
    $redis = $_redis->pipeline();
    $count = 0;
    $perNum = 0;

    $keys = array_flip($GLOBALS['sourceNeed']);
    //开始从标准输入读入数据
    while(!feof(STDIN)) {
        try {
            $line = fgets(STDIN,  1024);
            $data = @json_decode($line, true);
            if(!$data) continue;
            if(isset($keys[$data['ext']]) && $data['m']==0) {
                if(empty($data['uid']) || empty($data['rmid'])) {
        			echo 'uid or rmid is empty'."\n";
					continue;
				}
				$redis->set($data['rmid'], $data['uid']);
				//echo $data['source'],"\t",$data['uid'],"\t",$data['rmid'],"\r\n";
				$count++;
				$perNum++;
			} else {
				continue;
			}
			
			if($perNum > 0 && ($perNum % PACK_NUM == 0)) {
				$redis->execute();
				unset($redis);
				$redis = $_redis->pipeline();
				$perNum = 0;
			}
		} catch(Exception $e) {
			echo "exception happened:".$e->getMessage()."\n";
			$perNum = 0;
			continue;
		}
	}
	if($perNum > 0) {
		try {
			$redis->execute();
		} catch(Exception $e) {
			echo "exception happened:".$e->getMessage();
		}
		unset($redis);
	}
}

//将数据流中的数据记录到redis
startReadLogToRedis();
