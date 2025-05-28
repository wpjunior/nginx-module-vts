
/*
 * Copyright (C) YoungJoo Kim (vozlt)
 */


#include "ngx_http_vhost_traffic_status_module.h"
#include "ngx_http_vhost_traffic_status_control.h"
#include "ngx_http_vhost_traffic_status_display.h"


void
ngx_http_vhost_traffic_status_node_upstream_lookup(
    ngx_http_vhost_traffic_status_control_t *control,
    ngx_http_upstream_server_t *usn)
{
    ngx_int_t                       rc;
    ngx_str_t                       key, usg, ush;
    ngx_uint_t                      i, j;
    ngx_http_upstream_server_t     *us;
    ngx_http_upstream_srv_conf_t   *uscf, **uscfp;
    ngx_http_upstream_main_conf_t  *umcf;

    umcf = ngx_http_get_module_main_conf(control->r, ngx_http_upstream_module);
    uscfp = umcf->upstreams.elts;

    key = *control->zone;

    if (control->group == NGX_HTTP_VHOST_TRAFFIC_STATUS_UPSTREAM_UA) {

#if nginx_version > 1007001
        usn->name = key;
#endif

        usn->weight = 0;
        usn->max_fails = 0;
        usn->fail_timeout = 0;
        usn->down = 0;
        usn->backup = 0;
        control->count++;
        return;
    }

    usg = ush = key;

    rc = ngx_http_vhost_traffic_status_node_position_key(&usg, 0);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, control->r->connection->log, 0,
                      "node_upstream_lookup::node_position_key(\"%V\", 0) group not found", &usg);
        return;
    }

    rc = ngx_http_vhost_traffic_status_node_position_key(&ush, 1);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, control->r->connection->log, 0,
                      "node_upstream_lookup::node_position_key(\"%V\", 1) host not found", &ush);
        return;
    }

    for (i = 0; i < umcf->upstreams.nelts; i++) {
        uscf = uscfp[i];

        /* nogroups */
        if (uscf->servers == NULL && uscf->port != 0) {
            continue;
        }

        us = uscf->servers->elts;

        if (uscf->host.len == usg.len) {
            if (ngx_strncmp(uscf->host.data, usg.data, usg.len) == 0) {

                for (j = 0; j < uscf->servers->nelts; j++) {
                    if (us[j].addrs->name.len == ush.len) {
                        if (ngx_strncmp(us[j].addrs->name.data, ush.data, ush.len) == 0) {
                            *usn = us[j];

#if nginx_version > 1007001
                            usn->name = us[j].addrs->name;
#endif

                            control->count++;
                            break;
                        }
                    }
                }

                break;
            }
        }
    }
}


void
ngx_http_vhost_traffic_status_node_control_range_set(
    ngx_http_vhost_traffic_status_control_t *control)
{
    ngx_uint_t  state;

    if (control->group == -1) {
        state = NGX_HTTP_VHOST_TRAFFIC_STATUS_CONTROL_RANGE_ALL;

    } else {
        state = NGX_HTTP_VHOST_TRAFFIC_STATUS_CONTROL_RANGE_ZONE;

        if (control->zone->len == 0) {
            state = NGX_HTTP_VHOST_TRAFFIC_STATUS_CONTROL_RANGE_NONE;

        } else if (control->zone->len == 1) {
            if (ngx_strncmp(control->zone->data, "*", 1) == 0) {
                state = NGX_HTTP_VHOST_TRAFFIC_STATUS_CONTROL_RANGE_GROUP;
            }
        }
    }

    control->range = state;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */
