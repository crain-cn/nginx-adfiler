<?php


function mappingClean($table, $month) {
    $host = '10.30.141.35';
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
        'page_size' => 500
    ]));

    $i = 1;
    while ($i < 100) {
       
        foreach ($rows as $row) {
            $last_access = $row['last_access']->time() * 1000;
            $uid = $row['uid'];
            $rmid = $row['rmid'];
            echo  $rmid ." - ".$last_access ."\n";
            $cql = sprintf("DELETE FROM mapping.%s WHERE uid = '%s' AND last_access = %ld",  $table, addslashes($uid), $last_access);
          //  echo $cql ."\n";
            $statement = new Cassandra\SimpleStatement($cql);
            $session->executeAsync($statement);
        }

        if ($rows->isLastPage()) {
            break;
        }
        $i++;
        $rows = nextPage($rows, $i);
    }

}


$table = "tencent";
$month = 2;
mappingClean($table, $month);
