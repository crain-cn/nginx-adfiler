#ifndef MAPPING_MODULE
#define MAPPING_MODULE 1
extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>

  extern ngx_module_t  ngx_http_mapping_module;

  ngx_module_t *ngx_modules[] = {
      &ngx_http_mapping_module,
      NULL
  };

  char *ngx_module_names[] = {
      (char *)"ngx_http_mapping_module",
      NULL
  };

  char *ngx_module_order[] = {
      NULL
  };

}

#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>

using namespace boost::interprocess;

typedef struct {
  std::shared_ptr<message_queue>   log_mq;
  std::shared_ptr<message_queue>   scylla_mq;
  std::shared_ptr<message_queue>   test_mq;
} ngx_mapping_main_conf_t;


static void *
ngx_mapping_create_main_conf(ngx_conf_t *cf);
static ngx_int_t ngx_mapping_preconfig(ngx_conf_t *cf);
static char *ngx_http_mapping(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_mapping_ku6(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_mapping_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_mapping_ku6_handler(ngx_http_request_t *r);

static ngx_int_t ngx_http_mapping_init_module(ngx_cycle_t *cycle);
static ngx_int_t ngx_http_mapping_init_worker(ngx_cycle_t *cycle);
static void ngx_http_mapping_exit_worker(ngx_cycle_t *cycle);

static ngx_command_t ngx_http_mapping_commands[] = {

    {
      ngx_string("mapping"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes|no arguments*/
      ngx_http_mapping, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL
    },
    {
      ngx_string("mapping_ku6"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes|no arguments*/
      ngx_http_mapping_ku6, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL
    },
   //kafka  ennd
    ngx_null_command /* command termination */
};


static ngx_http_module_t ngx_http_mapping_module_ctx = {
    NULL,// ngx_mapping_preconfig, /* preconfiguration */
    NULL, /* postconfiguration */

    ngx_mapping_create_main_conf, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */


    NULL,   /* create local conf */
    NULL, /* merge location conf */
};

/* Module definition. */
ngx_module_t ngx_http_mapping_module = {
    NGX_MODULE_V1,
    &ngx_http_mapping_module_ctx, /* module context */
    ngx_http_mapping_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    ngx_http_mapping_init_module, /* init module */
    ngx_http_mapping_init_worker, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL,// ngx_http_mapping_exit_worker, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_mapping_create_main_conf(ngx_conf_t *cf)
{
    ngx_mapping_main_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_mapping_main_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->log_mq = nullptr;
    conf->scylla_mq = nullptr;
    conf->test_mq = nullptr;
    return conf;
}

ngx_int_t
ngx_http_mapping_init_module(ngx_cycle_t *cycle) {

 // message_queue::remove("mapping_queue");
   try {
      //Create a message_queue.
     message_queue test_mq
        (open_or_create               //only create
        ,"test_mq"           //name
        ,100000                    //max message number
        ,1000                       //max message size
      );

     message_queue log_mq
        (open_or_create               //only create
        ,"mapping_log"           //name
        ,100000                    //max message number
        ,400                       //max message size
     );

     message_queue scylla_mq
        (open_or_create               //only create
        ,"mapping_scylla"           //name
        ,100000                    //max message number
        ,100                       //max message size
      );
   } catch (interprocess_exception& e) {
     std::cout << e.what( ) << std::endl;
   }
   return 0;
}

#endif
