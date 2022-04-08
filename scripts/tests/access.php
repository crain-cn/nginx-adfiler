<?php

$fp = fopen("./access.log","r");
   $ch = curl_init();
   $i =0;
while(!feof($fp)){  
    $ch = curl_init();
    
   $str = fgets($fp, 1024);
   $data = preg_match ('([GET|POST][^"]+)', $str, $matches);
   list($t, $url, $h)= explode(" ", $matches[0]);
   echo $url;
   //var_dump($matches[0]);
$i++;
   curl_setopt($ch, CURLOPT_URL, "http://127.0.0.1:10001". $url);
   curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
   curl_setopt($ch, CURLOPT_HEADER, 0);
   $output = curl_exec($ch);
 echo $i ."\n";
 
}
  curl_close($ch);