/*
 * Copyright (C) 2021 "IoT.bzh"
 * Author "Fulup Ar Foll" <fulup@iot.bzh>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at https://opensource.org/licenses/MIT.
 * $RP_END_LICENSE$
 */

#define _GNU_SOURCE

#include "curl-glue.h"

#include <errno.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

// Fulup TDB remove when Jose will have made public with afb-libafb
int wrap_base64_decode(
		const char *data,
		size_t datalen,
		uint8_t **decoded,
		size_t *decodedlen,
		int url)
{
	uint16_t u16;
	uint8_t u8, *result;
	size_t in, out, iin;
	char c;

	/* allocate enougth output */
	result = malloc(datalen);
	if (result == NULL) return -1;

	/* decode the input */
	for (iin = in = out = 0 ; in < datalen ; in++) {
		c = data[in];
		if (c != '\n' && c != '\r' && c != '=') {
			if ('A' <= c && c <= 'Z')
				u8 = (uint8_t)(c - 'A');
			else if ('a' <= c && c <= 'z')
				u8 = (uint8_t)(c - 'a' + 26);
			else if ('0' <= c && c <= '9')
				u8 = (uint8_t)(c - '0' + 52);
			else if (c == '+' || c == '-')
				u8 = (uint8_t)62;
			else if (c == '/' || c == '_')
				u8 = (uint8_t)63;
			else {
				free(result);
				return -1;
			}
			if (!iin) {
				u16 = (uint16_t)u8;
				iin = 6;
			} else {
				u16 = (uint16_t)((u16 << 6) | u8);
				iin -= 2;
				u8 = (uint8_t)(u16 >> iin);
				result[out++] = u8;
                if (out == datalen) return -1; // added Fulup
			}
		}
	}

	/* terminate */
    result[out]='\0'; // added fulup
	*decoded = realloc(result, out);
	if (out && *decoded == NULL) {
		free(result);
		return -1;
	}
	*decodedlen = out;
	return 0;
}

int wrap_base64_encode(
		const uint8_t *data,
		size_t datalen,
		char **encoded,
		size_t *encodedlen,
		int width,
		int pad,
		int url)
{
	uint16_t u16 = 0;
	uint8_t u8 = 0;
	size_t in, out, rlen, n3, r3, iout, nout;
	int iw;
	char *result, c;

	/* compute unformatted output length */
	n3 = datalen / 3;
	r3 = datalen % 3;
	nout = 4 * n3 + r3 + !!r3;

	/* deduce formatted output length */
	rlen = nout;
	if (pad)
		rlen += ((~rlen) + 1) & 3;
	if (width)
		rlen += rlen / (unsigned)width;

	/* allocate the output */
	result = malloc(rlen + 1);
	if (result == NULL) return -1;

	/* compute the formatted output */
	iw = width;
	for (in = out = iout = 0 ; iout < nout ; iout++) {
		/* get in 'u8' the 6 bits value to add */
		switch (iout & 3) {
		case 0:
			u16 = (uint16_t)data[in++];
			u8 = (uint8_t)(u16 >> 2);
			break;
		case 1:
			u16 = (uint16_t)(u16 << 8);
			if (in < datalen)
				u16 = (uint16_t)(u16 | data[in++]);
			u8 = (uint8_t)(u16 >> 4);
			break;
		case 2:
			u16 = (uint16_t)(u16 << 8);
			if (in < datalen)
				u16 = (uint16_t)(u16 | data[in++]);
			u8 = (uint8_t)(u16 >> 6);
			break;
		case 3:
			u8 = (uint8_t)u16;
			break;
		}
		u8 &= 63;

		/* encode 'u8' to the char 'c' */
		if (u8 < 52) {
			if (u8 < 26)
				c = (char)('A' + u8);
			else
				c = (char)('a' + u8 - 26);
		} else {
			if (u8 < 62)
				c = (char)('0' + u8 - 52);
			else if (u8 == 62)
				c = url ? '-' : '+';
			else
				c = url ? '_' : '/';
		}

		/* put to output with format */
		result[out++] = c;
		if (iw && !--iw) {
			result[out++] = '\n';
			iw = width;
		}
	}

	/* pad the output */
	while (out < rlen) {
		result[out++] = '=';
		if (iw && !--iw) {
			result[out++] = '\n';
			iw = width;
		}
	}

	/* terminate */
	result[out] = 0;
	*encoded = result;
	*encodedlen = rlen;
	return 0;
}


