
/*
 * Copyright (C) YoungJoo Kim (vozlt)
 */


#include "ngx_http_vhost_traffic_status_module.h"
#include "ngx_http_vhost_traffic_status_shm.h"
#include "ngx_http_vhost_traffic_status_display_prometheus.h"
#include "ngx_http_vhost_traffic_status_display.h"
#include "ngx_http_vhost_traffic_status_control.h"


static ngx_int_t ngx_http_vhost_traffic_status_display_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_vhost_traffic_status_display_handler_default(ngx_http_request_t *r);


static ngx_int_t
ngx_http_vhost_traffic_status_display_handler(ngx_http_request_t *r)
{
    u_char                                    *p;
    ngx_int_t                                  rc;
    ngx_http_vhost_traffic_status_ctx_t       *ctx;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_vhost_traffic_status_module);

    if (!ctx->enable) {
        return NGX_HTTP_NOT_IMPLEMENTED;
    }

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }


    p = (u_char *) ngx_strlchr(r->uri.data, r->uri.data + r->uri.len, '/');

    if (p) {
        p = (u_char *) ngx_strlchr(p + 1, r->uri.data + r->uri.len, '/');
    }

    /* default processing handler */
    rc = ngx_http_vhost_traffic_status_display_handler_default(r);

    return rc;
}

static ngx_int_t
ngx_http_vhost_traffic_status_display_handler_default(ngx_http_request_t *r)
{
    ngx_str_t                                  uri, type;
    ngx_int_t                                  size, rc;
    ngx_buf_t                                 *b;
    ngx_chain_t                                out;
    ngx_slab_pool_t                           *shpool;
    ngx_http_vhost_traffic_status_ctx_t       *ctx;
    ngx_http_vhost_traffic_status_loc_conf_t  *vtscf;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_vhost_traffic_status_module);

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_vhost_traffic_status_module);

    if (!ctx->enable) {
        return NGX_HTTP_NOT_IMPLEMENTED;
    }

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    uri = r->uri;

    if (uri.len == 1) {
        if (ngx_strncmp(uri.data, "/", 1) == 0) {
            uri.len = 0;
        }
    }

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_set(&type, "text/plain");

    r->headers_out.content_type_len = type.len;
    r->headers_out.content_type = type;

    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;

        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    size = ngx_http_vhost_traffic_status_display_get_size(r);
    if (size == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "display_handler_default::display_get_size() failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "display_handler_default::ngx_create_temp_buf() failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }


    shpool = (ngx_slab_pool_t *) vtscf->shm_zone->shm.addr;
    ngx_shmtx_lock(&shpool->mutex);
    b->last = ngx_http_vhost_traffic_status_display_prometheus_set(r, b->last);
    ngx_shmtx_unlock(&shpool->mutex);

    if (b->last == b->pos) {
        b->last = ngx_sprintf(b->last, "#");
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = (r == r->main) ? 1 : 0; /* if subrequest 0 else 1 */
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}


ngx_int_t
ngx_http_vhost_traffic_status_display_get_upstream_nelts(ngx_http_request_t *r)
{
    ngx_uint_t                      i, j, n;
    ngx_http_upstream_server_t     *us;
#if (NGX_HTTP_UPSTREAM_ZONE)
    ngx_http_upstream_rr_peer_t    *peer;
    ngx_http_upstream_rr_peers_t   *peers;
#endif
    ngx_http_upstream_srv_conf_t   *uscf, **uscfp;
    ngx_http_upstream_main_conf_t  *umcf;

    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);
    uscfp = umcf->upstreams.elts;

    for (i = 0, j = 0, n = 0; i < umcf->upstreams.nelts; i++) {

        uscf = uscfp[i];

        /* groups */
        if (uscf->servers && !uscf->port) {
            us = uscf->servers->elts;

#if (NGX_HTTP_UPSTREAM_ZONE)
            if (uscf->shm_zone == NULL) {
                goto not_supported;
            }

            peers = uscf->peer.data;

            ngx_http_upstream_rr_peers_rlock(peers);

            for (peer = peers->peer; peer; peer = peer->next) {
                n++;
            }

            ngx_http_upstream_rr_peers_unlock(peers);

not_supported:

#endif

            for (j = 0; j < uscf->servers->nelts; j++) {
                n += us[j].naddrs;
            }
        }
    }

    return n;
}


