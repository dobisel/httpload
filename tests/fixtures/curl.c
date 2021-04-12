#include "logging.h"
#include "fixtures/curl.h"

#include <unistd.h>
#include <curl/curl.h>

int
curl_get(const char *url, char *const outbuff, char *const errbuff) {
    CURL *curl;
    CURLcode res;
    long status;
    FILE *fout;
    FILE *ferr;
    
    fout = fmemopen(outbuff, CURL_BUFFMAX, "wb");
    if (fout == NULL) {
        return ERR;
    }

    ferr = fmemopen(errbuff, CURL_BUFFMAX, "wb");
    if (ferr == NULL) {
        return ERR;
    }

    curl = curl_easy_init();
    if (curl == NULL) {
        return ERR;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_STDERR, ferr);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fout);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return ERR;
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    sleep(.5F);
    fclose(fout);
    fclose(ferr);
    return status;
}


