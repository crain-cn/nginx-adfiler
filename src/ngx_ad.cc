#include "ngx_mapping.h"
#include "handler/mapping.h"
#include "handler/mapping_ku6.h"

static ngx_int_t
ngx_http_handler_mapping(ngx_http_request_t *r)
{
  ngx_mapping_main_conf_t  *main_conf;
  main_conf = ngx_http_get_module_main_conf(r, ngx_http_mapping_module);

  std::unique_ptr<httpd::request> req(new httpd::request());
  std::unique_ptr<httpd::response> rep(new httpd::response());

  req->copy_headers(r->headers_in.headers, r->http_version);
  req->copy_uri(r->unparsed_uri);

  handler::mapping mapping;
  mapping.log_mq = main_conf->log_mq;
  mapping.scylla_mq = main_conf->scylla_mq;
  mapping.test_mq = main_conf->test_mq;
  rep = mapping.handle(std::move(req), std::move(rep));
  return rep->send_headers(r);
}

static char
*ngx_http_mapping(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {

    ngx_http_core_loc_conf_t *clcf;
    clcf = (ngx_http_core_loc_conf_t *)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_handler_mapping;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_handler_mapping_ku6(ngx_http_request_t *r) {

  std::unique_ptr<httpd::request> req(new httpd::request());
  std::unique_ptr<httpd::response> rep(new httpd::response());

  req->copy_headers(r->headers_in.headers, r->http_version);
  req->copy_uri(r->unparsed_uri);

  handler::mapping_ku6 mapping;

  rep = mapping.handle(std::move(req), std::move(rep));
  return rep->send_headers(r);
}

static char
*ngx_http_mapping_ku6(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {

    ngx_http_core_loc_conf_t *clcf;
    clcf = (ngx_http_core_loc_conf_t *)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_handler_mapping_ku6;

    return NGX_CONF_OK;
}

//timer
static ngx_connection_t dummy;
static ngx_event_t ngx_ev_partner;

static void
ngx_timer_partner(ngx_event_t *ev)
{
    model::syncPartners();
    ngx_add_timer(ev, 1000 * 60 * 5);
}

ngx_int_t
ngx_http_mapping_init_worker(ngx_cycle_t *cycle) {

  size_t q_size = 4096;
  spdlog::set_async_mode(q_size, spdlog::async_overflow_policy::block_retry);

  spdlog::create<spdlog::sinks::simple_file_sink_st>
    ("mapping_debug", "/data/logs/mapping_debug.log", false);

  model::syncPartners();

  //auto logger = spdlog::create<spdlog::sinks::hour_file_sink_mt>
  //  ("cookie_mapping", "/data/logs/cookie_mapping");
  //logger->set_pattern("%v");

  dummy.fd = (ngx_socket_t) -1;
  ngx_memzero(&ngx_ev_partner, sizeof(ngx_event_t));
  ngx_ev_partner.handler = ngx_timer_partner;
  ngx_ev_partner.log = cycle->log;
  ngx_ev_partner.data = &dummy;
  ngx_add_timer(&ngx_ev_partner, 1000 * 60 * 5);
  ngx_mapping_main_conf_t  *main_conf;

  main_conf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_mapping_module);
  main_conf->log_mq = std::make_shared<message_queue>(open_only, "mapping_log");
  main_conf->scylla_mq = std::make_shared<message_queue>(open_only, "mapping_scylla");
  main_conf->test_mq = std::make_shared<message_queue>(open_only, "test_mq");
  return 0;
}

