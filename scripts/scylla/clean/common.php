<?php


function nextPage($rows, $i) {
    echo " page: " . $i . "\n";
    try { 
        $i++;
        $rows = $rows->nextPage();
    } catch (Exception $exc) {
        $rows = nextPage($rows, $i);
        echo $exc->getTraceAsString();
    }
    return $rows;
}

function mappingClean($table, $month) {
    $host = '10.28.205.85, 10.30.137.254, 10.30.137.244, 10.30.141.35, 10.30.249.92, 10.29.131.97, 10.30.249.76, 10.28.206.221';
    $cluster   = Cassandra::cluster()
                ->withContactPoints($host)
                ->withRoundRobinLoadBalancingPolicy()  
                ->build();
    $session   = $cluster->connect("mapping");

    $expirs = date("Y-m-1 00:00:00",strtotime("-$month month"));
    echo $expirs ."\n";
   
    $cql = "SELECT  * FROM mapping.$table WHERE last_access <= '$expirs' ALLOW FILTERING";
    $statement = new Cassandra\SimpleStatement($cql); 
    $rows = $session->execute($statement, new Cassandra\ExecutionOptions([
        'page_size' => 10
    ]));

    $i = 1;
    while ($i < 100) {
       
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
        $rows = nextPage($rows,  $i);
    }

}