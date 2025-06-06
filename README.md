Nginx virtual host traffic status module
==========

[![CI](https://github.com/vozlt/nginx-module-vts/actions/workflows/ci.yml/badge.svg)](https://github.com/vozlt/nginx-module-vts/actions/workflows/ci.yml)
[![License](http://img.shields.io/badge/license-BSD-brightgreen.svg)](https://github.com/vozlt/nginx-module-vts/blob/master/LICENSE)

Nginx virtual host traffic status module

Table of Contents
=================

* [Version](#version)
* [Test](#test)
* [Dependencies](#dependencies)
* [Compatibility](#compatibility)
* [Screenshots](#screenshots)
* [Installation](#installation)
* [Synopsis](#synopsis)
* [Description](#description)
* [Calculations and Intervals](#calculations-and-intervals)
* [Set](#set)
* [Variables](#variables)
* [Limit](#limit)
  * [To limit traffic for server](#to-limit-traffic-for-server)
  * [To limit traffic for filter](#to-limit-traffic-for-filter)
  * [To limit traffic for upstream](#to-limit-traffic-for-upstream)
* [Use cases](#use-cases)
  * [To calculate traffic for individual country using GeoIP](#to-calculate-traffic-for-individual-country-using-geoip)
  * [To calculate traffic for individual storage volume](#to-calculate-traffic-for-individual-storage-volume)
  * [To calculate traffic for individual user agent](#to-calculate-traffic-for-individual-user-agent)
  * [To calculate traffic for detailed http status code](#to-calculate-traffic-for-detailed-http-status-code)
  * [To calculate traffic for dynamic dns](#to-calculate-traffic-for-dynamic-dns)
  * [To calculate traffic except for status page](#to-calculate-traffic-except-for-status-page)
  * [To maintain statistics data permanently](#to-maintain-statistics-data-permanently)
* [Customizing](#customizing)
  * [To customize after the module installed](#to-customize-after-the-module-installed)
  * [To customize before the module installed](#to-customize-before-the-module-installed)
* [Directives](#directives)
  * [vhost_traffic_status](#vhost_traffic_status)
  * [vhost_traffic_status_zone](#vhost_traffic_status_zone)
  * [vhost_traffic_status_display](#vhost_traffic_status_display)
  * [vhost_traffic_status_filter](#vhost_traffic_status_filter)
  * [vhost_traffic_status_filter_by_host](#vhost_traffic_status_filter_by_host)
  * [vhost_traffic_status_filter_by_set_key](#vhost_traffic_status_filter_by_set_key)
  * [vhost_traffic_status_filter_check_duplicate](#vhost_traffic_status_filter_check_duplicate)
  * [vhost_traffic_status_filter_max_node](#vhost_traffic_status_filter_max_node)
  * [vhost_traffic_status_limit](#vhost_traffic_status_limit)
  * [vhost_traffic_status_limit_traffic](#vhost_traffic_status_limit_traffic)
  * [vhost_traffic_status_limit_traffic_by_set_key](#vhost_traffic_status_limit_traffic_by_set_key)
  * [vhost_traffic_status_limit_check_duplicate](#vhost_traffic_status_limit_check_duplicate)
  * [vhost_traffic_status_set_by_filter](#vhost_traffic_status_set_by_filter)
  * [vhost_traffic_status_average_method](#vhost_traffic_status_average_method)
  * [vhost_traffic_status_histogram_buckets](#vhost_traffic_status_histogram_buckets)
  * [vhost_traffic_status_bypass_limit](#vhost_traffic_status_bypass_limit)
  * [vhost_traffic_status_bypass_stats](#vhost_traffic_status_bypass_stats)
  * [vhost_traffic_status_stats_by_upstream](#vhost_traffic_status_stats_by_upstream)
* [Releases](#releases)
* [See Also](#see-also)
* [TODO](#todo)
* [Author](#author)

## Version

![GitHub Release](https://img.shields.io/github/v/release/vozlt/nginx-module-vts?display_name=tag&sort=semver)

See the [GitHub Releases](https://github.com/vozlt/nginx-module-vts/releases) for the latest tagged release.

## Test
Run `sudo prove -r t` after you have installed this module. The `sudo` is required because
the test requires Nginx to listen on port 80.

## Dependencies
* [nginx](http://nginx.org)

## Compatibility
* Nginx
  * 1.27.x (last tested: 1.27.3)
  * 1.22.x (last tested: 1.22.0)
  * 1.19.x (last tested: 1.19.6)
  * 1.18.x (last tested: 1.18.0)
  * 1.16.x (last tested: 1.15.1)
  * 1.15.x (last tested: 1.15.0)
  * 1.14.x (last tested: 1.14.0)
  * 1.13.x (last tested: 1.13.12)
  * 1.12.x (last tested: 1.12.2)
  * 1.11.x (last tested: 1.11.10)
  * 1.10.x (last tested: 1.10.3)
  * 1.8.x (last tested: 1.8.0)
  * 1.6.x (last tested: 1.6.3)
  * 1.4.x (last tested: 1.4.7)

Earlier versions is not tested.


## Installation

1. Clone the git repository.

  ```
  shell> git clone git://github.com/vozlt/nginx-module-vts.git
  ```

2. Add the module to the build configuration by adding
  `--add-module=/path/to/nginx-module-vts`

3. Build the nginx binary.

4. Install the nginx binary.

### Installtion with Profile-Guided Optimization

It can be built with Profile-Guided Optimization (PGO) using gcc `fprofile` options. The detail of the PGO mechanisms has refer to the section 7.4 of [this paper](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf).
Here is an example of the process to make a PGO supported binary. Please use at your own risk.

1. Compile with fprofile-generate.

  ```
  shell> pwd
  /somewhere/nginx
  shell> CC=gcc ./auto/configure --with-cc-opt='-fprofile-generate -fprofile-dir=./objs' --with-ld-opt='-lgcov' --add-module=/somewhere/nginx-module-vts
  shell> make
  ```

2. Execute this module tests.

  ```
  shell> pwd
  /somewhere/nginx-module-vts
  shell> sudo PATH=/somewhere/nginx/objs:$PATH prove -r t/000.display_html.t
  ...(during runtime it records coverage data into .gcda files)
  ```

3. Recompile with fprofile-use

  ```
  shell> pwd
  /somewhere/nginx
  shell> CC=gcc ./auto/configure --with-cc-opt='-fprofile-use -fprofile-dir=/somewhere/nginx-module-vts/objs' --with-ld-opt='-lgcov' --add-module=/somewhere/nginx-module-vts
  shell> make
  ```

## Synopsis

```Nginx
http {
    vhost_traffic_status_zone;

    ...

    server {

        ...

        location /metrics {
            vhost_traffic_status_display;
        }
    }
}
```

## Description
This is an Nginx module that provides access to virtual host status information.
It contains the current status such as servers, upstreams, caches.
This is similar to the live activity monitoring of nginx plus.
The built-in html is also taken from the demo page of old version.

First of all, the directive `vhost_traffic_status_zone` is required,
and then if the directive `vhost_traffic_status_display` is set, can be access to as follows:

* /metrics
  * If you request `/metrics`, will respond with a [prometheus](https://prometheus.io) document containing the current activity data.


## Calculations and Intervals

### Averages

All averages are currently calculated as [AMM](https://en.wikipedia.org/wiki/Arithmetic_mean)(Arithmetic Mean) over the last [64](https://github.com/vozlt/nginx-module-vts/blob/master/src/ngx_http_vhost_traffic_status_node.h#L11) values.


## Set
It can get the status values in nginx configuration separately using `vhost_traffic_status_set_by_filter` directive.
It can acquire almost all status values and the obtained value is stored in user-defined-variable which is first argument.

* Directive Syntax
  * **vhost_traffic_status_set_by_filter** *$variable* *group*/*zone*/*name*

```Nginx
http {

    geoip_country /usr/share/GeoIP/GeoIP.dat;

    vhost_traffic_status_zone;
    vhost_traffic_status_filter_by_set_key $geoip_country_code country::*;

    ...
    upstream backend {
        10.10.10.11:80;
        10.10.10.12:80;
    }

    server {

        server_name example.org;

        ...

        vhost_traffic_status_filter_by_set_key $geoip_country_code country::$server_name;

        vhost_traffic_status_set_by_filter $requestCounter server/example.org/requestCounter;
        vhost_traffic_status_set_by_filter $requestCounterKR filter/country::example.org@KR/requestCounter;

        location /backend {
            vhost_traffic_status_set_by_filter $requestCounterB1 upstream@group/backend@10.10.10.11:80/requestCounter;
            proxy_pass http://backend;
        }
    }
}
```

The above settings are as follows:

* $requestCounter
  * serverZones -> example.org -> requestCounter
* $requestCounterKR
  * filterZones -> country::example.org -> KR -> requestCounter
* $requestCounterB1
  * upstreamZones -> backend -> 10.0.10.11:80 -> requestCounter

Please see the [vhost_traffic_status_set_by_filter](#vhost_traffic_status_set_by_filter) directive for detailed usage.


## Variables
The following embedded variables are provided:

* **$vts_request_counter**
  * The total number of client requests received from clients.
* **$vts_in_bytes**
  * The total number of bytes received from clients.
* **$vts_out_bytes**
  * The total number of bytes sent to clients.
* **$vts_1xx_counter**
  * The number of responses with status codes 1xx.
* **$vts_2xx_counter**
  * The number of responses with status codes 2xx.
* **$vts_3xx_counter**
  * The number of responses with status codes 3xx.
* **$vts_4xx_counter**
  * The number of responses with status codes 4xx.
* **$vts_5xx_counter**
  * The number of responses with status codes 5xx.
* **$vts_cache_miss_counter**
  * The number of cache miss.
* **$vts_cache_bypass_counter**
  * The number of cache bypass.
* **$vts_cache_expired_counter**
  * The number of cache expired.
* **$vts_cache_stale_counter**
  * The number of cache stale.
* **$vts_cache_updating_counter**
  * The number of cache updating.
* **$vts_cache_revalidated_counter**
  * The number of cache revalidated.
* **$vts_cache_hit_counter**
  * The number of cache hit.
* **$vts_cache_scarce_counter**
  * The number of cache scare.
* **$vts_request_time_counter**
  * The number of accumulated request processing time.
* **$vts_request_time**
  * The average of request processing times.

## Limit

It is able to limit total traffic per each host by using the directive
[`vhost_traffic_status_limit_traffic`](#vhost_traffic_status_limit_traffic).
It also is able to limit all traffic by using the directive
[`vhost_traffic_status_limit_traffic_by_set_key`](#vhost_traffic_status_limit_traffic_by_set_key).
When the limit is exceeded, the server will return the 503
(Service Temporarily Unavailable) error in reply to a request. 
The return code can be changeable.

### To limit traffic for server
```Nginx
http {

    vhost_traffic_status_zone;

    ...

    server {

        server_name *.example.org;

        vhost_traffic_status_limit_traffic in:64G;
        vhost_traffic_status_limit_traffic out:1024G;

        ...
    }
}
```

* Limit in/out total traffic on the `*.example.org` to 64G and 1024G respectively.
It works individually per each domain if `vhost_traffic_status_filter_by_host` directive is enabled.

### To limit traffic for filter
```Nginx
http {
    geoip_country /usr/share/GeoIP/GeoIP.dat;

    vhost_traffic_status_zone;

    ...

    server {

        server_name example.org;

        vhost_traffic_status_filter_by_set_key $geoip_country_code country::$server_name;
        vhost_traffic_status_limit_traffic_by_set_key FG@country::$server_name@US out:1024G;
        vhost_traffic_status_limit_traffic_by_set_key FG@country::$server_name@CN out:2048G;

        ...

    }
}

```

* Limit total traffic of going into US and CN on the `example.org` to 1024G and 2048G respectively.

### To limit traffic for upstream
```Nginx
http {

    vhost_traffic_status_zone;

    ...

    upstream backend {
        server 10.10.10.17:80;
        server 10.10.10.18:80;
    }

    server {

        server_name example.org;

        location /backend {
            vhost_traffic_status_limit_traffic_by_set_key UG@backend@10.10.10.17:80 in:512G;
            vhost_traffic_status_limit_traffic_by_set_key UG@backend@10.10.10.18:80 in:1024G;
            proxy_pass http://backend;
        }

        ...

    }
}

```

* Limit total traffic of going into upstream backend on the `example.org` to 512G and 1024G per each peer.

`Caveats:` Traffic is the cumulative transfer or counter, not a bandwidth.

## Use cases

It is able to calculate the user defined individual stats by using the directive `vhost_traffic_status_filter_by_set_key`.

### To calculate traffic for individual country using GeoIP
```Nginx
http {
    geoip_country /usr/share/GeoIP/GeoIP.dat;

    vhost_traffic_status_zone;
    vhost_traffic_status_filter_by_set_key $geoip_country_code country::*;

    ...

    server {

        ...

        vhost_traffic_status_filter_by_set_key $geoip_country_code country::$server_name;

        location /metrics {
            vhost_traffic_status_display;
        }
    }
}
```

* Calculate traffic for individual country of total server groups.
* Calculate traffic for individual country of each server groups.

Basically, country flags image is built-in in HTML.
The country flags image is enabled if the `country` string is included
in group name which is second argument of `vhost_traffic_status_filter_by_set_key` directive.

### To calculate traffic for individual storage volume
```Nginx
http {
    vhost_traffic_status_zone;

    ...

    server {

        ...

        location ~ ^/storage/(.+)/.*$ {
            set $volume $1;
            vhost_traffic_status_filter_by_set_key $volume storage::$server_name;
        }

        location /metrics {
            vhost_traffic_status_display;
        }
    }
}
```

* Calculate traffic for individual storage volume matched by regular expression of location directive.

### To calculate traffic for individual user agent
```Nginx
http {
    vhost_traffic_status_zone;

    map $http_user_agent $filter_user_agent {
        default 'unknown';
        ~iPhone ios;
        ~Android android;
        ~(MSIE|Mozilla) windows;
    }

    vhost_traffic_status_filter_by_set_key $filter_user_agent agent::*;

    ...

    server {

        ...

        vhost_traffic_status_filter_by_set_key $filter_user_agent agent::$server_name;

        location /metrics {
            vhost_traffic_status_display;
        }
    }
}
```

* Calculate traffic for individual `http_user_agent`

### To calculate traffic for detailed http status code
```Nginx
http {
    vhost_traffic_status_zone;

    server {

        ...

        vhost_traffic_status_filter_by_set_key $status $server_name;

        location /metrics {
            vhost_traffic_status_display;
        }
    }
}
```

* Calculate traffic for detailed `http status code`

`Caveats:` [$status](http://nginx.org/en/docs/http/ngx_http_core_module.html#variables) variable is available in nginx-(1.3.2, 1.2.2).

### To calculate traffic for dynamic dns

If the domain has multiple DNS A records, you can calculate traffic for individual IPs
for the domain using the filter feature or a variable in proxy_pass.

```Nginx
http {
    vhost_traffic_status_zone;

    upstream backend {
        elb.example.org:80;
    }

    ...

    server {

        ...

        location /backend {
            vhost_traffic_status_filter_by_set_key $upstream_addr upstream::backend;
            proxy_pass backend;
        }
    }
}
```

* Calculate traffic for individual IPs for the domain `elb.example.org`.
If `elb.example.org` has multiple DNS A records, will be display all IPs in `filterZones`.
In the above settings, as NGINX starts up or reloads it configuration,
it queries a DNS server to resolve domain and DNS A records is cached in memory.
Therefore the DNS A records are not changed in memory even if
DNS A records are chagned by DNS administrator unless NGINX re-starts up or reloads.

```Nginx
http {
    vhost_traffic_status_zone;

    resolver 10.10.10.53 valid=10s

    ...

    server {

        ...

        location /backend {
            set $backend_server elb.example.org;
            proxy_pass http://$backend_server;
        }
    }
}
```

* Calculate traffic for individual IPs for the domain `elb.example.org`.
If `elb.example.org`'s DNS A record is changed,
will be display both the old IP and the new IP in `::nogroups`.
Unlike the first upstream group setting, the second setting works well
even if DNS A records are chagned by DNS administrator.

`Caveats:` Please more details about NGINX DNS see the
[dns-service-discovery-nginx-plus](https://www.nginx.com/blog/dns-service-discovery-nginx-plus).

### To calculate traffic except for status page

```Nginx
http {
    vhost_traffic_status_zone;

    ...

    server {

        ...

        location /metrics {
            vhost_traffic_status_bypass_limit on;
            vhost_traffic_status_bypass_stats on;
            vhost_traffic_status_display;
        }
    }
}
```

* The `/metrics` uri is excluded from the status traffic calculation and limit feature. 
See the following directives:
  * [vhost_traffic_status_bypass_limit](#vhost_traffic_status_bypass_limit)
  * [vhost_traffic_status_bypass_stats](#vhost_traffic_status_bypass_stats)



## Directives


### vhost_traffic_status

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status** \<on\|off\> |
| **Default** | off |
| **Context** | http, server, location |

`Description:` Enables or disables the module working.
If you set `vhost_traffic_status_zone` directive, is automatically enabled.

### vhost_traffic_status_zone

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_zone** [shared:*name:size*] |
| **Default** | shared:vhost_traffic_status:1m |
| **Context** | http |

`Description:` Sets parameters for a shared memory zone that will keep states for various keys.
The cache is shared between all worker processes.
In most cases, the shared memory size used by nginx-module-vts does not increase much.
The shared memory size is increased pretty when using `vhost_traffic_status_filter_by_set_key`
directive but if filter's keys are fixed(*eg. the total number of the country code is about 240*)
it does not continuously increase.

If you use `vhost_traffic_status_filter_by_set_key` directive, set it as follows:

* Set to more than 32M shared memory size by default.
(`vhost_traffic_status_zone shared:vhost_traffic_status:32m`)
* If the message(*`"ngx_slab_alloc() failed: no memory in vhost_traffic_status_zone"`*)
printed in error_log, increase to more than (usedSize * 2).


### vhost_traffic_status_display

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_display** |
| **Default** | - |
| **Context** | http, server, location |

`Description:` Enables or disables the module display handler.

### vhost_traffic_status_filter

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_filter** \<on\|off\> |
| **Default** | on |
| **Context** | http, server, location |

`Description:` Enables or disables the filter features.

### vhost_traffic_status_filter_by_host

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_filter_by_host** \<on\|off\> |
| **Default** | off |
| **Context** | http, server, location |

`Description:` Enables or disables the keys by Host header field.
If you set `on` and nginx's server_name directive set several or wildcard name starting with an asterisk, e.g. “*.example.org”
and requested to server with hostname such as (a|b|c).example.org or *.example.org
then json serverZones is printed as follows:

```Nginx
server {
  server_name *.example.org;
  vhost_traffic_status_filter_by_host on;

  ...

}
```

```Json
  ...
  "serverZones": {
      "a.example.org": {
      ...
      },
      "b.example.org": {
      ...
      },
      "c.example.org": {
      ...
      }
      ...
   },
   ...
```

It provides the same function that set `vhost_traffic_status_filter_by_set_key $host`.

### vhost_traffic_status_filter_by_set_key

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_filter_by_set_key** *key* [*name*] |
| **Default** | - |
| **Context** | http, server, location |

`Description:` Enables the keys by user defined variable.
The *key* is a key string to calculate traffic.
The *name* is a group string to calculate traffic.
The *key* and *name* can contain variables such as $host, $server_name.
The *name*'s group belongs to `filterZones` if specified.
The *key*'s group belongs to `serverZones` if not specified second argument *name*.
The example with geoip module is as follows:

```Nginx
server {
  server_name example.org;
  vhost_traffic_status_filter_by_set_key $geoip_country_code country::$server_name;

  ...

}
```

```Json
  ...
  "serverZones": {
  ...
  },
  "filterZones": {
      "country::example.org": {
          "KR": {
              "requestCounter":...,
              "inBytes":...,
              "outBytes":...,
              "responses":{
                  "1xx":...,
                  "2xx":...,
                  "3xx":...,
                  "4xx":...,
                  "5xx":...,
                  "miss":...,
                  "bypass":...,
                  "expired":...,
                  "stale":...,
                  "updating":...,
                  "revalidated":...,
                  "hit":...,
                  "scarce":...
              },
              "requestMsecCounter":...,
              "requestMsec":...,
              "requestMsecs":{
                  "times":[...],
                  "msecs":[...]
              },
          },
          "US": {
          ...
          },
          ...
      },
      ...
  },
  ...

```

### vhost_traffic_status_filter_check_duplicate

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_filter_check_duplicate** \<on\|off\> |
| **Default** | on |
| **Context** | http, server, location |

`Description:` Enables or disables the deduplication of vhost_traffic_status_filter_by_set_key.
It is processed only one of duplicate values(`key` + `name`) in each directives(http, server, location) if this option is enabled.

### vhost_traffic_status_filter_max_node

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_filter_max_node** *number* [*string* ...] |
| **Default** | 0 |
| **Context** | http |

`Description:` Enables the limit of filter size using the specified *number* and *string* values.
If the *number* is exceeded, the existing nodes are deleted by the [LRU](https://en.wikipedia.org/wiki/Cache_replacement_policies#LRU) algorithm.
The *number* argument is the size of the node that will be limited.
The default value `0` does not limit filters.
The one node is an object in `filterZones` in JSON document.
The *string* arguments are the matching string values for the group string value set by `vhost_traffic_status_filter_by_set_key` directive. 
Even if only the first part matches, matching is successful like the regular expression `/^string.*/`.
By default, If you do not set *string* arguments then it applied for all filters.


For examples:

`$ vi nginx.conf`

```Nginx
http {

    geoip_country /usr/share/GeoIP/GeoIP.dat;

    vhost_traffic_status_zone;

    # The all filters are limited to a total of 16 nodes.
    # vhost_traffic_status_filter_max_node 16

    # The `/^uris.*/` and `/^client::ports.*/` group string patterns are limited to a total of 64 nodes.
    vhost_traffic_status_filter_max_node 16 uris client::ports;

    ...

    server {

        server_name example.org;

        ...

        vhost_traffic_status_filter_by_set_key $uri uris::$server_name;
        vhost_traffic_status_filter_by_set_key $remote_port client::ports::$server_name;
        vhost_traffic_status_filter_by_set_key $geoip_country_code country::$server_name;

    }
}
```

`$ for i in {0..1000}; do curl -H 'Host: example.org' -i "http://localhost:80/test$i"; done`

![screenshot-vts-filter-max-node](https://user-images.githubusercontent.com/3648408/41475027-96c96136-70f8-11e8-8dd6-ed1825d7b216.png)

In the above example, the `/^uris.*/` and `/^client::ports.*/` group string patterns are limited to a total of 16 nodes.
The other filters like `country::.*` are not limited.

### vhost_traffic_status_limit

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_limit** \<on\|off\> |
| **Default** | on |
| **Context** | http, server, location |

`Description:` Enables or disables the limit features.

### vhost_traffic_status_limit_traffic

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_limit_traffic** *member*:*size* [*code*] |
| **Default** | - |
| **Context** | http, server, location |

`Description:` Enables the traffic limit for specified *member*.
The *member* is a member string to limit traffic.
The *size* is a size(k/m/g) to limit traffic.
The *code* is a code to return in response to rejected requests.(Default: 503)

The available *`member`* strings are as follows:
* **request**
  * The total number of client requests received from clients.
* **in**
  * The total number of bytes received from clients.
* **out**
  * The total number of bytes sent to clients.
* **1xx**
  * The number of responses with status codes 1xx.
* **2xx**
  * The number of responses with status codes 2xx.
* **3xx**
  * The number of responses with status codes 3xx.
* **4xx**
  * The number of responses with status codes 4xx.
* **5xx**
  * The number of responses with status codes 5xx.
* **cache_miss**
  * The number of cache miss.
* **cache_bypass**
  * The number of cache bypass.
* **cache_expired**
  * The number of cache expired.
* **cache_stale**
  * The number of cache stale.
* **cache_updating**
  * The number of cache updating.
* **cache_revalidated**
  * The number of cache revalidated.
* **cache_hit**
  * The number of cache hit.
* **cache_scarce**
  * The number of cache scare.

### vhost_traffic_status_limit_traffic_by_set_key

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_limit_traffic_by_set_key** *key* *member*:*size* [*code*] |
| **Default** | - |
| **Context** | http, server, location|

`Description:` Enables the traffic limit for specified *key* and *member*.
The *key* is a key string to limit traffic.
The *member* is a member string to limit traffic.
The *size* is a size(k/m/g) to limit traffic.
The *code* is a code to return in response to rejected requests.(Default: 503)


The *`key`* syntax is as follows:
* *`group`*@[*`subgroup`*@]*`name`*

The available *`group`* strings are as follows:
* **NO**
  * The group of server.
* **UA**
  * The group of upstream alone.
* **UG**
  * The group of upstream group.(use *`subgroup`*)
* **CC**
  * The group of cache.
* **FG**
  * The group of filter.(use *`subgroup`*)

The available *`member`* strings are as follows:
* **request**
  * The total number of client requests received from clients.
* **in**
  * The total number of bytes received from clients.
* **out**
  * The total number of bytes sent to clients.
* **1xx**
  * The number of responses with status codes 1xx.
* **2xx**
  * The number of responses with status codes 2xx.
* **3xx**
  * The number of responses with status codes 3xx.
* **4xx**
  * The number of responses with status codes 4xx.
* **5xx**
  * The number of responses with status codes 5xx.
* **cache_miss**
  * The number of cache miss.
* **cache_bypass**
  * The number of cache bypass.
* **cache_expired**
  * The number of cache expired.
* **cache_stale**
  * The number of cache stale.
* **cache_updating**
  * The number of cache updating.
* **cache_revalidated**
  * The number of cache revalidated.
* **cache_hit**
  * The number of cache hit.
* **cache_scarce**
  * The number of cache scare.

The *member* is the same as `vhost_traffic_status_limit_traffic` directive.

### vhost_traffic_status_limit_check_duplicate

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_limit_check_duplicate** \<on\|off\> |
| **Default** | on |
| **Context** | http, server, location |

`Description:` Enables or disables the deduplication of vhost_traffic_status_limit_by_set_key.
It is processed only one of duplicate values(`member` | `key` + `member`)
in each directives(http, server, location) if this option is enabled.

### vhost_traffic_status_set_by_filter

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_set_by_filter** *$variable* *group*/*zone*/*name* |
| **Default** | - |
| **Context** | http, server, location, if |

`Description:` Get the specified status value stored in shared memory.
It can acquire almost all status values and the obtained value is stored in *$variable* which is first argument.

* **group**
  * server
  * filter
  * upstream@alone
  * upstream@group
  * cache
* **zone**
  * server
    * *name*
  * filter
    * *filter_group*@*name*
  * upstream@group
    * *upstream_group*@*name*
  * upstream@alone
    * @*name*
  * cache
    * *name*
* **name**
  * requestCounter
    * The total number of client requests received from clients.
  * requestMsecCounter
    * The number of accumulated request processing time in milliseconds.
  * requestMsec
    * The average of request processing times in milliseconds.
  * responseMsecCounter
    * The number of accumulated only upstream response processing time in milliseconds.
  * responseMsec
    * The average of only upstream response processing times in milliseconds.
  * inBytes
    * The total number of bytes received from clients.
  * outBytes
    * The total number of bytes sent to clients.
  * 1xx, 2xx, 3xx, 4xx, 5xx
    * The number of responses with status codes 1xx, 2xx, 3xx, 4xx, and 5xx.
  * cacheMaxSize
    * The limit on the maximum size of the cache specified in the configuration.
  * cacheUsedSize
    * The current size of the cache.
  * cacheMiss
    * The number of cache miss.
  * cacheBypass
    * The number of cache bypass.
  * cacheExpired
    * The number of cache expired.
  * cacheStale
    * The number of cache stale.
  * cacheUpdating
    * The number of cache updating.
  * cacheRevalidated
    * The number of cache revalidated.
  * cacheHit
    * The number of cache hit.
  * cacheScarce
    * The number of cache scare.
  * weight
    * Current weight setting of the server.
  * maxFails
    * Current max_fails setting of the server.
  * failTimeout
    * Current fail_timeout setting of the server.
  * backup
    * Current backup setting of the server.(0\|1)
  * down
    * Current down setting of the server.(0\|1)

`Caveats:` The *name* is case sensitive. All return values take the integer type.

For examples:
* requestCounter in serverZones
  * **vhost_traffic_status_set_by_filter** `$requestCounter` `server/example.org/requestCounter`
* requestCounter in filterZones
  * **vhost_traffic_status_set_by_filter** `$requestCounter` `filter/country::example.org@KR/requestCounter`
* requestCounter in upstreamZones
  * **vhost_traffic_status_set_by_filter** `$requestCounter` `upstream@group/backend@10.10.10.11:80/requestCounter`
* requestCounter in upstreamZones::nogroups
  * **vhost_traffic_status_set_by_filter** `$requestCounter` `upstream@alone/10.10.10.11:80/requestCounter`
* cacheHit in cacheZones
  * **vhost_traffic_status_set_by_filter** `$cacheHit` `cache/my_cache_name/cacheHit`

### vhost_traffic_status_average_method

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_average_method** \<AMM\|WMA\> [*period*] |
| **Default** | AMM 60s |
| **Context** | http, server, location |

`Description:` Sets the method which is a formula that calculate the average of response processing times.
The *period* is an effective time of the values used for the average calculation.(Default: 60s)
If *period* set to 0, effective time is ignored.
In this case, the last average value is displayed even if there is no requests and after the elapse of time.
The corresponding values are `requestMsec` and `responseMsec` in JSON.

* **AMM**
  * The AMM is the [arithmetic mean](https://en.wikipedia.org/wiki/Arithmetic_mean).
* **WMA**
  * THE WMA is the [weighted moving average](https://en.wikipedia.org/wiki/Moving_average#Weighted_moving_average).

### vhost_traffic_status_histogram_buckets

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_histogram_buckets** *second* ... |
| **Default** | - |
| **Context** | http, server, location |

`Description:` Sets the observe buckets to be used in the histograms.
By default, if you do not set this directive, it will not work.
The *second* can be expressed in decimal places with a minimum value of 0.001(1ms).
The maximum size of the buckets is 32. If this value is insufficient for you,
change the `NGX_HTTP_VHOST_TRAFFIC_STATUS_DEFAULT_BUCKET_LEN` in the `src/ngx_http_vhost_traffic_status_node.h`

For examples:
* **vhost_traffic_status_histogram_buckets** `0.005` `0.01` `0.05` `0.1` `0.5` `1` `5` `10`
  * The observe buckets are [5ms 10ms 50ms 100ms 500ms 1s 5s 10s].
* **vhost_traffic_status_histogram_buckets** `0.005` `0.01` `0.05` `0.1`
  * The observe buckets are [5ms 10ms 50ms 100ms].

`Caveats:` By default, if you do not set this directive, the histogram statistics does not work.
by `vhost_traffic_status_histogram_buckets` directive.

### vhost_traffic_status_bypass_limit

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_bypass_limit** \<on\|off\> |
| **Default** | off |
| **Context** | http, server, location |

`Description:` Enables or disables to bypass `vhost_traffic_status_limit` directives.
The limit features is bypassed if this option is enabled.
This is mostly useful if you want to connect the status web page like `/metrics` regardless of `vhost_traffic_status_limit` directives as follows:

```Nginx
http {
    vhost_traffic_status_zone;

    ...

    server {

        ...

        location /metrics {
            vhost_traffic_status_bypass_limit on;
            vhost_traffic_status_display;
        }
    }
}
```

### vhost_traffic_status_bypass_stats

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_bypass_stats** \<on\|off\> |
| **Default** | off |
| **Context** | http, server, location |

`Description:` Enables or disables to bypass `vhost_traffic_status`.
The traffic status stats features is bypassed if this option is enabled.
In other words, it is excluded from the traffic status stats.
This is mostly useful if you want to ignore your request in status web page like `/metrics` as follows:

```Nginx
http {
    vhost_traffic_status_zone;

    ...

    server {

        ...

        location /metrics {
            vhost_traffic_status_bypass_stats on;
            vhost_traffic_status_display;
        }
    }
}
```

### vhost_traffic_status_stats_by_upstream

| -   | - |
| --- | --- |
| **Syntax**  | **vhost_traffic_status_stats_by_upstream** \<on\|off\> |
| **Default** | on  |
| **Context** | http|

`Description:` Enables or disables to stats `upstreamZone`.
The `upstreamZone` in the traffic status stats features is bypassed if this option is disabled.
In other words, it is excluded from the traffic status stats.
This is mostly useful if you want to be disable statistics collection for upstream servers to reduce CPU load.

```Nginx
http {
    vhost_traffic_status_zone;
    vhost_traffic_status_stats_by_upstream off;

    proxy_cache_path /var/cache/nginx keys_zone=zone1:1m max_size=1g inactive=24h;
    upstream backend {
       ...
    }
    ...

    server {

        ...

        location /metrics {
            vhost_traffic_status_display;
        }
        location /backend {
            proxy_cache zone1;
            proxy_pass http://backend;
        }
    }
}
```

### vhost_traffic_status_measure_status_codes

Allows tracking of specific HTTP status codes or all status codes in the Vhost Traffic Status module.


| -   | - |
| --- | --- |
| **Syntax**  | vhost_traffic_status_measure_status_codes [all] [status_code1] [status_code2] ... |
| **Default** | off |
| **Context** | http |



#### Parameters
- `status_code1, status_code2, ...`: Specific HTTP status codes to track (100-599)
- `all`: Track all HTTP status codes

#### Examples

Track specific status codes:
```nginx
vhost_traffic_status_measure_status_codes 200 404 500;
```

Track all status codes:
```nginx
vhost_traffic_status_measure_status_codes all;
```

#### Description
- By default, no specific status code tracking is enabled
- Status codes must be in ascending order
- Only valid HTTP status codes between 100 and 599 are accepted
- When using `all`, every status code will be tracked

## Releases

To cut a release, create a changelog entry PR with [git-chglog](https://github.com/git-chglog/git-chglog)

    version="v0.2.0"
    git checkout -b "cut-${version}"
    git-chglog -o CHANGELOG.md --next-tag "${version}"
    git add CHANGELOG.md
    sed -i "s/NGX_HTTP_VTS_MODULE_VERSION \".*/NGX_HTTP_VTS_MODULE_VERSION \"${version}\"/" src/ngx_http_vhost_traffic_status_module.h
    git add src/ngx_http_vhost_traffic_status_module.h
    git-chglog -t .chglog/RELNOTES.tmpl --next-tag "${version}" "${version}" | git commit -F-
    
After the PR is merged, create the new tag and release on the [GitHub Releases](https://github.com/vozlt/nginx-module-vts/releases).

## See Also
* Stream traffic status
  * [nginx-module-sts](https://github.com/vozlt/nginx-module-sts)
  * [nginx-module-stream-sts](https://github.com/vozlt/nginx-module-stream-sts)

* Prometheus
  * [nginx-vts-exporter](https://github.com/hnlq715/nginx-vts-exporter)

* System protection
  * [nginx-module-sysguard](https://github.com/vozlt/nginx-module-sysguard)

## TODO
* Add an implementation that periodically updates computed statistic in each worker processes to shared memory to reduce the contention due to locks when using ngx_shmtx_lock().

## Author
YoungJoo.Kim(김영주) [<vozltx@gmail.com>]
