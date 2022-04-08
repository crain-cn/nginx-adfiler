<?php
define('PACK_NUM', 200); //redis一次提交的数量

//predis配置
$redisConfig = array('tcp://10.170.237.112:6379','tcp://10.170.237.112:6380','tcp://10.170.237.112:6381','tcp://10.170.237.112:6382','tcp://10.170.237.112:6383','tcp://10.170.237.112:6384','tcp://10.170.237.112:6385','tcp://10.170.237.112:6386','tcp://10.170.237.112:6387','tcp://10.170.237.112:6388','tcp://10.170.243.116:6379','tcp://10.170.243.116:6380','tcp://10.170.243.116:6381','tcp://10.170.243.116:6382','tcp://10.170.243.116:6383','tcp://10.170.243.116:6384','tcp://10.170.243.116:6385','tcp://10.170.243.116:6386','tcp://10.170.243.116:6387','tcp://10.170.243.116:6388','tcp://10.170.246.132:6379','tcp://10.170.246.132:6380','tcp://10.170.246.132:6381','tcp://10.170.246.132:6382','tcp://10.170.246.132:6383','tcp://10.170.246.132:6384','tcp://10.170.246.132:6385','tcp://10.170.246.132:6386','tcp://10.170.246.132:6387','tcp://10.170.246.132:6388','tcp://10.170.239.217:6379','tcp://10.170.239.217:6380','tcp://10.170.239.217:6381','tcp://10.170.239.217:6382','tcp://10.170.239.217:6383','tcp://10.170.239.217:6384','tcp://10.170.239.217:6385','tcp://10.170.239.217:6386','tcp://10.170.239.217:6387','tcp://10.170.239.217:6388');
$sourceNeed = array(5);  //需要存到redis的来源方id

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
