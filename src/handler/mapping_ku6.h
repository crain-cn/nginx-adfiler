#include "httpd/handler_base.h"
#include "utils/helper.h"
#include "utils/cookie.h"

using namespace httpd;
namespace handler {
/*
 * @name k.php
 *
 * 酷6 mapping引流入口
 *
 * 酷6的mapping方式与其他媒体不同,必须由我们发起mapping(但是我们无法做到)
 * 因此建立此引流入口,酷6会把此gif图片放入曝光地址,曝光后直接回跳酷6,最终跳转到三方mapping入口
 *
 * @notice 此入口的唯一作用就是引流,没有其他任何作用[不记录日志],只要访问就会直接跳转酷6
 *
 */

class mapping_ku6 : public httpd::handler_base {
public:

  const std::string COOKIE_KEY = "a";//RM Rmid Cookie Key
  const int DSP_ID = 100026;

  virtual std::unique_ptr<response> handle(
      std::unique_ptr<request> req,
      std::unique_ptr<response> rep) {

    auto gifData = base64_decode("R0lGODlhAQABAIAAAAAAAAAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==");
    rep->set_mime_type("image/gif");
    rep->add_header("cache-control", "no-cache,must-revalidate");
    rep->add_header("Expires", "0");
    rep->add_header("Content-Length", std::to_string(gifData.length()));
    rep->_content = gifData;

    auto cookieHeader = req->get_header("Cookie");
    auto cookie = cookie::getCookie(cookieHeader, COOKIE_KEY);
    std::string rmid = "";
    if (!cookie.empty()) {
      rmid = cookie;
    } else {
      rmid = createRmidCookie();
    }

    //加入noredirect=1 则酷6就不会跳转
    char l_buffer[100];
    sprintf(l_buffer, "http://cm.fastapi.net/?dspid=%d&dspuid=%s&gethuid=1", DSP_ID, rmid.c_str());

    rep->set_status(response::status_type::moved_temporarily);
    rep->add_header("Location", l_buffer);
    return std::move(rep);
  }
};

}
