/*
 * Copyright (C) 2021 "IoT.bzh"
 * Author "Fulup Ar Foll" <fulup@iot.bzh>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at https://opensource.org/licenses/MIT.
 * $RP_END_LICENSE$
 *
 * Examples:
 *  GET  httpSendGet(oidc->httpPool, "https://example.com", idp->headers, NULL|token, NULL|opts, callback, ctx);
 *  POST httpSendPost(oidc->httpPool, url, idp->headers, NULL|token, NULL|opts, (void*)post,datalen, callback, ctx);
 */

#pragma once

#include <curl/curl.h>
#include <sys/types.h>
#include <stdint.h>

#define MAGIC_HTTP_RQT 951357
#define MAGIC_HTTP_POOL 583498
#define DFLT_HEADER_MAX_LEN 1024
#define HTTP_DFLT_AGENT "sec-gate-oidc/1.0"


typedef struct httpPoolS httpPoolT;

typedef enum
{
    HTTP_HANDLE_FREE,
    HTTP_HANDLE_KEEP,
} httpRqtActionT;

typedef struct
{
    const char *tag;
    const char *value;
} httpKeyValT;

typedef void (*httpFreeCtxCbT)(void *userData);

// curl options
typedef struct
{
    const char *username;
    const char *password;
    const char *bearer;
    long timeout;
    const long sslchk;
    const long verbose;
    const long maxsz;
    const long speedlimit;
    const long speedlow;
    const long follow;
    const long maxredir;
    const char *proxy;
    const char *cainfo;
    const char *sslcert;
    const char *sslkey;
    const char *tostr;
    const char *agent;
    const httpKeyValT *headers;
    const httpFreeCtxCbT freeCtx;
} httpOptsT;

typedef struct httpRqtS httpRqtT;
typedef httpRqtActionT (*httpRqtCbT)(httpRqtT *httpRqt);

// http request handle
typedef struct httpRqtS
{
    int magic;
    int verbose;
    char *body;
    char *headers;
    char *ctype;
    long length;
    long hdrLen;
    long bodyLen;
    long status;
    char error[CURL_ERROR_SIZE];
    void *easy;
    struct timespec startTime;
    struct timespec stopTime;
    uint64_t msTime;
    void *userData;
    httpRqtCbT callback;
    httpFreeCtxCbT freeCtx;
} httpRqtT;

// mainloop glue API interface
typedef void *(*evtMainLoopCbT)();
typedef int (*multiTimerCbT)(httpPoolT *httpPool, long timeout);
typedef int (*multiSocketCbT)(httpPoolT *httpPool, CURL *easy, int sock, int action, void *sockp);
typedef int (*evtRunLoopCbT)(httpPoolT *httpPool, long seconds);

// glue callbacks handle
typedef struct
{
    evtMainLoopCbT evtMainLoop;
    evtRunLoopCbT evtRunLoop;
    multiTimerCbT multiTimer;
    multiSocketCbT multiSocket;
} httpCallbacksT;

// multi-pool handle
typedef struct httpPoolS
{
    int magic;
    int verbose;
    CURLM *multi;
    void *evtLoop;
    void *evtTimer;
    httpCallbacksT *callback;
} httpPoolT;

// glue proto to get mainloop callbacks
httpCallbacksT *glueGetCbs(void);

// API to build and lauch request (if httpPoolT==NULL then run synchronously)
int httpBuildQuery(const char *uid, char *response, size_t maxlen, const char *prefix, const char *url, httpKeyValT *query);
int httpSendPost(httpPoolT *pool, const char *url, const httpOptsT *opts, httpKeyValT *tokens, void *databuf, long datalen, httpRqtCbT callback, void *ctx);
int httpSendGet(httpPoolT *pool, const char *url, const httpOptsT *opts, httpKeyValT *tokens, httpRqtCbT callback, void *ctx);

// init curl multi pool with an abstract mainloop and corresponding callbacks
httpPoolT *httpCreatePool(void *evtLoop, httpCallbacksT *mainLoopCbs, int verbose);

// curl action callback to be called from glue layer
int httpOnSocketCB(httpPoolT *httpPool, int sock, int action);
int httpOnTimerCB(httpPoolT *httpPool);
char * httpEncode64 (const char* inputData, size_t inputLen);
char * httpDecode64 (const char* inputData, size_t inputLen, int url);
