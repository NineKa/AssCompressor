#ifndef AssCompressor_netFetcher_H
#define AssCompressor_netFetcher_H

#include <curl/curl.h>
#include <string>
#include <exception>
#include <cstdio>
#include <map>
#include <utility>
#include <cstring>
#include "configure.h"
#include "pipeError.h"

namespace netFetcher{
    size_t curlDefWriteFunc(char*, size_t, size_t, void*);
    std::string quickFetch(const char* _url);
    std::string quickFetch(const char* _url, const char* _cookie);
    std::string urlEncode(std::string&);
    std::string urlDecode(std::string&);
    
    typedef std::map<std::string, std::string>      cookieList;
    typedef std::pair<std::string, std::string>     cookie;
    
    std::string cookieString(cookieList&);
};

size_t netFetcher::curlDefWriteFunc(char* ptr,
                                    size_t size,
                                    size_t nmemb,
                                    void* writeData){
    if (ptr == nullptr) return 0L;
    static_cast<std::string*>(writeData)->append(ptr, size*nmemb);
    return size*nmemb;
}

std::string netFetcher::quickFetch(const char* _url){
    return netFetcher::quickFetch(_url, "");
}

std::string netFetcher::quickFetch(const char* _url, const char* _cookie){
    std::string bufferData;
    CURL* curlHandle = curl_easy_init();
    if (curlHandle == nullptr) throw std::runtime_error(CURL_INIT_FAIL);
    curl_easy_setopt(curlHandle, CURLOPT_URL, _url);
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, quickFetch_UserAgent);
    curl_easy_setopt(curlHandle,
                     CURLOPT_CONNECTTIMEOUT_MS, quickFetch_ConnectTimeoutMS);
    curl_easy_setopt(curlHandle,
                     CURLOPT_CONNECTTIMEOUT_MS, quickFetch_TimeoutMS);
    curl_easy_setopt(curlHandle,
                     CURLOPT_WRITEDATA, static_cast<void*>(&bufferData));
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, curlDefWriteFunc);
    curl_easy_setopt(curlHandle, CURLOPT_ENCODING, "");
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION,
                     (quickFetch_FollowRedirection)? 1L : 0L);
    
    if (strcmp("", _cookie)!=0) {
        curl_easy_setopt(curlHandle, CURLOPT_COOKIE, _cookie);
    }
    
    CURLcode response = curl_easy_perform(curlHandle);
    curl_easy_cleanup(curlHandle);
    switch (response) {
        case CURLE_OK: break;
        case CURLE_OPERATION_TIMEDOUT:
            throw std::runtime_error(CURL_TIMEOUT); break;
        case CURLE_FAILED_INIT:
            throw std::runtime_error(CURL_INIT_FAIL); break;
        default:
            throw std::runtime_error(CURL_EXCEPTION);
            break;
    }
    return bufferData;
}

std::string netFetcher::cookieString(cookieList& _list){
    const std::string equalSign("=");
    const std::string semiColm(";");
    const std::string spaceSign(" ");
    CURL* encodehandle = curl_easy_init();
    if (encodehandle == nullptr) throw std::runtime_error(CURL_INIT_FAIL);
    std::string raw_string = "";
    for (cookieList::iterator iter = _list.begin();
         iter != _list.end();
         iter++) {
        char* encodedstr_first= curl_easy_escape(encodehandle,
                                                 iter->first.c_str(),
                                                 (int)(iter->first.length()));
        char* encodedstr_secon= curl_easy_escape(encodehandle,
                                                 iter->second.c_str(),
                                                 (int)iter->second.length());
        
        raw_string+=std::string(encodedstr_first)+
                    equalSign+
                    std::string(encodedstr_secon)+
                    semiColm+spaceSign;
        
        curl_free(static_cast<void*>(encodedstr_first));
        curl_free(static_cast<void*>(encodedstr_secon));
        
    }
    curl_easy_cleanup(encodehandle);
    return raw_string.substr(0, raw_string.length()-2);
}
#endif