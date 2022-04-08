#include "httpd/handler_base.h"
#include <memory>
#include <list>
#include <sys/time.h>
#include <chrono>
#include <random>
#include <unordered_map>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "utils/cookie.h"
#include "utils/helper.h"
#include "model/partner.h"

#include <boost/interprocess/ipc/message_queue.hpp>

using namespace httpd;
/*
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
std::vector<int> mapping_vec {2,3,5,6,8,10,11,21,26,27,29,31};

namespace handler {
const std::string COOKIE_KEY = "a";//RM Rmid Cookie Key
const std::string COOKIE_MZ = "amz";//秒针 Cookie Key
const std::string COOKIE_TIME = "t_";//来源方mapping时间 Cookie Key
const std::string MAPPING_KEY = "Mapping_";

const std::string LOG_TOPIC = "cookie_mapping";

class mapping : public handler_base {
 public:
  std::shared_ptr<message_queue>   log_mq;
  std::shared_ptr<message_queue>   scylla_mq;
  std::shared_ptr<message_queue>   test_mq;
  enum class ext_type: int {
    q  =  2,//腾讯
    a  =  3,//爱奇艺
    s  =  5,//搜狐
    m  =  6,//秒针
    y  =  8,//优酷
    x  =  9,//reachmax
    k  =  10,//ku 6
    f  = 11,//风行
    b =  12,//暴风
    t =  13//土豆
  };
  enum class error {
    error  =  0,
    ERROR_NO_EXT      =  1,
    ERROR_NO_UID      =  2,
    ERROR_NO_PARTNER  =  3,
    ERROR_BAD_EXT     =  4,
    ERROR_NO_URL      =  5,
    ERROR_REDIS_TIMEOUT  = 6,
    ERROR_COOKIE_GET  =  7
  };

   virtual std::unique_ptr<response> handle(
       std::unique_ptr<request> req,
       std::unique_ptr<response> rep) {
    auto gifData = base64_decode("R0lGODlhAQABAIAAAAAAAAAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==");
    rep->_content = gifData;
    rep->set_mime_type("image/gif");
    rep->add_header("Cache-control", "no-cache,must-revalidate");
    rep->add_header("Expires", "0");
    rep->add_header("Content-Length", std::to_string(gifData.length()));

    cookieHeader_ = req->get_header("Cookie");
    query_url_ = req->get_url();
    std::string ext_str = req->get_query_param("ext");
    request_.source = req->get_query_param("s");
    std::string userAgent = req->get_header("User-Agent");
    handleExt(ext_str);

    if (!request_.uid_name.empty()) {
      uid_ = req->get_query_param(request_.uid_name);
      if (uid_.size() == 0) {
        auto tmp = req->get_query_param("uid");
        if (tmp.size() > 0) {
          uid_ = tmp;
        }
      }
    }

    rep = std::move(recordCookieLog(std::move(rep)));

    if(request_.error > 0) return end_normal(std::move(rep));
    if(request_.partner.open == 0)  {
      return end_normal(std::move(rep));
    }

    //adm 不限制移动端
    if (ext_ != 4) {
      request_.m = 0;
      if (token_match_size(userAgent, "android|iphone|ipad") > 1) {
        request_.m = 1;
        return end_normal(std::move(rep));
      }
    }

    dofilter();

    //tx->mz/adm->rm->tx 算主动mapping ext_type::q
    if (ext_ == (int)ext_type::q && request_.pid == 12) {
      if (request_.partner.self.size() > 0) {
        std::string pKey = MAPPING_KEY + std::to_string(ext_);
        auto partner = model::partners.find(pKey);
        auto tx_self = partner->second.self;
        auto url = url_replace(tx_self);
        return header_location(std::move(rep), url);
      }
    }

    //没有筛选到可用合作方
    if (request_.pid == 0) {
      //来源方保存cookie映射关系 跳转到来源方
      if (request_.partner.self.size() > 0) {
        auto url = url_replace(request_.partner.self);
        return header_location(std::move(rep), url);
      } else {
        return end_normal(std::move(rep));
      }
    }

    //设置mapping周期cookie
    auto cookie_key = COOKIE_TIME + std::to_string(request_.pid);

    auto rep_cookie= cookie::setCookie(cookie_key, std::to_string(time(nullptr)) ,3153600000);
    rep->add_header("Set-Cookie", rep_cookie);

    if (request_.url.size() == 0) {
        request_.error = (int)error::ERROR_NO_URL;
        return end_normal(std::move(rep));
    }

    auto url = url_replace(request_.url);
    //admaster 被动mapping redir=http://m.reachmax.cn/r.gif?ext=7&s=${source} 需转义
    if(request_.pid == 7) {
      boost::regex reg("&s=") ;
      std::string url = boost::regex_replace(url, reg, "%26s=");
    }
    return header_location(std::move(rep), url);
  }

  std::unique_ptr<response> end_normal(std::unique_ptr<response> rep) {
    rep->set_status(response::status_type::ok);
    rep->add_header("error", std::to_string(request_.error));
    log_save("");
    return std::move(rep);
  }

  std::unique_ptr<response> header_location(std::unique_ptr<response> rep, std::string url) {
    log_save(url);
    rep->set_status(response::status_type::moved_temporarily);
    rep->add_header("P3P", "CP=CURa ADMa DEVa PSAo PSDo OUR BUS UNI PUR INT DEM STA PRE COM NAV OTC NOI DSP COR");
    rep->add_header("Location", url);
    return std::move(rep);
  }

  void log_save(std::string r_url) {
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> log_writer(s);

    log_writer.StartObject();

    log_writer.Key("source");
    log_writer.String(request_.source.c_str());

    log_writer.Key("ext");
    log_writer.Uint(ext_);
    log_writer.Key("f");
    log_writer.Uint(request_.flag);

    log_writer.Key("m");
    log_writer.Uint(request_.m);

    log_writer.Key("type");
    log_writer.Uint(request_.type);

    std::stringstream  time_ss;
    std::time_t tt = std::time(nullptr);

    struct tm tm;
    localtime_r(&tt, &tm);
    time_ss << std::put_time(&tm, "%F %T");
    log_writer.Key("time");
    log_writer.String(time_ss.str().c_str());

    log_writer.Key("uid");
    log_writer.String(uid_.c_str());
    log_writer.Key("rmid");
    log_writer.String(rmid_.c_str());

    log_writer.Key("is_new");
    log_writer.Uint(request_.is_new);
    log_writer.Key("winner");
    log_writer.Uint(request_.pid);
    log_writer.Key("is_open");
    log_writer.Uint(request_.partner.open);
    log_writer.Key("error");
    log_writer.Uint(request_.error);

    log_writer.Key("usetime");
    log_writer.Uint(0);

    log_writer.EndObject();


    auto ad_match_size = token_match_size(query_url_, "ad.gif");
    if (ad_match_size == 1) {
      try {
         std::string test_msg = query_url_ + "  "+ r_url + ",m::" + std::to_string(request_.m) + ",e::"
             + std::to_string(request_.error);
         test_mq.get()->send(test_msg.c_str(), test_msg.length(), 0);
       } catch (interprocess_exception& e) {
          std::cout << e.what( ) << std::endl;
       }
    }

    std::string msg;
    try {
       msg = s.GetString();
       log_mq.get()->send(msg.c_str(), msg.length(), 0);
    } catch (interprocess_exception& e) {
       std::cout << e.what( ) << std::endl;
    }
    auto send_scylla = [&]() {
       std::stringstream ss;
       ss << ext_ << ", " << rmid_ << ", " << uid_;
       msg = ss.str();
       try {
         scylla_mq.get()->send(msg.c_str(), msg.length(), 0);
       } catch (interprocess_exception& e) {
          std::cout << e.what( ) << std::endl;
       }
    };
    if (!rmid_.empty() && !uid_.empty() && request_.m == 0) {
      switch (ext_) {
        case (int)ext_type::q:
          if (request_.flag == 1) {
            send_scylla();
          }
          break;
        default:
          auto find_res = std::find(std::begin(mapping_vec), std::end(mapping_vec), ext_);
           if (find_res != std::end(mapping_vec)) {
             send_scylla();
           }
          break;
      }
    }
   // auto logger = spdlog::get("cookie_mapping");
    //if (logger != nullptr) {
    //  logger->info(std::string(s.GetString()));
    //}
    uid_.clear();
    rmid_.clear();
    cookieHeader_ .clear();
    query_url_.clear();
    request_ = {};

  }

  std::string url_replace(std::string url) {
    if (url.size() == 0) return "";
    using namespace std::chrono;
    auto ns = duration_cast<std::chrono::nanoseconds>
      (system_clock::now().time_since_epoch()).count();
   /*
    std::default_random_engine generator(ns);
    std::uniform_int_distribution<int> dis(10000, 99999);
    auto random = std::to_string(ns) + std::to_string(dis(generator));
    */
    auto timestramp = std::to_string(time(nullptr));
    auto random = std::to_string(ns);
    std::list<std::string> keys{"${rmid}", "${time}", "${random}", "${source}"};
    std::list<std::string> values{this->rmid_, timestramp, random, this->request_.source};
    for (auto key : keys) {
      auto value = values.front();
      values.pop_front();
      boost::replace_all(url, key, value);
    }
    return url;
  }

  std::unique_ptr<response> recordCookieLog(std::unique_ptr<response> rep) {
    auto cookie = cookie::getCookie(cookieHeader_, COOKIE_KEY);
    std::string path("/");
    std::string domain(".reachmax.cn");
    if (cookie.empty()) {
      request_.is_new = 1;
      if (request_.flag == 0) {
        request_.error = (int)error::ERROR_COOKIE_GET;
      } else {
        this->rmid_ = createRmidCookie();
        auto rep_cookie= cookie::setCookie(COOKIE_KEY, this->rmid_, 3153600000 , path, domain);
        rep->add_header("Set-Cookie", rep_cookie);
      }
    } else {
      this->rmid_ = cookie;
      request_.is_new = 0;
    }

    //秒针 cookie也要写在主域名下
    if(ext_ == (int)ext_type::m) {
      auto mzid = cookie::getCookie(cookieHeader_, COOKIE_MZ);
      if(mzid.empty()) {
        //86400 = 24 h    3153600000 = 365*86400*100 = 100y
        auto mz_cookieHeader = cookie::setCookie(COOKIE_MZ, uid_, 3153600000 , path, domain);
        rep->add_header("Set-Cookie", mz_cookieHeader);
      }
    }

    return rep;
  }

  int token_match_size(std::string query_string, std::string token_regex) {
    std::string text(query_string);
    boost::regex regex(token_regex, boost::regex::icase);
    boost::sregex_token_iterator iter(text.begin(), text.end(), regex, 0);
    boost::sregex_token_iterator end;
    int n = 0;
    for( ; iter != end; ++iter ) {
        //std::cout<<*iter<<'\n';
        n++;
    }
    return n;
  }

  bool handleExt(std::string ext_str) {
    if (ext_str == "q") {
      ext_ = (int)ext_type::q;
    } else if (ext_str == "a") {
      ext_ = (int)ext_type::a;
    } else if (ext_str == "s") {
      ext_ = (int)ext_type::s;
    } else if (ext_str == "m") {
      ext_ = (int)ext_type::m;
    } else if (ext_str == "y") {
      ext_ = (int)ext_type::y;
    } else if (ext_str == "x") {
      ext_ = (int)ext_type::m;
    } else if (ext_str == "k") {
      ext_ = (int)ext_type::k;
    } else if (ext_str == "f") {
      ext_ = (int)ext_type::f;
    } else if (ext_str == "b") {
      ext_ = (int)ext_type::b;
    } else if (ext_str == "t") {
      ext_ = (int)ext_type::t;
    } else if (ext_str.size() > 0) {
      ext_ = std::atoi(ext_str.c_str());
    } else {
      ext_ = 0;
    }

    if (ext_ == 0) {
      auto ad_match_size = token_match_size(query_url_, "ad.gif");
      if (ad_match_size == 1) {
        ext_ = 4;
        request_.uid_name = "aid";
      }
    }

    if (ext_ == 0) {
      auto ad_match_size = token_match_size(query_url_, "bd.gif");
      if (ad_match_size == 1) {
        ext_ = 1;
      }
    }

    //获取来源方
    if (request_.source.size() > 0) {
       request_.flag = 0;
    } else {
      request_.source = "";


     //腾讯dmp跳转回来时URL中有两个ext  后一个是来源方标识
      auto match_size = token_match_size(query_url_, "ext=");
      if (match_size == 2) {
        request_.source = std::to_string(ext_);
        ext_ = (int)ext_type::q;
        request_.flag = 0;
      } else {
        request_.source = std::to_string(ext_);
        request_.flag = 1;
      }

    }
    auto source =  request_.source;
    std::string pKey = MAPPING_KEY + source;
   // std::cout << "key:" << pKey <<std::endl;
    auto partner = model::partners.find(pKey);
    //判断是否找到可用的媒体方
    if (partner == model::partners.end()) {
      request_.error = (int)error::ERROR_BAD_EXT;
      return false;
    }

    request_.partner = partner->second;

    if (atoi(request_.source.c_str()) != ext_) {
      std::string pKey = MAPPING_KEY + std::to_string(ext_);
      auto p2 = model::partners.find(pKey);
      if (request_.uid_name.size() == 0
          && !p2->second.uid.empty()) {
        request_.uid_name = p2->second.uid;
      }
    } else {
      if (request_.uid_name.size() == 0) {
         request_.uid_name = request_.partner.uid;
      }
    }

     //如果没有开启mapping则直接退出
    //1/DMP 2/媒体 3/DSP
    request_.type = request_.partner.type;
    return true;
  }

  void dofilter() {
    using namespace  boost::property_tree;
    std::unordered_map<int, int> weight{};
    auto parter = request_.partner;
    //判断有效期
    request_.pid = checkVaildExpire(parter.weight, parter.expires);
    if (request_.pid == 0) return ;
    //检查可跳转的url是否存在,如果不存在当做无法跳转处理
    if (parter.url.find(request_.pid) != parter.url.end()) {
      request_.url = parter.url[request_.pid];
    }
  }

