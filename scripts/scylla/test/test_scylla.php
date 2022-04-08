<?php
require './scylla.php';

scylla::createClustersSession();

$rows = [
    ['rmid' => "1", 'uid' => 'b1'],
    ['rmid' => "2", 'uid' => 'b2'],
];
scylla::batchInsert("test", $rows);


$row = ['rmid' => "1", 'uid' => 'i1'];
scylla::insert("test", $row);


scylla::saveCounter("test", 1);