<?php

$redis = new redis();
$redis->connect("127.0.0.1", "6379");

$fp = fopen("./mapping.log.prod","r");
while(!feof($fp)){
    $str = fgets($fp, 1024);
    list($key, $data) = explode("::", $str);
        echo $key ."\n";
    $data = json_decode($data, true);
    if (empty($data)) {
        continue;
    }
    foreach ($data as $field => $v ) {
        if ($field == "url" || $field  =="cycle"|| $field  =="weight") {
           $v = json_encode($v);
        }
        $redis->hset($key, $field, $v);   
    }
}
var_dump($redis->hgetall("Mapping_17"));
