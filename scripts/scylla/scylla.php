<?php

class scylla {

    
    public static $session;
        
    public static $month_millisecond;
    public static $hour_millisecond;
    public static $month_microsecond;
    
    
    public static $tablePairs =[
        0 => "test",
        2 => "tencent",
        3 => "iqiyi",
        5 => "sohu",
        6 => "miaozhen",
        8 => "youku",
        10 => "ku6",
        11 => "fengxing",
        21 => "letv",
        26 => "pptv",
        27 => "yinyuetai",
        29 => "mgtv",
        //31 => "amnet"
    ];
    
    
    public static $hosts_arr = [
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
    
    /*
    * @project https://github.com/datastax/php-driver
    * @Api doc https://datastax.github.io/php-driver
    */
    public static function createClustersSession() {
  
        $keyspace  ='mapping';
        $hosts = implode(",", self::$hosts_arr);
        $cluster = Cassandra::cluster()    
                ->withPersistentSessions(true)
                ->withDefaultConsistency(Cassandra::CONSISTENCY_TWO)
                ->withIOThreads(2)
                ->withProtocolVersion(3)
                ->withRoundRobinLoadBalancingPolicy()
                ->withContactPoints($hosts)            
                ->build();

        self::$session  = $cluster->connect($keyspace); 
              
        self::$month_microsecond = strtotime((new DateTime())->format('Y-m-1 00:00:00'));
        self::$month_millisecond  = self::$month_microsecond * 1000;
        self::$hour_millisecond = strtotime((new DateTime())->format('Y-m-d H:00:00')) * 1000;
 
        echo date('Y-m-d H:i:s'). " scylla created" . "\n";

    }
         
    public static function del( $table, $uid) {
        $session = self::$session;
        $result = "";
        $cql = sprintf("DELETE FROM mapping.%s WHERE uid = '%s'", $table, $uid);
        $statement = new Cassandra\SimpleStatement($cql);

        $session->executeAsync($statement);
           
        return $result;
    }

             
    public static function saveCounter($table, $count) {
        $millisecond = strtotime((new DateTime())->format('Y-m-d H:i:s')) * 1000;
        
        $session = self::$session;
        $cql = sprintf("INSERT INTO mapping.counter (mapping_table, count, last_access) VALUES('%s', %ld, %ld)",
                    $table, 
                    $count, 
                    $millisecond 
                );
        
        $statement = new Cassandra\SimpleStatement($cql);
        $session->executeAsync($statement); 
 
    }


    public static function insert($table, $row, $async = true) {
        
        if (empty($row['rmid']) || empty($row['uid'])) {
            return ;
        }
        
        $session = self::$session;
        $result = "";
        $cql = sprintf("INSERT INTO mapping.%s  (rmid, uid, last_access) VALUES ('%s', '%s', %ld)", 
            $table,        
            $row['rmid'],
            $row['uid'],
            self::$month_millisecond
        );
        
        $statement = new Cassandra\SimpleStatement($cql); 
        
        if ($async) {
           $session->executeAsync($statement); 
        } else {
           $session->execute($statement); 
        } 

        return $result;
    }


    public static function batchInsert($table, array $rows, $async = true) {
        $result = "";
        $session = self::$session;
        $cql = sprintf("INSERT INTO mapping.%s  (rmid, uid, last_access) VALUES (?, ?, ?)", $table);
        $simple    = new Cassandra\SimpleStatement($cql );
        $batch     = new Cassandra\BatchStatement(Cassandra::BATCH_LOGGED);
        foreach ($rows as $row) {
            if (empty($row['rmid']) || empty($row['uid'])) {
                continue;
            }
        
            $batch->add($simple, array(
                $row['rmid'],
                $row['uid'],
                new Cassandra\Timestamp(self::$month_microsecond)  // =  $month_microsecond *1000
            ));
        }
        
        if ($async) {
           $session->executeAsync($batch); 
        } else {
           $session->execute($batch); 
        } 
        return $result;
    }

    
    public static function cinLog($table, $special = false, $print = false) {
        if (!in_array($table, self::$tablePairs)) {
            echo "$table not found int tablePairs";
            return ;
        } else {
            echo "$table mapping ... \n";
        }

        foreach(self::$tablePairs as $key => $value) {
            if ($value == $table) {
               $ext = $key;
            }
        }
        
        $i = 0; 
        $mobile_num = 0;
        $empty_num = 0;
           
        $start_time = microtime(true);
        
        $special_table = '';
        if ($special) {
           $special_table = sprintf("%s_dmp", $table); //youku_dmp, iqiyi_dmp
        }
        
        while(!feof(STDIN)) {
            $string =  str_replace(PHP_EOL, '', fgets(STDIN,  1024));
            $data = json_decode($string, true);
            
            if (empty( $data['uid']) || empty($data['rmid'])) {
                $empty_num++;
                continue;
            }

            if ($data['m'] == 1) {
                $mobile_num++;
                continue;
            }

            if ($data['ext'] != $ext ) {
                continue;
            }

            $i++;
            if ($print) {
                echo sprintf("%d %s::%s",   $i, $data['rmid'], $data['uid']) ."\n";
            }
            scylla::insert($table,  $data);
            
            if ($special && !empty($special_table)) {
                scylla::insert($special_table,  $data);
            }
        }   
        if ($i > 0) {
            scylla::saveCounter($table, $i);
            if ($special && !empty($special_table)) {
                scylla::saveCounter($special_table, $i);
            }
        }
           
        $end_time = microtime(true);
        $total_time = ($end_time  - $start_time);
        $res = [
            'table' => $table,
            'total_num' => $i,
            'empty_num' => $empty_num,
            'empty_percent' => ($empty_num/$i) * 100 ."%",
            'total_time' => $total_time ."s",
            'time' => date('Y-m-d H:i:s')
        ];

        $json_res = json_encode($res);
        echo $json_res . "\n"; 
        $f = fopen("/data/logs/scylla_res.log", "a+");
        fwrite($f,  $json_res."\n");
        fclose($f);
    }

}

