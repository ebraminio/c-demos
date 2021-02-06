// gcc curl-download.c `pkg-config --libs --cflags libcurl` -Wall && ./a.out
#include <curl/curl.h>
#include <stdio.h>

size_t print_func(void *ptr, size_t size /*always==1*/, size_t nmemb,
                  void *userdata) {
  printf("%.*s", (unsigned)nmemb, (unsigned char *)ptr);
  return nmemb;
}

int main(void) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return 1;
  CURLcode res;
  curl_easy_setopt(curl, CURLOPT_URL, "https://ipinfo.io");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &print_func);
  res = curl_easy_perform(curl);
  if (res != CURLE_OK)
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  curl_easy_cleanup(curl);
  return 0;
}
