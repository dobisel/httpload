#include "logging.h"
#include "fixtures/curl.h"

#include <unistd.h>
#include <curl/curl.h>

int
curl_request(const char *verb, const char *url, struct curl_slist *headers, 
        curlhook_t optionscb, char *const outbuff, char *const errbuff, 
        const char *payload, size_t payloadsize) {
    CURL *curl;
    CURLcode res;
    long status;
    FILE *fout;
    FILE *ferr;
    
    curl = curl_easy_init();
    if (curl == NULL) {
        return ERR;
    }

    if (outbuff) {
        fout = fmemopen(outbuff, CURL_BUFFMAX, "wb");
        if (fout == NULL) {
            return ERR;
        }
        
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fout);
    }

    if (errbuff) {
        ferr = fmemopen(errbuff, CURL_BUFFMAX, "wb");
        if (ferr == NULL) {
            return ERR;
        }

        curl_easy_setopt(curl, CURLOPT_STDERR, ferr);
    }
  
    if (payloadsize) {
        //curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        //curl_easy_setopt(curl, CURLOPT_INFILESIZE, payloadsize);

        /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadsize);
 
        /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    }
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, verb);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    /* Hook */
    if (optionscb != NULL) {
        optionscb(curl);
    }
    
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        WARN("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return ERR;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    sleep(.3F);

    if (outbuff) {
        fclose(fout);
    }

    if (errbuff) {
        fclose(ferr);
    }

    return status;
}
