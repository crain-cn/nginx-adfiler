<?php


function nextPage($rows, $i) {
    echo " page: " . $i++ . "\n";
    if ($i > 100) {
        exit;
    }
    try { 
        $rows = $rows->nextPage();
    } catch (Exception $exc) {
        $rows = nextPage($rows, $i);
        echo $exc->getTraceAsString();
    }
    return $rows;
}

function mappingClean($table, $month) {

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
                ->withRoundRobinLoadBalancingPolicy()  
                ->build();
    $session   = $cluster->connect("mapping");

    $expirs = date("Y-m-1 00:00:00",strtotime("-$month month"));
    echo $expirs ."\n";
   
    $cql = "SELECT  * FROM mapping.$table WHERE last_access < '$expirs' ALLOW FILTERING";
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

function mappingDmpClean($table, $month) {
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
                ->withRoundRobinLoadBalancingPolicy()  
                ->build();
    $session   = $cluster->connect("mapping");

    $expirs = date("Y-m-1 00:00:00",strtotime("-$month month"));
    echo $expirs ."\n";
   
    $cql = "SELECT  * FROM mapping.$table WHERE last_access < '$expirs' ALLOW FILTERING";
    $statement = new Cassandra\SimpleStatement($cql); 
    $rows = $session->execute($statement, new Cassandra\ExecutionOptions([
        'page_size' => 500
    ]));

    $i = 1;
    while ($i < 200) {
       
        foreach ($rows as $row) {
            $last_access = $row['last_access']->time() * 1000;
            $uid = $row['uid'];
            $rmid = $row['rmid'];
            echo  $rmid ." - ".$last_access ."\n";
            $cql = sprintf("DELETE FROM mapping.%s WHERE rmid = '%s' AND last_access = %ld",  $table, $rmid, $last_access);
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