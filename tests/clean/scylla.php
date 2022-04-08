<?php

$hosts_arr = [
    "10.28.205.85", 
    "10.30.137.254", 
    "10.30.137.244", 
    "10.30.141.35", 
    "10.30.249.92", 
    "10.29.131.97", 
    "10.30.249.76", 
    "10.28.206.221", 
    "10.144.113.242", 
    "10.144.113.195",
    "10.144.115.144",
    "10.144.115.137",
    "10.144.115.13",
    "10.144.115.36",
    "10.28.207.220", 
    "10.144.115.188",
];
$hosts = implode(",", $hosts_arr);   

$cluster   = Cassandra::cluster()
            ->withContactPoints($hosts)
            ->build();
$session   = $cluster->connect("mapping");

$cql = "SELECT  * FROM mapping.tencent ";
$statement = new Cassandra\SimpleStatement($cql); 
$rows = $session->execute($statement, new Cassandra\ExecutionOptions([
    'page_size' => 500
]));

$i = 100;
while ($i > 0) {
    $i --;
    echo "entries in page: " . $rows->count() . "\n";

    foreach ($rows as $row) {
       var_dump($row);
    }

    if ($rows->isLastPage()) {
        break;
    }

    $rows = $rows->nextPage();
}
