<?php
require './common.php';

$cql = "SELECT  * FROM mapping.tencent WHERE last_access < '2017-05-01 00:00:00' ALLOW FILTERING";
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
