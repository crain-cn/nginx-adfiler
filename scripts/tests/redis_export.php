<?php

$redis = new redis();
$redis->connect("127.0.0.1", "6379");
$key = "D_2043691";
$fp = fopen('/dev/shm/$key.json', 'a');

    $key = "D_2043691";
    $rst = $redis->hGetAll($key);
    if (!empty($rst)) {
        foreach ($rst as $field => $value) {
          
                $rst[$field] = $value;
            
        }
      fwrite($fp, json_encode($rst) ."\n");
    }

fclose($fp);
