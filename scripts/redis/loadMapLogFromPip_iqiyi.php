<?php
define('PACK_NUM', 200); //redis一次提交的数量

//predis配置
$redisConfig = array('tcp://10.170.234.28:6379','tcp://10.170.234.28:6380','tcp://10.170.234.28:6381','tcp://10.170.234.28:6382','tcp://10.170.234.28:6383','tcp://10.170.234.28:6384','tcp://10.170.234.28:6385','tcp://10.170.234.28:6386','tcp://10.170.234.28:6387','tcp://10.170.234.28:6388','tcp://10.170.246.30:6379','tcp://10.170.246.30:6380','tcp://10.170.246.30:6381','tcp://10.170.246.30:6382','tcp://10.170.246.30:6383','tcp://10.170.246.30:6384','tcp://10.170.246.30:6385','tcp://10.170.246.30:6386','tcp://10.170.246.30:6387','tcp://10.170.246.30:6388','tcp://10.170.224.72:6379','tcp://10.170.224.72:6380','tcp://10.170.224.72:6381','tcp://10.170.224.72:6382','tcp://10.170.224.72:6383','tcp://10.170.224.72:6384','tcp://10.170.224.72:6385','tcp://10.170.224.72:6386','tcp://10.170.224.72:6387','tcp://10.170.224.72:6388','tcp://10.170.246.60:6379','tcp://10.170.246.60:6380','tcp://10.170.246.60:6381','tcp://10.170.246.60:6382','tcp://10.170.246.60:6383','tcp://10.170.246.60:6384','tcp://10.170.246.60:6385','tcp://10.170.246.60:6386','tcp://10.170.246.60:6387','tcp://10.170.246.60:6388');
$sourceNeed = array(3);  //需要存到redis的来源方id

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
