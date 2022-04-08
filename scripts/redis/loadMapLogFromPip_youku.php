<?php
define('PACK_NUM', 200); //redis一次提交的数量

//predis配置
$redisConfig = array('tcp://10.44.162.35:6379',
					'tcp://10.44.162.35:6380',
					'tcp://10.44.162.35:6381',
					'tcp://10.44.162.35:6382',
					'tcp://10.44.162.35:6383',
					'tcp://10.44.162.35:6384',
					'tcp://10.44.162.35:6385',
					'tcp://10.44.162.35:6386',
					'tcp://10.44.162.35:6387',
					'tcp://10.44.162.35:6388',
					'tcp://10.44.160.254:6379',
					'tcp://10.44.160.254:6380',
					'tcp://10.44.160.254:6381',
					'tcp://10.44.160.254:6382',
					'tcp://10.44.160.254:6383',
					'tcp://10.44.160.254:6384',
					'tcp://10.44.160.254:6385',
					'tcp://10.44.160.254:6386',
					'tcp://10.44.160.254:6387',
					'tcp://10.44.160.254:6388',
					'tcp://10.44.161.246:6379',
					'tcp://10.44.161.246:6380',
					'tcp://10.44.161.246:6381',
					'tcp://10.44.161.246:6382',
					'tcp://10.44.161.246:6383',
					'tcp://10.44.161.246:6384',
					'tcp://10.44.161.246:6385',
					'tcp://10.44.161.246:6386',
					'tcp://10.44.161.246:6387',
					'tcp://10.44.161.246:6388');
$sourceNeed = array(8);  //需要存到redis的来源方id

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
