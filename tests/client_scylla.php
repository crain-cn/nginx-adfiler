<?php

$client = new swoole_client(SWOOLE_SOCK_TCP);

$client->connect('127.0.0.1', 6889);

$fp = fopen("./pptv_uids_1212.log", "r");
$i = 0;
$empty_num  = 0;
$big_num = 0;
$total_time = 0;


    
$cmd_json = json_encode([
    "cmd" => "set",
    "pub" => 108,
    "var1" => "a1",
    "var2" => "b121"
]);

$client->send($cmd_json);
 echo $client->recv();

 $cmd_json = json_encode([
    "cmd" => "get",
    "pub" => 108,
    "var1" => "a1"
]);

$client->send($cmd_json);
echo $client->recv(); 

    

$fp_res = fopen("./pptv_res.log", "w+");
while(!feof($fp)) {
    $key = substr(fgets($fp),0, -1);
    if  (empty($key)) {
        continue;
    }
    $i++;
    
    if ($i % 1000 == 0) {
       echo $big_num;
       sleep(2);
      echo "sleep 2..........." ."\n";
    }
    
    if ($i % 10000 == 0) {
      break;
    }
    $cmd_json = json_encode([
        "cmd" => "get",
        "pub" => 108,
        "var1" => $key
    ]);

    $start_time = microtime(true);
    $client->send($cmd_json);
    $data = json_decode($client->recv(), true);
    $end_time = microtime(true);
    $req_time = ($end_time  - $start_time) * 1000;
    $total_time += $req_time;
       
    $row_res = json_encode([
        'uid' => $key,
        'rmid' => $data,
        'req_time' => $req_time
    ]);
    if (empty($data)) {
        $empty_num++;
        echo $row_res ."\n";
        fwrite($fp_res, $row_res."\n");
    }

    if ($req_time > 5) {
        $big_num++;
     
        echo $row_res ." > 5" ."\n";
        fwrite($fp_res, $row_res."\n");
    }

} 

$res = [
    'total_num' => $i,
    'empty_num' => $empty_num,
    'big_num' => $big_num,
    'empty_percent' => ($empty_num/$i) * 100 ."%",
    'big_percent' => ($big_num/$i) * 100 ."%",
    'avg_time' => $total_time/$i
];

echo json_encode($res) . "\n";
$client->close();

fclose($fp_res);
fclose($fp);