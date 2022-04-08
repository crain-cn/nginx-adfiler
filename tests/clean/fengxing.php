<?php
require './common.php';

$table = "fengxing";
$cql = "SELECT  * FROM mapping.$table WHERE last_access < '2017-05-01 00:00:00' ALLOW FILTERING";
$statement = new Cassandra\SimpleStatement($cql); 
$rows = $session->execute($statement, new Cassandra\ExecutionOptions([
    'page_size' => 50
]));

$i = 1;
while ($i < 100) {
    $i++;
    echo " page: " . $i . "\n";
    foreach ($rows as $row) {
        $last_access = $row['last_access']->time() * 1000;
        $uid = $row['uid'];
        echo  $uid ." - ".$last_access ."\n";
        $cql = sprintf("DELETE FROM mapping.%s WHERE uid = '%s' AND last_access = %ld",  $table, addslashes($uid), $last_access);
      //  echo $cql ."\n";
        $statement = new Cassandra\SimpleStatement($cql);
        $session->executeAsync($statement);
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