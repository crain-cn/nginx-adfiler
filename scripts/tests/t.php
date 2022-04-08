<?php

function base62_encode($str) {
    $string = 'vPh7zZwA2LyU4bGq5tcVfIMxJi6XaSoK9CNp0OWljYTHQ8REnmu31BrdgeDkFs';
    $out = '';
    for($t=floor(log10($str)/log10(62)); $t>=0; $t--) {
        $a = floor($str / pow(62, $t));
        $out = $out.substr($string, $a, 1);
        $str = $str - ($a * pow(62, $t));
    }
    return $out;
}

function  base62_decode($s, $ver = false) {
	$base = 62;
	$index = 'vPh7zZwA2LyU4bGq5tcVfIMxJi6XaSoK9CNp0OWljYTHQ8REnmu31BrdgeDkFs';
	$ret = 0;
	$len = strlen($s) - 1;
	for ($t = 0; $t <= $len; $t ++) {
	    $ret += strpos($index, substr($s, $t, 1)) * pow($base, $len - $t);
	}
	return $ret;
}


function createRmidCookie()
{
    $s1 = base62_encode(microtime(true)*1000);
    mt_srand(microtime(true) + getmypid() *1000);
    $s2 = base62_encode(mt_rand(10000, 50000));
    return sprintf("%s%s", $s1, $s2);
}

mt_srand(microtime(true) + getmypid() *1000);
echo mt_rand(10000, 50000) ."\n";
echo createRmidCookie() . "\n";
echo  base62_decode(substr(createRmidCookie() , 0, 7)) ."\n";
echo  base62_decode(substr("6fh7lUjA2X", 0, 7));
