extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}
#include <string>
#include <vector>
#include <unordered_map>
#include <experimental/string_view>

namespace httpd {
class request {
public :
    std::string _method;
    std::string _url;
    std::string _version;
    int http_version_major;
    int http_version_minor;

    size_t content_length = 0;
    std::unordered_map<std::string, std::string> _headers;
    std::unordered_map<std::string, std::string> _query_parameters;

    std::string get_header(const std::string& name) const {
         auto res = _headers.find(name);
         if (res == _headers.end()) {
             return "";
         }
         return res->second;
     }

     std::string get_query_param(const std::string& name) const {
         auto res = _query_parameters.find(name);
         if (res == _query_parameters.end()) {
             return "";
         }
         return res->second;
     }

     void copy_headers(const ngx_list_t &from, int http_version) {
       http_version_major = http_version / 1000;
       http_version_minor = http_version % 1000;

       ngx_uint_t i;
       const ngx_list_part_t* part = &from.part;
       const ngx_table_elt_t* header = static_cast<ngx_table_elt_t*>(part->elts);

       for (i = 0 ; /* void */; i++) {
         if (i >= part->nelts) {
           if (part->next == NULL) {
             break;
           }

           part = part->next;
           header = static_cast<ngx_table_elt_t*>(part->elts);

           i = 0;
         }

         if (header[i].hash == 0) {
           continue;
         }
         auto key = std::string((char*)header[i].key.data, header[i].key.len);
         auto value = std::string((char*)header[i].value.data, header[i].value.len);
         _headers[key] = value;
       }
     }

     void copy_uri(ngx_str_t uri) {
       _url = std::string((char *)uri.data, uri.len);
       set_query_param();
     }

     std::string get_url() {
       return _url;
     }

protected:
    static short hex_to_byte(char c) {
        if (c >='a' && c <= 'z') {
            return c - 'a' + 10;
        } else if (c >='A' && c <= 'Z') {
            return c - 'A' + 10;
        }
        return c - '0';
    }

    /**
     * Convert a hex encoded 2 bytes substring to char
     */
    static char hexstr_to_char(const std::experimental::string_view& in, size_t from) {

        return static_cast<char>(hex_to_byte(in[from]) * 16 + hex_to_byte(in[from + 1]));
    }

    /**
     * URL_decode a substring and place it in the given out std::string
     */
    static bool url_decode(const std::experimental::string_view& in, std::string& out) {
        size_t pos = 0;
        char buff[in.length()];
        for (size_t i = 0; i < in.length(); ++i) {
            if (in[i] == '%') {
                if (i + 3 <= in.size()) {
                    buff[pos++] = hexstr_to_char(in, i + 1);
                    i += 2;
                } else {
                    return false;
                }
            } else if (in[i] == '+') {
                buff[pos++] = ' ';
            } else {
                buff[pos++] = in[i];
            }
        }
        out = std::string(buff, pos);
        return true;
    }

    void add_param(const std::experimental::string_view& param) {
        size_t split = param.find('=');

        if (split >= param.length() - 1) {
            std::string key;
            if (url_decode(param.substr(0,split) , key)) {
                _query_parameters[key] = "";
            }
        } else {
            std::string key;
            std::string value;
            if (url_decode(param.substr(0,split), key)
                    && url_decode(param.substr(split + 1), value)) {
                _query_parameters[key] = value;
            }
        }

    }

    std::string set_query_param() {
        size_t pos = _url.find('?');
        if (pos == std::string::npos) {
            return _url;
        }
        size_t curr = pos + 1;
        size_t end_param;
        std::experimental::string_view url = _url;
        while ((end_param = _url.find('&', curr)) != std::string::npos) {
            add_param(url.substr(curr, end_param - curr) );
            curr = end_param + 1;
        }
        add_param(url.substr(curr));
        return _url.substr(0, pos);
    }

};
}