// callback might be called as many time as needed to transfert all data
static size_t httpBodyCB(void *data, size_t blkSize, size_t blkCount, void *ctx)
{
    httpRqtT *httpRqt = (httpRqtT *)ctx;
    assert(httpRqt->magic == MAGIC_HTTP_RQT);
    size_t size = blkSize * blkCount;

    if (httpRqt->verbose > 1)
        fprintf(stderr, "-- httpBodyCB: blkSize=%ld blkCount=%ld\n", blkSize, blkCount);

    // final callback is called from multiCheckInfoCB when CURLMSG_DONE
    if (!data) return 0;

    httpRqt->body = realloc(httpRqt->body, httpRqt->bodyLen + size + 1);
    if (!httpRqt->body)
        return 0; // hoops

    memcpy(&(httpRqt->body[httpRqt->bodyLen]), data, size);
    httpRqt->bodyLen += size;
    httpRqt->body[httpRqt->bodyLen] = 0;

    return size;
}

// callback might be called as many time as needed to transfert all data
static size_t httpHeadersCB(void *data, size_t blkSize, size_t blkCount, void *ctx)
{
    httpRqtT *httpRqt = (httpRqtT *)ctx;
    assert(httpRqt->magic == MAGIC_HTTP_RQT);
    size_t size = blkSize * blkCount;

    if (httpRqt->verbose > 2)
        fprintf(stderr, "-- httpHeadersCB: blkSize=%ld blkCount=%ld\n", blkSize, blkCount);

    // final callback is called from multiCheckInfoCB when CURLMSG_DONE
    if (!data)
        return 0;

    httpRqt->headers = realloc(httpRqt->headers, httpRqt->hdrLen + size + 1);
    if (!httpRqt->headers)
        return 0; // hoops

    memcpy(&(httpRqt->headers[httpRqt->hdrLen]), data, size);
    httpRqt->hdrLen += size;
    httpRqt->headers[httpRqt->hdrLen] = 0;

    return size;
}

static void multiCheckInfoCB(httpPoolT *httpPool)
{
    int count;
    CURLMsg *msg;

    // read action resulting messages
    while ((msg = curl_multi_info_read(httpPool->multi, &count)))
    {
        if (httpPool->verbose > 2)
            fprintf(stderr, "-- multiCheckInfoCB: status=%d \n", msg->msg);

        if (msg->msg == CURLMSG_DONE)
        {
            httpRqtT *httpRqt;

            // this is a httpPool request 1st search for easyhandle
            CURL *easy = msg->easy_handle;
            CURLcode estatus = msg->data.result;

            // retreive httpRqt from private easy handle
            if (httpPool->verbose > 1) fprintf(stderr, "-- multiCheckInfoCB: done\n");

            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &httpRqt);

            if (estatus != CURLE_OK)  {
                char * url, *message;
                int len;
                curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &url);

                len= asprintf (&message, "[request-error] status=%d error='%s' url=[%s]", estatus, curl_easy_strerror(estatus), url);
                if (httpPool->verbose)  fprintf(stderr, "\n--- %s\n", message);
                httpRqt->status=-((long)estatus);
                httpRqt->body= message;
                httpRqt->length=len;
            } else {
                curl_easy_getinfo(easy, CURLINFO_SIZE_DOWNLOAD, &httpRqt->length);
                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &httpRqt->status);
                curl_easy_getinfo(easy, CURLINFO_CONTENT_TYPE, &httpRqt->ctype);
            }

            // do some clean up
            curl_multi_remove_handle(httpPool->multi, easy);
            curl_easy_cleanup(easy);

            // compute request elapsed time
            clock_gettime(CLOCK_MONOTONIC, &httpRqt->stopTime);
            httpRqt->msTime = (httpRqt->stopTime.tv_nsec - httpRqt->startTime.tv_nsec) / 1000000 + (httpRqt->stopTime.tv_sec - httpRqt->startTime.tv_sec) * 1000;

            // call request callback (note: callback should free httpRqt)
            httpRqtActionT status = httpRqt->callback(httpRqt);
            if (status == HTTP_HANDLE_FREE)
            {
                if (httpRqt->freeCtx && httpRqt->userData) httpRqt->freeCtx(httpRqt->userData);
                if (httpRqt->body) free (httpRqt->body);
                free(httpRqt);
            }

            break;
        }
    }
}

// call from glue evtLoop. Map event name and pass event to curl action loop
int httpOnSocketCB(httpPoolT *httpPool, int sock, int action)
{
    assert(httpPool->magic == MAGIC_HTTP_POOL);
    int running = 0;

    if (httpPool->verbose > 2) fprintf(stderr, "httpOnSocketCB: sock=%d action=%d\n", sock, action);
    CURLMcode status = curl_multi_socket_action(httpPool->multi, sock, action, &running);
    if (status != CURLM_OK)
        goto OnErrorExit;

    multiCheckInfoCB(httpPool);
    return 0;

OnErrorExit:
    fprintf(stderr, "[curl-multi-action-fail]: curl_multi_socket_action fail (httpOnSocketCB)");
    return -1;
}