ngx_int_t
ngx_http_vhost_traffic_status_display_get_size(ngx_http_request_t *r)
{
    ngx_uint_t                                 size, un;
    ngx_slab_pool_t                           *shpool;
    ngx_http_vhost_traffic_status_loc_conf_t  *vtscf;
    ngx_http_vhost_traffic_status_shm_info_t  *shm_info;

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_vhost_traffic_status_module);
    shpool = (ngx_slab_pool_t *) vtscf->shm_zone->shm.addr;

    shm_info = ngx_pcalloc(r->pool, sizeof(ngx_http_vhost_traffic_status_shm_info_t));
    if (shm_info == NULL) {
        return NGX_ERROR;
    }

    /* Caveat: Do not use duplicate ngx_shmtx_lock() before this function. */
    ngx_shmtx_lock(&shpool->mutex);

    ngx_http_vhost_traffic_status_shm_info(r, shm_info);

    ngx_shmtx_unlock(&shpool->mutex);

    /* allocate memory for the upstream groups even if upstream node not exists */
    un = shm_info->used_node
         + (ngx_uint_t) ngx_http_vhost_traffic_status_display_get_upstream_nelts(r);


    size = sizeof(ngx_http_vhost_traffic_status_node_t) / NGX_PTR_SIZE
            * NGX_ATOMIC_T_LEN * un  /* values size */
            + (un * 1024)            /* names  size */
            + 4096;                  /* main   size */

    if (size <= 0) {
        size = shm_info->max_size;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "vts::display_get_size(): size[%ui] used_size[%ui], used_node[%ui]",
                   size, shm_info->used_size, shm_info->used_node);

    return size;
}


u_char *
ngx_http_vhost_traffic_status_display_get_time_queue(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_time_queue_t *q,
    ngx_uint_t offset)
{
    u_char     *p, *s;
    ngx_int_t   i;

    if (q->front == q->rear) {
        return (u_char *) "";
    }

    p = ngx_pcalloc(r->pool, q->len * NGX_INT_T_LEN);
    if (p == NULL) {
        return (u_char *) "";
    }

    s = p;

    for (i = q->front; i != q->rear; i = (i + 1) % q->len) {
        s = ngx_sprintf(s, "%M,", *((ngx_msec_t *) ((char *) &(q->times[i]) + offset)));
    }

    if (s > p) {
       *(s - 1) = '\0';
    }

    return p;
}


u_char *
ngx_http_vhost_traffic_status_display_get_time_queue_times(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_time_queue_t *q)
{
    return ngx_http_vhost_traffic_status_display_get_time_queue(r, q,
               offsetof(ngx_http_vhost_traffic_status_node_time_t, time));
}


u_char *
ngx_http_vhost_traffic_status_display_get_time_queue_msecs(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_time_queue_t *q)
{
    return ngx_http_vhost_traffic_status_display_get_time_queue(r, q,
               offsetof(ngx_http_vhost_traffic_status_node_time_t, msec));
}

    
u_char *
ngx_http_vhost_traffic_status_display_get_histogram_bucket(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_histogram_bucket_t *b,
    ngx_uint_t offset,
    const char *fmt)
{
    char        *dst;
    u_char      *p, *s;
    ngx_uint_t   i, n;

    n = b->len;

    if (n == 0) {
        return (u_char *) "";
    }

    p = ngx_pcalloc(r->pool, n * NGX_INT_T_LEN);
    if (p == NULL) {
        return (u_char *) "";
    }

    s = p;

    for (i = 0; i < n; i++) {
        dst = (char *) &(b->buckets[i]) + offset;

        if (ngx_strncmp(fmt, "%M", 2) == 0) {
            s = ngx_sprintf(s, fmt, *((ngx_msec_t *) dst));

        } else if (ngx_strncmp(fmt, "%uA", 3) == 0) {
            s = ngx_sprintf(s, fmt, *((ngx_atomic_uint_t *) dst));
        }
    }

    if (s > p) {
       *(s - 1) = '\0';
    }

    return p;
}


u_char *
ngx_http_vhost_traffic_status_display_get_histogram_bucket_msecs(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_histogram_bucket_t *b)
{
    return ngx_http_vhost_traffic_status_display_get_histogram_bucket(r, b,
               offsetof(ngx_http_vhost_traffic_status_node_histogram_t, msec), "%M,");
}


u_char *
ngx_http_vhost_traffic_status_display_get_histogram_bucket_counters(
    ngx_http_request_t *r,
    ngx_http_vhost_traffic_status_node_histogram_bucket_t *b)
{
    return ngx_http_vhost_traffic_status_display_get_histogram_bucket(r, b,
               offsetof(ngx_http_vhost_traffic_status_node_histogram_t, counter), "%uA,");
}


char *
ngx_http_vhost_traffic_status_display(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_vhost_traffic_status_display_handler;

    return NGX_CONF_OK;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */
