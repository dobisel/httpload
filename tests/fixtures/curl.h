#ifndef FIXTURE_CURL_H
#define FIXTURE_CURL_H

#define CURL_BUFFMAX    1024 * 2

int
curl_get(const char *url, char *const outbuff, char *const errbuff);
#endif
