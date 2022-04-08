#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <exception>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "hiredis/hiredis.h"
#include "spdlog/spdlog.h"
namespace model {

struct partner {
  int open;
  int type;
  std::string muid;
  std::unordered_map<int, int> weight;  //"weight":{"12":"100"}  pid->weight
  std::unordered_map<int, int> expires; //"weight":{"12":"100"}
  std::unordered_map<int, std::string> url;
  std::string self;
  std::string uid;
};

static std::unordered_map<std::string, partner> partners;

int maxPartnerId = 100;

void syncPartners() {

  spdlog::get("mapping_debug")->info("sync_mapping");

  redisContext *redisC = redisConnect("127.0.0.1", 6379);
  if (redisC != nullptr && redisC->err) {
    char buf[100];
    sprintf(buf, "Redis Error: reply is NULL [%d]\n", __LINE__);
    spdlog::get("mapping_debug")->error(buf);
    redisReconnect(redisC);
    return ;
  }

  for (int i = 0; i <= maxPartnerId; i++) {
    std::string key = "Mapping_" + std::to_string(i);
    std::unordered_map<std::string, std::string> partnerMap{};

    redisReply *reply = (redisReply *)redisCommand(redisC, "HGETALL %s", key.c_str());
    if (reply == nullptr) {
      char buf[100];
      sprintf(buf, "Redis Error: reply is NULL [%d]\n", __LINE__);
      spdlog::get("mapping_debug")->error(buf);
      return ;
    } else if (reply->type == REDIS_REPLY_ERROR) {
      char buf[100];
      freeReplyObject(reply);
      sprintf(buf, "Redis Error:  REDIS_REPLY_ERROR [%d]\n", __LINE__);
      spdlog::get("mapping_debug")->error(buf);
      return ;
    } else if (reply->type != REDIS_REPLY_ARRAY
        || reply->elements == 0) {
      freeReplyObject(reply);
      continue;
    }

    for (size_t i = 0; i < reply->elements; i = i + 2 ) {
      std::string col_key(reply->element[i]->str);
      std::string col_value(reply->element[i+1]->str);
      partnerMap[col_key] = col_value;
    }
    freeReplyObject(reply);

    try {
      partner p;
      p.open = atoi(partnerMap["open"].c_str());
      p.type = atoi(partnerMap["type"].c_str());
      p.self = partnerMap["self"];

      using namespace  boost::property_tree;
      ptree w_ptree, e_ptree, u_ptree;
      std::stringstream ss1, ss2, ss3;

      ss1 << partnerMap["weight"];
      read_json(ss1, w_ptree);
      for (auto w: w_ptree) {
        auto first = atoi(w.first.c_str());
       p.weight[first] = w.second.get_value<int>();
      }

      ss2 << partnerMap["cycle"];
      read_json(ss2, e_ptree);
      for (auto e: e_ptree) {
        auto first = atoi(e.first.c_str());
       p.expires[first] = e.second.get_value<int>();
      }

      ss3 << partnerMap["url"];
      read_json(ss3, u_ptree);
      for (auto u: u_ptree) {
        auto first = atoi(u.first.c_str());
       p.url[first] = u.second.get_value<std::string>();
      }
     p.open = atoi(partnerMap["open"].c_str());
     p.type = atoi(partnerMap["type"].c_str());
     p.self = partnerMap["self"];
     p.muid = atoi(partnerMap["muid"].c_str());
     p.uid = partnerMap["uid"];
     partners[key] = p;
     // spdlog::get("mapping_debug")->info("sync_mapping" + key);
    } catch(std::exception& e){
      spdlog::get("mapping_debug")->debug(e.what());
    }
  }
  redisFree(redisC);
  spdlog::get("mapping_debug")->flush();

}
}