//  .weight{ {4, 100}, {7,90}},
//  .expires { {4, 1296000}, {7, 2592000}},
 // .url { {4, "http"}},//{"4":"http:}
   int checkVaildExpire(std::unordered_map<int, int> weight,
      std::unordered_map<int, int>expires) {
    if (weight.size() == 0) {
      return 0;
    }
    std::unordered_map<int, int> winners{};
    for (auto e : expires) {
      int pid = e.first;
      int expire = e.second;
      auto value = cookie::getCookie(cookieHeader_, COOKIE_TIME + std::to_string(pid));
      //判断是否符合时间周期
      if(value.size() == 0) {
         //跳转到合作方后，获取时间失败，加限制条件，防止重复跳转
        if((ext_ == 6 && pid == 4) || (ext_ == 4 && pid == 7)){
          continue;
        }
        winners[pid] = weight.find(pid) != weight.end() ? weight[pid] : 0;
      } else {
          if(atoi(value.c_str()) + expire < time(nullptr)) {
              //发现时间过期,需要可以重新mapping
              winners[pid] = weight.find(pid) != weight.end() ? weight[pid] : 0;
          }
      }
    }
    if (winners.size() == 0) return 0;
    int maxWeight = 0, lastPid = 0, sum = 0;
    for (auto w : winners) {
      sum += w.second;
      if (w.second > maxWeight) {
        maxWeight = w.second;
      }
    }
    std::vector<int> maxWeightPids{};
    for (auto w : winners) {
      if (w.second == maxWeight) {
        maxWeightPids.push_back(w.first);
      }
    }
    if (maxWeightPids.size() > 0) {
      std::random_shuffle(maxWeightPids.begin(), maxWeightPids.end());
      lastPid = maxWeightPids.front();
    }
    if (lastPid == 0 || sum == 0) return 0;
    return lastPid;
  }
 private:
   struct requestData {
     int error = 0;
     std::string source;
     int type = 0;
     int flag = 0;
     model::partner partner;
     std::string url;
     int pid = 0;
     int m = 0;
     int is_new = 0;
     std::string uid_name;
   } request_;

  std::string query_url_;
  std::string cookieHeader_;
  std::string rmid_;
  std::string uid_;
  int ext_ = 0;
};

}
