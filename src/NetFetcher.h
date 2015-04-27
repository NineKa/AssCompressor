#ifndef __class__NetFetcher__
#define __class__NetFetcher__
#include <curl/curl.h>
#include <string>
#include <exception>
#include <cstdlib>
#include <mutex>
#include <vector>
#include <thread>

#define Error_CURL_NULL_PTR "Unable to init curl, nullptr returned after malloc"
#define Error_CURL_EMPTY_URL "Invalid URL (empty)"
#define Error_CURL_INVALID_PATH "Invalid path (fopen return NULL)"
#define Error_CURL_CONNECTION_INTERRUPTED "Connection interrupted"

struct NetFetcherProgress{
    curl_off_t download_total_byte;
    curl_off_t download_current_byte;
    curl_off_t upload_total_byte;
    curl_off_t upload_current_byte;
    double download_percentage;
    double upload_percentage;
};

class NetFetcher{
public:
    NetFetcher(std::string& _url):NetFetcher(){this->url = _url;}
    NetFetcher(const char* _url):NetFetcher(){this->url = std::string(_url);}
	NetFetcher();
	~NetFetcher();
    void setURL(std::string& _url){this->url = _url;}
    void setURL(const char* _url){
        std::string _url_str(_url);
        this->setURL(_url_str);
    }
    std::string getURL() {return this->url;}
    std::string getBufferedData() {return this->data_buffer;}
    void appendHeader(const char* header_data){this->header = curl_slist_append(this->header, header_data);}
    void setWriteData(void* _in_writeData){curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEDATA, _in_writeData);}
    void setWriteFunction(size_t (*_in_writeFunction)(void*,size_t,size_t,void*)){curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEFUNCTION, _in_writeFunction);}
    void setProgressFunction(int(*_in_progressFunction)(void*,curl_off_t,curl_off_t,curl_off_t,curl_off_t)){
        curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFOFUNCTION, _in_progressFunction);
    }
    void setProgressFunction(int(*_in_progressFunction)(NetFetcherProgress*,curl_off_t,curl_off_t,curl_off_t,curl_off_t)){
        curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFOFUNCTION, _in_progressFunction);
    }
    void setProgressData(void* _progressData){curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFODATA, _progressData);}
    void setAllowProgress(bool _allowed){curl_easy_setopt(this->curl_ptr, CURLOPT_NOPROGRESS, (_allowed)?0L:1L);}
    void setTimeout(long _timeout){curl_easy_setopt(this->curl_ptr, CURLOPT_TIMEOUT, _timeout);}
    void setConnectionTimeout(long _timeout){curl_easy_setopt(this->curl_ptr, CURLOPT_CONNECTTIMEOUT, _timeout);}
    void setAcceptEncoding(const char* _encoding){curl_easy_setopt(this->curl_ptr, CURLOPT_ACCEPT_ENCODING, _encoding);}
    void setFollowLocation(bool _allowed){curl_easy_setopt(this->curl_ptr, CURLOPT_FOLLOWLOCATION, (_allowed)?1L:0L);}
    void setUserAgent(const char* _agent){curl_easy_setopt(this->curl_ptr, CURLOPT_USERAGENT, _agent);}
    void setSilence(bool _silence){this->silence = !_silence;}
    void perform(){
        curl_easy_setopt(this->curl_ptr, CURLOPT_URL, this->url.c_str());
        curl_easy_setopt(this->curl_ptr, CURLOPT_HTTPHEADER, this->header);
        CURLcode statue = curl_easy_perform(this->curl_ptr);
        while (statue==CURLE_GOT_NOTHING) {
            statue = curl_easy_perform(this->curl_ptr);
        }
        if (statue!=CURLE_OK && this->silence){
            const std::string error_msg(this->curl_error_buffer);
            throw std::runtime_error(error_msg);
        }
    }
    long getRespondCode(){
        long respondCode = 200;
        curl_easy_getinfo(this->curl_ptr, CURLINFO_RESPONSE_CODE, &respondCode);
        return respondCode;
    }
	std::string quickFetch();
	std::string quickFetch(std::string&);
    std::string quickFetch(const char*);
    
    void quickDownload(const char*, const char*);
    void quickDownload(const char*);
    
    void reset(){
        curl_easy_cleanup(this->curl_ptr);
        this->url = "";
        this->data_buffer = "";
        this->curl_ptr = curl_easy_init();
        if (this->curl_ptr == nullptr && this->silence){
            const std::string curl_null_ptr_err(Error_CURL_NULL_PTR);
            throw std::runtime_error(curl_null_ptr_err);
        }
        curl_easy_setopt(this->curl_ptr, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEFUNCTION, this->curlWriteFunction);
        curl_easy_setopt(this->curl_ptr, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(this->curl_ptr, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(this->curl_ptr, CURLOPT_ERRORBUFFER, this->curl_error_buffer);
        curl_easy_setopt(this->curl_ptr, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(this->curl_ptr, CURLOPT_HTTPHEADER, this->header);
        curl_easy_setopt(this->curl_ptr, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(this->curl_ptr, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFOFUNCTION, this->curlProgressionFunction);
    }
    
    std::string& getRefBufferedData() {return this->data_buffer;}
private:
    bool silence = true;
	std::string url = "";
	CURL* curl_ptr = nullptr;
    curl_slist* header = nullptr;
    NetFetcherProgress progress;
    char curl_error_buffer[CURL_ERROR_SIZE];
	std::string data_buffer = "";
    std::string user_agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_2) AppleWebKit/600.4.10 (KHTML, like Gecko) Version/8.0.4 Safari/600.4.10";
protected:
    FILE* file_stream = nullptr;
    static size_t curlWriteFunction(char*,size_t,size_t,NetFetcher*);
    static size_t curlDownloadFunction(void*,size_t,size_t,NetFetcher*);
    static int curlProgressionFunction(NetFetcher*,curl_off_t,curl_off_t,curl_off_t,curl_off_t);
};

NetFetcher::NetFetcher(){
	this->url = "";
	this->data_buffer = "";
    this->curl_ptr = curl_easy_init();
    if (this->curl_ptr == nullptr && this->silence){
        const std::string curl_null_ptr_err(Error_CURL_NULL_PTR);
        throw std::runtime_error(curl_null_ptr_err);
    }
    curl_easy_setopt(this->curl_ptr, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(this->curl_ptr, CURLOPT_WRITEFUNCTION, this->curlWriteFunction);
    curl_easy_setopt(this->curl_ptr, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(this->curl_ptr, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(this->curl_ptr, CURLOPT_ERRORBUFFER, this->curl_error_buffer);
    curl_easy_setopt(this->curl_ptr, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(this->curl_ptr, CURLOPT_HTTPHEADER, this->header);
    curl_easy_setopt(this->curl_ptr, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(this->curl_ptr, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(this->curl_ptr, CURLOPT_XFERINFOFUNCTION, this->curlProgressionFunction);
    curl_easy_setopt(this->curl_ptr, CURLOPT_USERAGENT, this->user_agent.c_str());
}

NetFetcher::~NetFetcher(){
    curl_easy_cleanup(this->curl_ptr);
    curl_free(this->header);
}

size_t NetFetcher::curlWriteFunction(char* ptr,
									 size_t size,
									 size_t nmemb,
                                     NetFetcher* obj){
    if (ptr==nullptr) {
        const std::string connection_interrupted(Error_CURL_CONNECTION_INTERRUPTED);
        throw std::runtime_error(connection_interrupted);
    }
	obj->getRefBufferedData().append(ptr,size*nmemb);
	return size*nmemb;
}

size_t NetFetcher::curlDownloadFunction(void* ptr,
                                        size_t size,
                                        size_t nmemb,
                                        NetFetcher* obj){
    if (ptr == nullptr){
        const std::string connection_interrupted(Error_CURL_CONNECTION_INTERRUPTED);
        throw std::runtime_error(connection_interrupted);
    }
    return fwrite(ptr, size, nmemb, obj->file_stream);
}

int NetFetcher::curlProgressionFunction(NetFetcher* obj,
                                        curl_off_t dtotoal,
                                        curl_off_t dnow,
                                        curl_off_t utotal,
                                        curl_off_t unow){
    NetFetcherProgress* recorder=&obj->progress;
    recorder->download_total_byte = dtotoal;
    recorder->download_current_byte = dnow;
    recorder->upload_total_byte = utotal;
    recorder->upload_current_byte = unow;
    recorder->download_percentage = (dtotoal!=0)?(double)dnow/(double)dtotoal:0;
    recorder->upload_percentage = (utotal!=0)?(double)unow/(double)utotal:0;
    printf("Download: %.3f%% Upload: %.3f%%", recorder->download_percentage, recorder->upload_percentage);
    return 0;
}

std::string NetFetcher::quickFetch(){
    this->data_buffer = "";
    CURL* local_curl_handler = curl_easy_init();
    if (local_curl_handler == nullptr && this->silence) {
        const std::string curl_null_ptr_err(Error_CURL_NULL_PTR);
        throw std::runtime_error(curl_null_ptr_err);
    }
    if (this->url == "" && this->silence){
        const std::string curl_empty_url(Error_CURL_EMPTY_URL);
        throw std::runtime_error(curl_empty_url);
    }
    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(local_curl_handler, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(local_curl_handler, CURLOPT_WRITEFUNCTION, this->curlWriteFunction);
    curl_easy_setopt(local_curl_handler, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(local_curl_handler, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(local_curl_handler, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(local_curl_handler, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(local_curl_handler, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(local_curl_handler, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(local_curl_handler, CURLOPT_HTTPHEADER, this->header);
    curl_easy_setopt(local_curl_handler, CURLOPT_USERAGENT, this->user_agent.c_str());
    if (curl_easy_perform(local_curl_handler)!=CURLE_OK && this->silence) {
        curl_easy_cleanup(local_curl_handler);
        local_curl_handler = nullptr;
        std::string error_des(errorBuffer);
        throw std::runtime_error(error_des);
    }
    curl_easy_cleanup(local_curl_handler);
    local_curl_handler = nullptr;
    this->url = "";
    return this->data_buffer;
}

std::string NetFetcher::quickFetch(std::string& _in_url){ this->url = _in_url; return this->quickFetch();}
std::string NetFetcher::quickFetch(const char* _in_url){ this->url = std::string(_in_url); return this->quickFetch();}

void NetFetcher::quickDownload(const char* _url, const char* _des){ this->url = std::string(_url); this->quickDownload(_des);}

void NetFetcher::quickDownload(const char* _des){
    if (this->url == "" && this->silence){
        const std::string curl_empty_url(Error_CURL_EMPTY_URL);
        throw std::runtime_error(curl_empty_url);
    }
    this->file_stream = fopen(_des, "wb");
    CURL* local_curl_handler = curl_easy_init();
    if (local_curl_handler == nullptr && this->silence) {
        const std::string curl_null_ptr_err(Error_CURL_NULL_PTR);
        throw std::runtime_error(curl_null_ptr_err);
    }
    if (this->file_stream == nullptr && this->silence){
        const std::string curl_invalid_path(Error_CURL_INVALID_PATH);
        throw std::runtime_error(curl_invalid_path);
    }
    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(local_curl_handler, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(local_curl_handler, CURLOPT_WRITEFUNCTION, this->curlDownloadFunction);
    curl_easy_setopt(local_curl_handler, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(local_curl_handler, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(local_curl_handler, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(local_curl_handler, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(local_curl_handler, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(local_curl_handler, CURLOPT_HTTPHEADER, this->header);
    curl_easy_setopt(local_curl_handler, CURLOPT_USERAGENT, this->user_agent.c_str());
    if (curl_easy_perform(local_curl_handler) != CURLE_OK && this->silence) {
        fclose(this->file_stream);
        this->file_stream = nullptr;
        curl_easy_cleanup(local_curl_handler);
        local_curl_handler = nullptr;
        std::string error_des(errorBuffer);
        throw std::runtime_error(error_des);
    }
    curl_easy_cleanup(local_curl_handler);
    local_curl_handler = nullptr;
    fclose(this->file_stream);
    this->file_stream = nullptr;
}
#endif
