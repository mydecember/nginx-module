#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char* ngx_echo_readconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void* ngx_echo_create_loc_conf(ngx_conf_t *cf);
//static char* ngx_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

typedef struct {
    ngx_str_t ecdata;
    ngx_str_t ecdata2;
    ngx_str_t ecdata3;
    // ngx_str_t     name;          /* location name */
    ngx_http_regex_t  *regex;
     //ngx_regex_t                   *split_regex;
    ngx_flag_t           enable;
} ngx_echo_loc_conf_t;

static ngx_command_t  ngx_echo_commands[] = {
    { ngx_string("cc_echo"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE3,
      ngx_echo_readconf,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_echo_loc_conf_t, ecdata),
      NULL },
      ngx_null_command
};


static ngx_http_module_t  ngx_echo_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,           /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_echo_create_loc_conf,  /* create location configuration */
    NULL//ngx_echo_merge_loc_conf /* merge location configuration */
};


ngx_module_t  ngx_module_echo = {
    NGX_MODULE_V1,
    &ngx_echo_module_ctx, /* module context */
    ngx_echo_commands,   /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_echo_handler(ngx_http_request_t *r)
{
    //printf("called:ngx_echo_handler\n");
    unsigned char buf[512];
    memset(buf,0,512);
    ngx_snprintf(buf, 512, "uri:%V\n", &(r->uri));    
    printf("%s", buf);
    memset(buf,0,512);
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t   out;

    ngx_echo_loc_conf_t  *cglcf;
    cglcf = ngx_http_get_module_loc_conf(r, ngx_module_echo);
    #if 0
        ngx_snprintf(buf, 512, "ecdata:%V\n", &(cglcf->ecdata));    
        printf("%d:%s",(int)cglcf->ecdata.len, buf);
        memset(buf,0,512);
        ngx_snprintf(buf, 512, "ecdata2:%V\n", &(cglcf->ecdata2));    
        //printf("%s", buf);
        ngx_snprintf(buf, 512, "ecdata3:%V\n", &(cglcf->ecdata3));    
        //printf("%s", buf);
    #endif

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        printf("NGX_HTTP_NOT_ALLOWED\n");
        return NGX_HTTP_NOT_ALLOWED;
    }
    if (r->headers_in.if_modified_since) {
        printf("NGX_HTTP_NOT_MODIFIED\n");
        return NGX_HTTP_NOT_MODIFIED;
    }

    if(cglcf->ecdata.data[0]=='$'){
       
       if(ngx_strncmp(cglcf->ecdata.data+1,"uri",3) == 0){

     		ngx_int_t       n;
       		int            captures[(1 + 2) * 3];

    		n = ngx_regex_exec(cglcf->regex->regex, &r->uri, captures, (1 + 2) * 3);

    	    if (n >= 0) { /* match */
    		printf("match\n");
    	    }

    	    if (n == NGX_REGEX_NO_MATCHED) {
    		return NGX_HTTP_NOT_ALLOWED;
    	    }

        } else {
            printf("not allowed\n");
            return NGX_HTTP_NOT_ALLOWED;
        }
    } else  {
        return NGX_OK;
    }   

    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";


    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = cglcf->ecdata3.len;

    if (r->method == NGX_HTTP_HEAD) {
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;


    b->pos  = cglcf->ecdata3.data;
    b->last = cglcf->ecdata3.data+(cglcf->ecdata3.len);

    b->memory = 1;
    b->last_buf = 1;
    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    return ngx_http_output_filter(r, &out);
}
static char *
ngx_echo_readconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    //printf("called:ngx_echo_readconf\n");
    //unsigned char buf[512];
    ngx_echo_loc_conf_t *ccf = conf;
    ngx_str_t *value;
    value = cf->args->elts;
    /*ngx_uint_t i = 0;
        printf("%d\n",(int)cf->args->nelts);
        for(i=0; i < cf->args->nelts; ++i) {
            ngx_snprintf(buf, 512, "arg%d:%V\n", i,&value[i]);
            printf("%s",buf);        
    }*/
    ccf->ecdata  = value[1];
    ccf->ecdata2 = value[2];
    ccf->ecdata3 = value[3];

    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_echo_handler; 
    /*pcre*/
    ngx_regex_compile_t  rc;
    u_char               errstr[NGX_MAX_CONF_ERRSTR];

    ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

    rc.pattern = value[2];//*regex;
    rc.err.len = NGX_MAX_CONF_ERRSTR;
    rc.err.data = errstr;

	/*#if (NGX_HAVE_CASELESS_FILESYSTEM)
	    rc.options = NGX_REGEX_CASELESS;
	#else
	    rc.options = caseless ? NGX_REGEX_CASELESS : 0;
	#endif*/

    ccf->regex = ngx_http_regex_compile(cf, &rc);
    if (ccf->regex == NULL) {
        return NGX_CONF_ERROR;
    } 

    //ngx_conf_set_str_slot(cf,cmd,conf);
    return NGX_CONF_OK;
}


static void *
ngx_echo_create_loc_conf(ngx_conf_t *cf)
{
    printf("called:ngx_echo_create_loc_conf\n");
    #if 0
    unsigned char buf[512];
    ngx_str_t *value;
    value = cf->args->elts;
    ngx_uint_t i = 0;
    for(i=0; i < cf->args->nelts; ++i) {
        ngx_snprintf(buf, 512, "arg%d:%V\n", i,&value[i]);
        printf("%s",buf);
    }
    #endif
    
    ngx_echo_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_echo_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->ecdata.len   = 0;
    conf->ecdata.data  = NULL;
    conf->ecdata2.len  = 0;
    conf->ecdata2.data = NULL;
    conf->ecdata3.len  = 0;
    conf->ecdata3.data = NULL;
    conf->regex        = NULL;
    conf->enable       = NGX_CONF_UNSET;
    return conf;
}
#if 0
static char *
ngx_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    printf("called:ngx_echo_merge_loc_conf\n");
    ngx_echo_loc_conf_t *prev = parent;
    ngx_echo_loc_conf_t *conf = child;
    ngx_echo_loc_conf_t *lcal;
   /* if (prev->ecdata.data != NULL)
        //printf("prev");
        lcal = prev;
    else if (conf->ecdata.data != NULL)
        lcal = conf;
    else
       {
        lcal = NULL;
        printf("both NULL\n");
       } */
       lcal = conf;
    if (lcal != NULL)
    {
         unsigned char buf[512];
    ngx_snprintf(buf, 512, "ecdata:%V\n", &(lcal->ecdata)); 
    //printf("lcal->ecdata.len:%u\n",(int)(lcal->ecdata.len));
    if(lcal->ecdata.len>0)   
    printf("%s", buf);
    }

    


    ngx_conf_merge_str_value(conf->ecdata, prev->ecdata, 10);
    ngx_conf_merge_value(conf->enable, prev->enable, 0);




/**
    if(conf->enable)
        ngx_echo_init(conf);
        */
    return NGX_CONF_OK;
   
}
#endif
