cd nginx
./configure --with-cc-opt='-O2' \
--prefix=/data/modules/ngx_mapping \
--http-log-path=/data/logs/access.log \
--error-log-path=/data/logs/error.log \
--lock-path=/var/lock/ngx_mapping.lock \
--pid-path=/run/ngx_mapping.pid \
--http-client-body-temp-path=/var/lib/nginx/body \
--http-fastcgi-temp-path=/var/lib/nginx/fastcgi \
--with-debug \
--with-pcre-jit \
--with-ipv6 \
--with-http_ssl_module \
--with-http_stub_status_module \
--with-http_realip_module \
--with-http_auth_request_module \
--with-http_addition_module \
--with-http_dav_module \
--with-http_geoip_module \
--with-http_gunzip_module \
--with-http_gzip_static_module \
--with-http_v2_module \
--with-http_sub_module \
--with-http_xslt_module \
--with-stream \
--with-stream_ssl_module \
--with-mail \
--with-mail_ssl_module \
--with-threads