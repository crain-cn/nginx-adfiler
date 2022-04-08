#ifndef HANDLERS_HH_
#define HANDLERS_HH_
#include <unordered_map>
#include "httpd/request.h"
#include "httpd/response.h"

namespace httpd {

class handler_base {
public:

    virtual std::unique_ptr<response> handle(std::unique_ptr<request> req, std::unique_ptr<response> rep) = 0;

    virtual ~handler_base() = default;

};

}
#endif /* HANDLERS_HH_ */
