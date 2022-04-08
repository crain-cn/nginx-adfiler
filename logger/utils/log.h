//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <mutex>
#include <string>
#include <cerrno>
#include <iomanip>

namespace spdlog
{
namespace sinks
{

/*
 * Default generator of daily log file names.
 */
struct hour_calculator
{
    // Create filename for the form basename.YYYYMMDDhh.extension
    static filename_t calc_filename(const filename_t& path)
    {
        struct tm tm;
        std::time_t tt = time(nullptr);
        localtime_r(&tt, &tm);
        std::stringstream log_file;
        log_file << path << "/" << std::put_time(&tm, "%Y%m%d%H") << ".log";
        return log_file.str();
    }
};

/*
 * Rotating file sink based on date. rotates at midnight
 */
template<class Mutex, class FileNameCalc = hour_calculator>
class hour_file_sink :public base_sink < Mutex >
{
public:
    //create daily file sink which rotates on given time
    hour_file_sink(
        const filename_t& _base_path): _base_path(_base_path)
    {

        _rotation_tp = _next_rotation_tp();
        _file_helper.open(FileNameCalc::calc_filename(_base_path));
    }

    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (std::chrono::system_clock::now() > _rotation_tp)
        {
           _rotation_tp = _next_rotation_tp();
           _file_helper.open(FileNameCalc::calc_filename(_base_path));
        }
        _file_helper.write(msg);
    }

private:
    std::chrono::system_clock::time_point _next_rotation_tp()
    {
        struct tm tm;
        std::time_t tt = time(nullptr);
        localtime_r(&tt, &tm);
        tm.tm_min = 0;
        tm.tm_sec = 0;
        tm.tm_hour += 1;
        auto rotation_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        return std::chrono::system_clock::time_point(rotation_time);
    }

    filename_t _base_path;
    std::chrono::system_clock::time_point _rotation_tp;
    details::file_helper _file_helper;
};

typedef hour_file_sink<std::mutex> hour_file_sink_mt;
typedef hour_file_sink<details::null_mutex> hour_file_sink_st;
}
}
