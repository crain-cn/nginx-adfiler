<?php
require dirname(__FILE__). '/scylla.php';

$options = getopt("t:s:v:");

$table = trim($options['t']);
$special = isset($options['s']) ? true : false;
$print = isset($options['v']) ? true : false;

scylla::createClustersSession();
scylla::cinlog($table, $special, $print);