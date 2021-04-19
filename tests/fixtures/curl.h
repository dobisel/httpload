#ifndef FIXTURE_CURL_H
#define FIXTURE_CURL_H

#include "fixtures/curl.h"
#include <curl/curl.h>

#define CURL_BUFFMAX    1024 * 2

typedef void (*curlhook_t) (CURL *curl);

int
curl_get(const char *url, struct curl_slist *headers, curlhook_t optionscb, 
         char *const outbuff, char *const errbuff);

#define HTTPGET(url) curl_get((url), NULL, NULL, NULL, NULL)
#endif
