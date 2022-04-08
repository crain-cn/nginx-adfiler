<?php
require dirname(__FILE__). '/scylla.php';

$context = new ZMQContext();
$subscriber = new ZMQSocket($context, ZMQ::SOCKET_SUB);
$subscriber->connect("tcp://localhost:5556");
#$subscriber->setSockOpt(ZMQ::SOCKOPT_SUBSCRIBE, "B");
$subscriber->setSockOpt(ZMQ::SOCKOPT_SUBSCRIBE, "");

scylla::createClustersSession();
while (true) {
    $content = $subscriber->recv();
    list($ext, $rmid, $uid) = explode(", ", $content);
    if (empty(scylla::$tablePairs[$ext])) {
        continue;
    }
    $table = scylla::$tablePairs[$ext];
    $insert_data = ['rmid' => $rmid, 'uid' => $uid];
    if ($ext == 3 || $ext == 8) {
        $special_table = sprintf("%s_dmp", $table);
        scylla::insert($special_table, $insert_data);
    }
    scylla::insert($table, $insert_data);
    //echo sprintf("%s, %s, %s", $ext, $rmid, $uid) . PHP_EOL ;
}