<?php

class logformat {

    //cat 2017041114.log  | grep $'108\t' | grep '"os":"Personal Computer"'  | grep '"newrmid":1' -v | php  logformat.php core
    public static function coreLog() {
  
        $f = fopen("/data/format_log/core_res.log", "w+");
        $i = 0;
        while(!feof(STDIN)) {
            $string = fgets(STDIN,  1024);
            $json =  strstr($string, '{');
            $data = json_decode($json, true);
            if (empty( $data['muid'])) {
                echo $string;
                break;
            }
            $i++;
            echo $i . "::" . $data['muid'] ."\n";
            fwrite($f,  $data['muid']."\n");
        }   
        fclose($f);

    }
    
    //cat 2017041210.log  | grep '"ext":26' | php pptv_mlog.php 
    public static function mappingLog() {
  
        $f = fopen("/data/format_log/mapping_res.log", "w+");
        $i = 0;
        while(!feof(STDIN)) {
            $string = fgets(STDIN,  1024);
            $data = json_decode($string, true);
            if (empty($data['uid']) || empty($data['rmid'])) {
                continue;
            }
            $i++;
            echo $i . "::" . $data['uid'] ."\n";
            fwrite($f,  $data['uid']."\n");
        }   
        fclose($f);

    }
}

if (isset($argv[1])) {
    exit;
}

switch ($argv[1]) {
    case "core":
        logformat::coreLog();
        break;
    case "mapping":
        logformat::mappingLog();
        break;
    default:
        exit("null \n");
        break;
}


