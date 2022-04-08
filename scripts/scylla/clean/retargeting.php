<?php
require './common.php';

$cql = "SELECT  * FROM retargeting.thirdparty  WHERE expires < '2017-08-01 00:00:00' ALLOW FILTERING";
$statement = new Cassandra\SimpleStatement($cql); 
$rows = $session->execute($statement, new Cassandra\ExecutionOptions([
    'page_size' => 50
]));

$i = 1;
while ($i < 100) {
    $i++;
    echo " page: " . $i . "\n";
    foreach ($rows as $row) {
        echo $row['key'] . " - ".$row['expires'] ."\n";
    }

    if ($rows->isLastPage()) {
        break;
    }
    $rows = nextPage($rows);

}

function nextPage($rows) {
    
    try {
        $rows = $rows->nextPage();
    } catch (Exception $exc) {
        nextPage($rows);
        echo $exc->getTraceAsString();
    }
    return $rows;
}