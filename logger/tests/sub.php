<?php
/*
* Pubsub envelope subscriber
* @author Ian Barber <ian(dot)barber(at)gmail(dot)com>
*/

//  Prepare our context and subscriber
$context = new ZMQContext();
$subscriber = new ZMQSocket($context, ZMQ::SOCKET_SUB);
$subscriber->connect("tcp://localhost:5563");
$subscriber->connect("tcp://localhost:5556");
#$subscriber->setSockOpt(ZMQ::SOCKOPT_SUBSCRIBE, "B");
$subscriber->setSockOpt(ZMQ::SOCKOPT_SUBSCRIBE, "");
while (true) {
    #$address = $subscriber->recv();
    $contents = $subscriber->recv();
    echo $contents. PHP_EOL ;

}
