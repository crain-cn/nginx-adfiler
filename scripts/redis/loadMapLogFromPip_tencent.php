<?php
define('PACK_NUM', 200); //redis一次提交的数量

//predis配置
$redisConfig = array(
	'tcp://10.163.3.30:6379',
	'tcp://10.163.3.30:6380',
	'tcp://10.163.3.30:6381',
	'tcp://10.163.3.30:6382',
	'tcp://10.163.3.30:6383',
	'tcp://10.163.3.30:6384',
	'tcp://10.163.3.30:6385',
	'tcp://10.163.3.30:6386',
	'tcp://10.163.3.30:6387',
	'tcp://10.163.3.30:6388',
	'tcp://10.170.228.157:6379',
	'tcp://10.170.228.157:6380',
	'tcp://10.170.228.157:6381',
	'tcp://10.170.228.157:6382',
	'tcp://10.170.228.157:6383',
	'tcp://10.170.228.157:6384',
	'tcp://10.170.228.157:6385',
	'tcp://10.170.228.157:6386',
	'tcp://10.170.228.157:6387',
	'tcp://10.170.228.157:6388',
	'tcp://10.170.240.155:6379',
	'tcp://10.170.240.155:6380',
	'tcp://10.170.240.155:6381',
	'tcp://10.170.240.155:6382',
	'tcp://10.170.240.155:6383',
	'tcp://10.170.240.155:6384',
	'tcp://10.170.240.155:6385',
	'tcp://10.170.240.155:6386',
	'tcp://10.170.240.155:6387',
	'tcp://10.170.240.155:6388',
	'tcp://10.172.219.218:6379',
	'tcp://10.172.219.218:6380',
	'tcp://10.172.219.218:6381',
	'tcp://10.172.219.218:6382',
	'tcp://10.172.219.218:6383',
	'tcp://10.172.219.218:6384',
	'tcp://10.172.219.218:6385',
	'tcp://10.172.219.218:6386',
	'tcp://10.172.219.218:6387',
	'tcp://10.172.219.218:6388',
	'tcp://10.172.228.231:6379',
	'tcp://10.172.228.231:6380',
	'tcp://10.172.228.231:6381',
	'tcp://10.172.228.231:6382',
	'tcp://10.172.228.231:6383',
	'tcp://10.172.228.231:6384',
	'tcp://10.172.228.231:6385',
	'tcp://10.172.228.231:6386',
	'tcp://10.172.228.231:6387',
	'tcp://10.172.228.231:6388',
	'tcp://10.172.229.205:6379',
	'tcp://10.172.229.205:6380',
	'tcp://10.172.229.205:6381',
	'tcp://10.172.229.205:6382',
	'tcp://10.172.229.205:6383',
	'tcp://10.172.229.205:6384',
	'tcp://10.172.229.205:6385',
	'tcp://10.172.229.205:6386',
	'tcp://10.172.229.205:6387',
	'tcp://10.172.229.205:6388'
);
$sourceNeed = array(2);  //需要存到redis的来源方id

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
			if(isset($keys[$data['ext']]) && $data['m']==0 && $data['f']==1) {
				if(empty($data['uid']) || empty($data['rmid'])) {
					echo 'uid or rmid is empty'."\n";
					continue;
				}
				$redis->set($data['uid'], $data['rmid']);
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
