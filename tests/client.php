<?php

$client = new swoole_client(SWOOLE_SOCK_TCP);
$timeout = 0.005; //5ms
$client->connect('127.0.0.1', 8848, $timeout);

$fp = fopen("./pptv_uids.log", "r");
$i = 0;
while(!feof($fp)) {
    $key = substr(fgets($fp),0, -1);
    $i++;
    if ($i == 100) {
        break;
    }
    $start_time = microtime(true);
    $client->send("get $key 108\r\n");
    echo $client->recv() ."\n";
    $end_time = microtime(true);
    $total = ($end_time  - $start_time) * 1000;
    echo "cos time：{$total} ms \n"; 
    if ($total > 5) {
        echo "get $key 108\r\n";
        break;
    }
} 
$client->close();