// called from glue event loop as Curl needs curl_multi_socket_action to be called regularly
int httpOnTimerCB(httpPoolT *httpPool)
{
    assert(httpPool->magic == MAGIC_HTTP_POOL);
    int running = 0;

    // timer transfers request to socket action (don't use curl_multi_perform)
    int err = curl_multi_socket_action(httpPool->multi, CURL_SOCKET_TIMEOUT, 0, &running);
    if (err != CURLM_OK)
        goto OnErrorExit;

    multiCheckInfoCB(httpPool);
    return 0;

OnErrorExit:
    fprintf(stderr, "multiOnTimerCB: curl_multi_socket_action fail\n");
    return -1;
}

static int httpSendQuery(httpPoolT *httpPool, const char *url, const httpOptsT *opts, httpKeyValT *tokens, void *datas, long datalen, httpRqtCbT callback, void *ctx)
{
    httpRqtT *httpRqt = calloc(1, sizeof(httpRqtT));
    httpRqt->magic = MAGIC_HTTP_RQT;
    httpRqt->easy = curl_easy_init();
    httpRqt->callback = callback;
    httpRqt->userData = ctx;
    clock_gettime(CLOCK_MONOTONIC, &httpRqt->startTime);

    char header[DFLT_HEADER_MAX_LEN];
    struct curl_slist *rqtHeaders = NULL;

    curl_easy_setopt(httpRqt->easy, CURLOPT_URL, url);
    curl_easy_setopt(httpRqt->easy, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(httpRqt->easy, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(httpRqt->easy, CURLOPT_HEADER, 0L); // do not pass header to bodyCB
    curl_easy_setopt(httpRqt->easy, CURLOPT_WRITEFUNCTION, httpBodyCB);
    curl_easy_setopt(httpRqt->easy, CURLOPT_HEADERFUNCTION, httpHeadersCB);
    curl_easy_setopt(httpRqt->easy, CURLOPT_ERRORBUFFER, httpRqt->error);
    curl_easy_setopt(httpRqt->easy, CURLOPT_HEADERDATA, httpRqt);
    curl_easy_setopt(httpRqt->easy, CURLOPT_WRITEDATA, httpRqt);
    curl_easy_setopt(httpRqt->easy, CURLOPT_PRIVATE, httpRqt);

    if (tokens) for (int idx = 0; tokens[idx].tag; idx++)  {
            snprintf(header, sizeof(header), "%s: %s", tokens[idx].tag, tokens[idx].value);
            rqtHeaders = curl_slist_append(rqtHeaders, header);
        }

    if (opts) {
        if (opts->headers) for (int idx = 0; opts->headers[idx].tag; idx++)   {
            snprintf(header, sizeof(header), "%s: %s", opts->headers[idx].tag, opts->headers[idx].value);
            rqtHeaders = curl_slist_append(rqtHeaders, header);
        }

        if (opts->freeCtx) httpRqt->freeCtx = opts->freeCtx;
        if (opts->follow) curl_easy_setopt(httpRqt->easy, CURLOPT_FOLLOWLOCATION, opts->follow);
        if (opts->verbose)  curl_easy_setopt(httpRqt->easy, CURLOPT_VERBOSE, opts->verbose);
        if (opts->agent) curl_easy_setopt(httpRqt->easy, CURLOPT_USERAGENT, opts->agent);
        if (opts->timeout)  curl_easy_setopt(httpRqt->easy, CURLOPT_TIMEOUT, opts->timeout);
        if (opts->sslchk) {
            curl_easy_setopt(httpRqt->easy, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(httpRqt->easy, CURLOPT_SSL_VERIFYHOST, 1L);
        }
        if (opts->sslcert) curl_easy_setopt(httpRqt->easy, CURLOPT_SSLCERT, opts->sslcert);
        if (opts->sslkey)  curl_easy_setopt(httpRqt->easy, CURLOPT_SSLKEY, opts->sslkey);
        if (opts->maxsz)   curl_easy_setopt(httpRqt->easy, CURLOPT_MAXFILESIZE, opts->maxsz);
        if (opts->speedlow)curl_easy_setopt(httpRqt->easy, CURLOPT_LOW_SPEED_TIME, opts->speedlow);
        if (opts->speedlimit) curl_easy_setopt(httpRqt->easy, CURLOPT_LOW_SPEED_LIMIT, opts->speedlimit);
        if (opts->maxredir)   curl_easy_setopt(httpRqt->easy, CURLOPT_MAXREDIRS, opts->maxredir);
        if (opts->username)   curl_easy_setopt(httpRqt->easy, CURLOPT_USERNAME, opts->username);
        if (opts->password)   curl_easy_setopt(httpRqt->easy, CURLOPT_PASSWORD, opts->password);
    }

    if (datas)
    { // raw post
        curl_easy_setopt(httpRqt->easy, CURLOPT_POSTFIELDSIZE, datalen);
        curl_easy_setopt(httpRqt->easy, CURLOPT_POST, 1L);
        curl_easy_setopt(httpRqt->easy, CURLOPT_POSTFIELDS, datas);
    }

    // add header into final request
    if (rqtHeaders)
        curl_easy_setopt(httpRqt->easy, CURLOPT_HTTPHEADER, rqtHeaders);

    if (httpPool)
    {
        CURLMcode mstatus;
        httpRqt->verbose = httpPool->verbose;

        // if httpPool add handle and run asynchronously
        mstatus = curl_multi_add_handle(httpPool->multi, httpRqt->easy);
        if (mstatus != CURLM_OK)
        {
            fprintf(stderr, "[curl-multi-fail] curl curl_multi_add_handle fail url=%s error=%s (httpSendQuery)", url, curl_multi_strerror(mstatus));
            goto OnErrorExit;
        }
    }
    else
    {
        CURLcode estatus;
        // no event loop synchronous call
        estatus = curl_easy_perform(httpRqt->easy);
        if (estatus != CURLE_OK)
        {
            fprintf(stderr, "utilsSendRqt: curl request fail url=%s error=%s", url, curl_easy_strerror(estatus));
            goto OnErrorExit;
        }

        curl_easy_getinfo(httpRqt->easy, CURLINFO_SIZE_DOWNLOAD, &httpRqt->length);
        curl_easy_getinfo(httpRqt->easy, CURLINFO_RESPONSE_CODE, &httpRqt->status);
        curl_easy_getinfo(httpRqt->easy, CURLINFO_CONTENT_TYPE, &httpRqt->ctype);

        // compute elapsed time and call request callback
        clock_gettime(CLOCK_MONOTONIC, &httpRqt->stopTime);
        httpRqt->msTime = (httpRqt->stopTime.tv_nsec - httpRqt->startTime.tv_nsec) / 1000000 + (httpRqt->stopTime.tv_sec - httpRqt->startTime.tv_sec) * 1000;

        // call request callback (note: callback should free httpRqt)
        httpRqtActionT status = httpRqt->callback(httpRqt);
        if (status == HTTP_HANDLE_FREE)
        {
            if (httpRqt->freeCtx && httpRqt->userData)
                httpRqt->freeCtx(httpRqt->userData);
            free(httpRqt);
        }

        // we're done
        curl_easy_cleanup(httpRqt->easy);
    }
    return 0;

OnErrorExit:
    free(httpRqt);
    return 1;
}

int httpSendPost(httpPoolT *httpPool, const char *url, const httpOptsT *opts, httpKeyValT *tokens, void *datas, long len, httpRqtCbT callback, void *ctx)
{
    return httpSendQuery(httpPool, url, opts, tokens, datas, len, callback, ctx);
}

int httpSendGet(httpPoolT *httpPool, const char *url, const httpOptsT *opts, httpKeyValT *tokens, httpRqtCbT callback, void *ctx)
{
    return httpSendQuery(httpPool, url, opts, tokens, NULL, 0, callback, ctx);
}

// create systemd source event and attach http processing callback to sock fd
static int multiSetSockCB(CURL *easy, int sock, int action, void *userdata, void *sockp)
{
    httpPoolT *httpPool = (httpPoolT *)userdata;
    assert(httpPool->magic == MAGIC_HTTP_POOL);

    if (httpPool->verbose > 1)
    {
        if (action == CURL_POLL_REMOVE)
            fprintf(stderr, "[multi-sock-remove] sock=%d (multiSetSockCB)\n", sock);
        else if (!sockp)
            fprintf(stderr, "[multi-sock-insert] sock=%d (multiSetSockCB)\n", sock);
    }
    int err = httpPool->callback->multiSocket(httpPool, easy, sock, action, sockp);
    if (err && action != CURL_POLL_REMOVE)
        fprintf(stderr, "[curl-source-attach-fail] curl_multi_assign failed (evtSetSocketCB)");

    return err;
}

static int multiSetTimerCB(CURLM *curl, long timeout, void *ctx)
{
    httpPoolT *httpPool = (httpPoolT *)ctx;
    assert(httpPool->magic == MAGIC_HTTP_POOL);

    if (httpPool->verbose > 1)
        fprintf(stderr, "-- multiSetTimerCB timeout=%ld\n", timeout);
    int err = httpPool->callback->multiTimer(httpPool, timeout);
    if (err)
        fprintf(stderr, "[afb-timer-fail] afb_sched_post_job fail error=%d (multiSetTimerCB)", err);

    return err;
}

// Create CURL multi httpPool and attach it to systemd evtLoop
httpPoolT *httpCreatePool(void *evtLoop, httpCallbacksT *mainLoopCbs, int verbose)
{

    // First call initialise global CURL static data
    static int initialised = 0;
    if (!initialised)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        initialised = 1;
    }
    httpPoolT *httpPool;
    httpPool = calloc(1, sizeof(httpPoolT));
    httpPool->magic = MAGIC_HTTP_POOL;
    httpPool->verbose = verbose;
    httpPool->callback = mainLoopCbs;
    if (verbose > 1)
        fprintf(stderr, "[httpPool-create-async] multi curl pool initialized\n");

    // add mainloop to httpPool
    httpPool->evtLoop = evtLoop;

    httpPool->multi = curl_multi_init();
    if (!httpPool->multi)
        goto OnErrorExit;

    curl_multi_setopt(httpPool->multi, CURLMOPT_SOCKETFUNCTION, multiSetSockCB);
    curl_multi_setopt(httpPool->multi, CURLMOPT_TIMERFUNCTION, multiSetTimerCB);
    curl_multi_setopt(httpPool->multi, CURLMOPT_SOCKETDATA, httpPool);
    curl_multi_setopt(httpPool->multi, CURLMOPT_TIMERDATA, httpPool);

    return httpPool;

OnErrorExit:
    fprintf(stderr, "[httpPool-create-fail] hoop curl_multi_init failed (httpCreatePool)");
    free(httpPool);
    return NULL;
}

// build request with query
int httpBuildQuery(const char *uid, char *response, size_t maxlen, const char *prefix, const char *url, httpKeyValT *query)
{
    size_t index = 0;
    maxlen = maxlen - 1; // space for '\0'

    // hoops nothing to build url
    if (!prefix && !url)
        goto OnErrorExit;

    // place prefix
    if (prefix)
    {
        for (int idx = 0; prefix[idx]; idx++)
        {
            response[index++] = prefix[idx];
            if (index == maxlen)
                goto OnErrorExit;
        }
        response[index++] = '/';
    }

    // place url
    if (url)
    {
        for (int idx = 0; url[idx]; idx++)
        {
            response[index++] = url[idx];
            if (index == maxlen)
                goto OnErrorExit;
        }
    }

    // loop on query arguments
    if (query) {
        // if no static query params initialize params list
        if (response[index-1] != '&') response[index++] = '?';

        for (int idx = 0; query[idx].tag; idx++)
        {
            for (int jdx = 0; query[idx].tag[jdx]; jdx++)
            {
                response[index++] = query[idx].tag[jdx];
                if (index == maxlen)
                    goto OnErrorExit;
            }
            if (query[idx].value) {
                response[index++] = '=';
                for (int jdx = 0; query[idx].value[jdx]; jdx++)
                {
                    response[index++] = query[idx].value[jdx];
                    if (index == maxlen)
                        goto OnErrorExit;
                }
            }
            response[index++] = '&';
        }
    }
    response[index] = '\0'; // remove last '&'
    return 0;

OnErrorExit:
    fprintf(stderr, "[url-too-long] idp=%s url=%s cannot add query to url (httpMakeRequest)", uid, url);
    return 1;
}

// convert a string into base64
char * httpEncode64 (const char* inputData, size_t inputLen) {
    if (!inputLen) inputLen=strlen(inputData);
    int status;
    char *data64;
    size_t len64;

    status= wrap_base64_encode ((uint8_t*)inputData, inputLen, &data64, &len64,0,1,0);
    if (status) goto OnErrorExit;

    return (data64);

OnErrorExit:
    if (data64) free (data64);
    return NULL;
}

// decode a string into base64
char * httpDecode64 (const char* inputData, size_t inputLen, int url) {
    if (!inputLen) inputLen=strlen(inputData);
    int status;
    char *data64;
    size_t len64;

    status= wrap_base64_decode (inputData, inputLen, (uint8_t**)&data64, &len64, url);
    if (status) goto OnErrorExit;

    data64[len64]='\0';
    return (data64);

OnErrorExit:
    if (data64) free (data64);
    return NULL;
}