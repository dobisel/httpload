#include "logging.h"
#include "server.h"
#include "testing.h"

#include <unistd.h>
#include <curl/curl.h>


int get(const char *url) {
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  res = curl_easy_perform(curl);
    
  DEBUG("CURL DOne");
  if(res != CURLE_OK) {
    ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  }

  curl_easy_cleanup(curl);
  return res;
}


void
test_single_packet() {
    struct httpd m = {
        .port = 9090,
        .forks = 1
    };
    log_setlevel(LOG_DEBUG);
    int err = httpd_fork(&m);
    if (err) {
        FATAL("Cannot start http mock server");
    }
    INFO("Listening on port: %d", m.port);
    
    eqint(0, get("http://localhost:9090"));

    httpd_terminate(&m);
    eqint(0, httpd_join(&m));
}

int
main() {
    log_setlevel(LOG_DEBUG);
    test_single_packet();
    return 0;
}
