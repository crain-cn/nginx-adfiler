/*
 *  mappings
    {2, "tencent"},
    {3, "iqiyi"},
    {5, "sohu"},
    {6, "miaozhen"},
    {8, "youku"},
    {10, "ku6"},
    {11, "fengxing"},
    {21, "letv"},
    {26, "pptv"},
    {27, "yinyuetai"},
    {29, "mgtv"}
*/
#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "rapidjson/document.h"
#include "spdlog/spdlog.h"
#include "utils/log.h"
#include "utils/threadPool.h"

ThreadPool pool(10);

class log_server {
private:
  zmq::context_t zmq_ctx_;
  zmq::socket_t zmq_pub_;
public:

  log_server() : zmq_ctx_(1), zmq_pub_(zmq_ctx_, ZMQ_PUB) {};

  void zmq_bind(std::string addr) {
    zmq_pub_.bind(addr);
  }

  void static init_logger() {
    size_t q_size = 4096;
    spdlog::set_async_mode(
        q_size,
        spdlog::async_overflow_policy::block_retry,
        nullptr,
        std::chrono::seconds(1)
    );

    auto logger = spdlog::create<spdlog::sinks::hour_file_sink_mt>
     ("cookie_mapping", "/data/logs/cookie_mapping");
    logger->set_pattern("%v");

  }

  void scylla_consumer() {
    using namespace boost::interprocess;
    try {
      message_queue mq(open_only, "mapping_scylla");
      while(true) {
        unsigned int priority;
        message_queue::size_type recvd_size;
        char buffer[1024] = {0};
        mq.receive(&buffer, sizeof(buffer), recvd_size, priority);
       // printf("scylla::%s\n", buffer);
        std::string msg(buffer);
        try {
          zmq::message_t zmq_msg(msg.size());
          memcpy ((void *)zmq_msg.data (), msg.c_str(), msg.size());
          zmq_pub_.send(zmq_msg);
        } catch (zmq::error_t &e) {
          std::cout << e.what() << std::endl;
        }
      }
    } catch(interprocess_exception &ex){
      std::cout << ex.what() << std::endl;
    }
  }

  void static log_consumer() {
      init_logger();
      using namespace boost::interprocess;
      auto logger = spdlog::get("cookie_mapping");
      try {
        message_queue mq(open_only,"mapping_log");
        auto logger = spdlog::get("cookie_mapping");
        while(true) {
          unsigned int priority;
          message_queue::size_type recvd_size;
          char buffer[1024] = {0};
          mq.receive(&buffer, sizeof(buffer), recvd_size, priority);
        //  printf("log::%s\n", buffer);
          std::string msg(buffer);
          logger->info(msg);
        }
      } catch(interprocess_exception &ex){
        std::cout << ex.what() << std::endl;
      }
    }

  void static test_consumer() {
      size_t q_size = 4096;
      spdlog::set_async_mode(
          q_size,
          spdlog::async_overflow_policy::block_retry,
          nullptr,
          std::chrono::seconds(1)
      );

    auto logger = spdlog::create<spdlog::sinks::hour_file_sink_mt>
     ("test_logger", "/data/logs/test");
    logger->set_pattern("%v");

     using namespace boost::interprocess;
     try {
       message_queue mq(open_only,"test_mq");
       auto logger = spdlog::get("test_logger");
       while(true) {
         unsigned int priority;
         message_queue::size_type recvd_size;
         char buffer[1024] = {0};
         mq.receive(&buffer, sizeof(buffer), recvd_size, priority);
       //  printf("log::%s\n", buffer);
         std::string msg(buffer);
         logger->info(msg);
       }
     } catch(interprocess_exception &ex){
       std::cout << ex.what() << std::endl;
     }
    }
};

int main () {
  std::thread log_thread([&]() {
    log_server::log_consumer();
  });

  std::thread test_thread([&]() {
    log_server::test_consumer();
  });

  std::thread scylla_thread([&]() {
    log_server server;
    server.zmq_bind("tcp://127.0.0.1:5556");
    server.scylla_consumer();
  });

  scylla_thread.join();
  log_thread.join();
  test_thread.join();
  return 0;
}
