#ifndef UTILS_HELPER_H_
#define UTILS_HELPER_H_
#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include <math.h>
#include <unistd.h>

using namespace std::chrono;
const std::string BASE_STRING = "vPh7zZwA2LyU4bGq5tcVfIMxJi6XaSoK9CNp0OWljYTHQ8REnmu31BrdgeDkFs";

size_t strpos(const std::string &haystack, const std::string &needle) {
  int sleng = haystack.length();
  int nleng = needle.length();
  if (sleng == 0 || nleng == 0)
    return std::string::npos;

  for (int i = 0, j = 0; i< sleng; j = 0, i++ ) {
    while (i+j < sleng && j < nleng && haystack[i+j] == needle[j])
      j++;
    if (j == nleng)
      return i;
  }
  return std::string::npos;
}

std::string base62_encode(int64_t num) {
  std::string out = "";
  for (auto t = floor(log10(num)/log10(62)); t >= 0; t--) {
    auto a = floor(num / pow(62, t));
    out += BASE_STRING.substr(a, 1);
    num = num - (a * pow(62, t));
  }
  return out;
}

int64_t base62_decode(const std::string str) {
  int64_t num = 0;
  int len = str.length() - 1;
  for(int t = 0; t <= len; t ++) {
    num += strpos(BASE_STRING, str.substr(t, 1)) * pow(62, len - t);
  }
  return num;
}

std::string createRmidCookie() {
  using namespace std::chrono;
  auto ms = duration_cast<std::chrono::milliseconds>
    (system_clock::now().time_since_epoch()).count();

  auto ns = duration_cast<std::chrono::nanoseconds>
    (system_clock::now().time_since_epoch()).count();

  std::default_random_engine generator(ns);
  std::uniform_int_distribution<int> dis(10000, 50000);
  std::string s1 = base62_encode(ms);  //7位数长度
  std::string s2 = base62_encode(dis(generator)); //3位数长度

  auto str = s1+s2;
  return str;
}

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
#endif
