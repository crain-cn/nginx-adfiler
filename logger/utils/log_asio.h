#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include <vector>
#include <thread>
#include <unordered_map>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <ctime>

#include "boost/filesystem.hpp"
#include "boost/thread/thread.hpp"
#include "boost/asio/deadline_timer.hpp"

#include "spdlog/spdlog.h"
typedef std::weak_ptr<spdlog::logger> logger_ptr;

using namespace std::chrono;

namespace httpd {

std::unordered_map<std::string, logger_ptr> log_loggers_;
std::vector<std::string> log_topics_;
int last_hour = 0;

const char* LOG_PATH = "/data/logs";

class Log {
public:


  void static lastHourTimer(boost::asio::deadline_timer *timer) {
    using namespace boost;
    if (timer->expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      timer->expires_at(timer->expires_at() + boost::posix_time::seconds(1));
      timer->async_wait(std::bind(&Log::lastHourTimer, timer));
    }

    std::time_t tt = time(nullptr);
    struct tm tm;
    localtime_r(&tt, &tm);
    if (last_hour != tm.tm_hour) {
      last_hour = tm.tm_hour;
      std::cout << " last_hour " <<  last_hour << std::endl;
    }

  }

  void static flushLoggerTimer(boost::asio::deadline_timer *timer) {
    using namespace boost;
    if (timer->expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      timer->expires_at(timer->expires_at() + boost::posix_time::seconds(3));
      timer->async_wait(std::bind(&Log::flushLoggerTimer, timer));
    }
    for (auto &logger: log_loggers_) {
      logger.second.lock()->flush();
    }
  }

  void static removeLoggerTimer(boost::asio::deadline_timer *timer) {
    if (timer->expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      timer->expires_at(timer->expires_at() + boost::posix_time::minutes(30));
      timer->async_wait(std::bind(&Log::removeLoggerTimer, timer));
    }
    std::time_t before_tt = system_clock::to_time_t(system_clock::now() - hours(1));
    struct tm before_tm;
    localtime_r(&before_tt, &before_tm);

    std::stringstream  before_hour;
    before_hour << "::" <<  before_tm.tm_hour;

    std::vector<std::string> vec;
    for (auto &logger: log_loggers_) {
      auto idx = logger.first.find(before_hour.str());
      if (idx != std::string::npos) {
        std::cout << " spdlog::drop: " << logger.first << std::endl;
        vec.push_back(logger.first);
        spdlog::drop(logger.first);
      }
    }

   for (auto logger_name : vec) {
     std::cout << " erase: " << logger_name << std::endl;
     log_loggers_.erase(logger_name);
   }
   before_hour.clear();

  }

  void static preLoggerTimer(boost::asio::deadline_timer *timer) {
    if (timer->expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      timer->expires_at(timer->expires_at() + boost::posix_time::minutes(30));
      timer->async_wait(std::bind(&Log::preLoggerTimer, timer));
    }

    std::time_t next_tt = system_clock::to_time_t(system_clock::now() + hours(1));
    for (auto &topic: log_topics_) {
      std::cout << "logger::pre_create " << std::endl;
      try {
       Log::createLogger(topic, next_tt);
      } catch (std::exception &e) {
       std::cout << e.what() << std::endl;
      }
    }

  }

  void static createLogger(const std::string &topic, std::time_t tt, bool init = false) {
    if (topic.empty()) {
      return ;
    }
    struct tm tm;
    localtime_r(&tt, &tm);

    std::stringstream  logger_name_ss;
    logger_name_ss << topic << "::" << tm.tm_hour;
    std::string logger_name = logger_name_ss.str();
    logger_name_ss.clear();

    if (log_loggers_.find(logger_name) != log_loggers_.end()) {
      std::cout << logger_name << "....logger has created !"  << std::endl;
    } else {
      std::stringstream  path_ss;
      path_ss << LOG_PATH << "/" << topic;

      std::stringstream  file_ss;
      file_ss <<  path_ss.str() << "/" << std::put_time(&tm, "%Y%m%d%H") << ".log";
      std::cout << "file:" << file_ss.str() << std::endl;

      boost::filesystem::path dir(path_ss.str());
      if(!(boost::filesystem::exists(dir))){
          if (boost::filesystem::create_directory(dir))
              std::cout << "....Successfully Created !" << std::endl;
      }
      try {
        auto logger = spdlog::create<spdlog::sinks::simple_file_sink_st>(logger_name,
            file_ss.str(), false);
        logger->set_pattern("%v"); //logger->set_pattern("[%Y-%b-%d %T.%e]: %v");
        log_loggers_[logger_name] = logger_ptr(logger);
      } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
      }
      if (init == true) {
        log_topics_.push_back(topic);
      }
    }

  }

  std::shared_ptr<spdlog::logger> static get(const std::string &topic) {

    std::stringstream  logger_name_ss;
    logger_name_ss << topic << "::" << last_hour;
    std::string logger_name = logger_name_ss.str();
    logger_name_ss.clear();

    auto logger = spdlog::get(logger_name);
    if (logger == nullptr) {
      _spinlock.lock();
      Log::initLogger(topic);
      _spinlock.unlock();
      auto logger = spdlog::get(logger_name);
    }
    return logger;
  }

  void static initLogger(const std::string &topic) {
    std::cout << "logger::init " << topic << std::endl;
    std::time_t tt = time(nullptr);
    Log::createLogger(topic, tt, true);
  }

  static void runConsumerAndTimer() {
    //boost::split(vLoggers, FLAGS_loggers, boost::is_any_of(", "));
    std::vector<std::string> vLoggers{};
    for (auto logger_name : vLoggers) {
      std::string::iterator end_pos = remove(logger_name.begin(), logger_name.end(), ' ');
      logger_name.erase(end_pos, logger_name.end());
      if (!logger_name.empty()) {
        Log::initLogger(logger_name);
      }
    }
    using namespace boost;
    asio::io_service io_service;
    asio::io_service::work work(io_service);


    asio::deadline_timer timer1(io_service, posix_time::seconds(3));
    timer1.async_wait(std::bind(&Log::flushLoggerTimer, &timer1));

    asio::deadline_timer timer2(io_service, posix_time::microseconds(1));
    timer2.async_wait(std::bind(&Log::removeLoggerTimer, &timer2));

    asio::deadline_timer timer3(io_service, posix_time::microseconds(1));
    timer3.async_wait(std::bind(&Log::preLoggerTimer, &timer3));

    asio::deadline_timer timer4(io_service, posix_time::microseconds(1));
    timer4.async_wait(std::bind(&Log::lastHourTimer, &timer4));

    std::thread t1([&io_service]() {
      io_service.run();
    });

    t1.join();

  }

};
}
