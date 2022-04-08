extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace httpd {
class response {
public:
  enum class status_type {
      ok = 200, //!< ok
      created = 201, //!< created
      accepted = 202, //!< accepted
      no_content = 204, //!< no_content
      multiple_choices = 300, //!< multiple_choices
      moved_permanently = 301, //!< moved_permanently
      moved_temporarily = 302, //!< moved_temporarily
      not_modified = 304, //!< not_modified
      bad_request = 400, //!< bad_request
      unauthorized = 401, //!< unauthorized
      forbidden = 403, //!< forbidden
      not_found = 404, //!< not_found
      internal_server_error = 500, //!< internal_server_error
      not_implemented = 501, //!< not_implemented
      bad_gateway = 502, //!< bad_gateway
      service_unavailable = 503  //!< service_unavailable
  } _status;

  std::unordered_map<std::string, std::string>  _headers;

  std::vector<std::string> _cookies;

  std::string _version;

  std::string _content;

  std::string _content_type;

  response&add_header(const std::string& h, const std::string& value) {
      if (h == "Set-Cookie") {
        _cookies.push_back(value);
      } if (h == "Content-Type") {
        _content_type = value;
      } else {
        _headers[h] = value;
      }
      return *this;
  }

  response&set_version(const std::string& version) {
      _version = version;
      return *this;
  }

  response&set_status(status_type status, const std::string& content = "") {
      _status = status;
      if (content != "") {
          _content = content;
      }
      return *this;
  }


  response&set_mime_type(const std::string& mime) {
      _content_type = mime;
      return *this;
  }

  char* ngx_copy_string(ngx_pool_t* pool, std::string src) {
      ngx_uint_t buffer_size = src.size() + 1;
      char* dst = static_cast<char*>(ngx_palloc(pool, buffer_size));
      if (dst == NULL) {
      //  LOG(ERROR) << "ngx_copy_string: ngx_palloc() returned NULL";
        return NULL;
      }
      strcpy(dst, src.c_str());
      dst[buffer_size-1] = '\0';
      return dst;
  }

  ngx_int_t send_headers(ngx_http_request_t* r) {
    auto headers_out = &r->headers_out;
    auto ngx_header = [&] (std::string key, std::string value) {
      auto header = reinterpret_cast<ngx_table_elt_t*>
        (ngx_list_push(&headers_out->headers));
      header->hash = 1;
      header->key.data = reinterpret_cast<u_char*>(ngx_copy_string(r->pool, key));
      header->key.len = key.length();
      header->value.data = reinterpret_cast<u_char*>(ngx_copy_string(r->pool, value));
      header->value.len = value.length();
      return header;
    };

    for (auto header : _headers) {
      auto key = header.first;
      auto value = header.second;
      if (key.compare("Date") == 0) {
        headers_out->date = ngx_header(key, value);
      } else if (key.compare("Etag") == 0) {
        headers_out->etag = ngx_header(key, value);
      } else if (key.compare("Expires") == 0) {
        headers_out->expires = ngx_header(key, value);
      } else if (key.compare("Last-Modified") == 0) {
        headers_out->last_modified = ngx_header(key, value);
      } else if (key.compare("Location") == 0) {
        headers_out->location = ngx_header(key, value);
      } else if (key.compare("Server") == 0) {
        headers_out->server = ngx_header(key, value);
      } else if (key.compare("Content-Encoding") == 0) {
        headers_out->content_encoding = ngx_header(key, value);
      } else if (key.compare("Refresh") == 0) {
        headers_out->refresh = ngx_header(key, value);
      } else if (key.compare("Content-Range") == 0) {
        headers_out->content_range = ngx_header(key, value);
      } else if (key.compare("Accept-Ranges") == 0) {
        headers_out->accept_ranges = ngx_header(key, value);
      } else if (key.compare("WWW-Authenticate") == 0) {
        headers_out->www_authenticate = ngx_header(key, value);
      } else {
        ngx_header(key, value);
      }
    }

    ngx_buf_t *b;
    ngx_chain_t out;

    b = reinterpret_cast<ngx_buf_t*>(ngx_pcalloc(r->pool, sizeof(ngx_buf_t)));
    out.buf = b;
    out.next = NULL;

    b->pos      = (u_char*)_content.c_str();
    b->last     = b->pos + _content.length();
    b->memory   = 1;
    b->last_buf = 1;

    headers_out->status = (int)_status;
    headers_out->content_length_n = _content.length();

    headers_out->content_type.data = (u_char*)_content_type.c_str();
    headers_out->content_type.len = _content_type.length();

    if (_cookies.size() > 0) {
       for (auto cookie : _cookies) {
         ngx_header("Set-Cookie", cookie);
       }
    }

    ngx_http_send_header(r);
    return ngx_http_output_filter(r, &out);
  }
};
}
