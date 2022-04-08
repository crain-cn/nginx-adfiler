#ifndef UTILS_COOKIE_H_
#define UTILS_COOKIE_H_
#include <string>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <sstream>

#include "assert.h"

using namespace std::chrono;

class cookie {
 public:
  static bool cookieExists(const std::string header, const char *name) {
    assert(name && *name);
    int len = strlen(name);
    bool hasValue = (strchr(name, '=') != nullptr);
    for (size_t pos = header.find(name); pos != std::string::npos;
         pos = header.find(name, pos + 1)) {
      if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
        pos += len;
        if (hasValue) {
          if (pos == header.size() || header[pos] == ';') return true;
        } else {
          if (pos < header.size() && header[pos] == '=') return true;
        }
      }
    }
    return false;
  }

  static std::string getCookie(const std::string header, const std::string &name) {
    assert(!name.empty());
   // std::string header = getHeader("Cookie");
    for (size_t pos = header.find(name); pos != std::string::npos;
         pos = header.find(name, pos + 1)) {
      if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
        pos += name.size();
        if (pos < header.size() && header[pos] == '=') {
          size_t end = header.find(';', pos + 1);
          if (end != std::string::npos) end -= pos + 1;
          return header.substr(pos + 1, end);
        }
      }
    }
    return "";
  }

  static std::string  setCookie(const std::string& name,
                        const std::string& value,
                        int64_t expire  = 0,
                            const std::string& path = "",
                            const std::string& domain = "",
                            bool secure  = false,
                            bool httponly = false,
                            bool encode_url = false) {

    std::string encoded_value;
    int len = 0;
    if (!value.empty() ) {
      encoded_value = encode_url ? cookie::url_encode(value.c_str()) : value;
      len += encoded_value.size();
    }
    len += path.size();
    len += domain.size();

    std::string cookie;
    cookie.reserve(len + 100);
    struct tm tm;
    if (value.empty()) {
      cookie += name.data();
      cookie += "=deleted; expires=";
      auto now = system_clock::now();
      std::time_t tt = system_clock::to_time_t(now - seconds(1));
      localtime_r(&tt, &tm);
      std::stringstream gmt;
      gmt << std::put_time(&tm, "%a, %d-%b-%Y %T GMT");
      cookie += gmt.str();
      cookie += "; Max-Age=0";
    } else {
      cookie += name.data();
      cookie += "=";
      cookie += encoded_value.empty() ? "" : encoded_value.data();
      if (expire > 0) {
        if (expire > 253402300799LL) {
         // raise_warning("Expiry date cannot have a year greater than 9999");
          return "";
        }
        cookie += "; expires=";
        auto now = system_clock::now();
        std::time_t tt = system_clock::to_time_t(now + seconds(expire));
        localtime_r(&tt, &tm);
        std::stringstream gmt;
        gmt << std::put_time(&tm, "%a, %d-%b-%Y %T GMT");
        cookie += gmt.str();
        cookie += "; Max-Age=" + std::to_string(expire);
      }
    }

    if (!path.empty()) {
      cookie += "; path=";
      cookie += path.data();
    }
    if (!domain.empty()) {
      cookie += "; domain=";
      cookie += domain.data();
    }
    if (secure) {
      cookie += "; secure";
    }
    if (httponly) {
      cookie += "; httponly";
    }
    return cookie;
  }


  static std::string url_encode(const char *in) {

    return std::string();
  }
};
#endif
