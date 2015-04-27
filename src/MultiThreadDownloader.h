#ifndef __class__MultiThreadDownloader__
#define __class__MultiThreadDownloader__
#include "NetFetcher.h"

#define Error_MultiThreadDownloader_SIZE "Unable to determine the size of the target"

struct DownloaderThreadInfo{
    size_t id;
    size_t startByte;
    size_t stopByte;
};

class MultiThreadDownloaderException : public std::exception{
public:
    MultiThreadDownloaderException(DownloaderThreadInfo* _info, std::string _des) : err_info{_info},err_des{_des} {}
    virtual const char* what(){
        std::string err_print = "";
        char* temp_conversion_id = new char[256];
        char* temp_conversion_startByte = new char[256];
        char* temp_conversion_stopByte = new char[256];
        sprintf(temp_conversion_id, "%ld", err_info->id);
        sprintf(temp_conversion_startByte, "%ld", err_info->startByte);
        sprintf(temp_conversion_stopByte, "%ld", err_info->stopByte);
        err_print = "Error at :" + std::string(temp_conversion_id)+ " (from: "+
        std::string(temp_conversion_startByte)+" to: "+std::string(temp_conversion_stopByte)+
        ") due to: "+ this->err_des;
        delete temp_conversion_id;
        delete temp_conversion_startByte;
        delete temp_conversion_stopByte;
        return err_print.c_str();
    }
    const char* onlyDes(){return err_des.c_str();}
private:
    const DownloaderThreadInfo* err_info;
    const std::string err_des = "";
};

std::mutex coutLocker;

class MultiThreadDownloader_SingleThread : public NetFetcher{
public:
    MultiThreadDownloader_SingleThread(size_t _id,size_t _startByte, size_t _stopByte, const char* _url, const char* _des) : url{_url},des{_des}{
        this->recorder_thread.id = _id;
        this->recorder_thread.startByte = _startByte;
        this->recorder_thread.stopByte = _stopByte;
    }
    void startThread(){
        char temp_str[256];
        sprintf(temp_str, "%ld", this->recorder_thread.id);
        des += ".thread_"+std::string(temp_str);
        this->setURL(this->url);
        this->setTimeout(0L);
        this->setWriteFunction(MultiThreadDownloader_SingleThread::downloadFunction);
        this->setWriteData(this);
        this->file_stream = fopen(this->des.c_str(), "wb");
        sprintf(temp_str, "Range: bytes=%ld-%ld", this->recorder_thread.startByte, this->recorder_thread.stopByte);
        this->appendHeader(temp_str);
        this->setProgressFunction(MultiThreadDownloader_SingleThread::progressFunction);
        this->setProgressData(this);
        this->perform();
        fclose(this->file_stream);
        if (this->getRespondCode()==416) {remove(des.c_str());}
    }
    DownloaderThreadInfo getInfo(){return this->recorder_thread;}
    DownloaderThreadInfo recorder_thread;
private:
    std::string url;
    std::string des;
    static size_t downloadFunction(void* _ptr, size_t _size, size_t _nmemb, void* _stream){
        return MultiThreadDownloader_SingleThread::curlDownloadFunction(_ptr, _size, _nmemb, (MultiThreadDownloader_SingleThread*)_stream);
    }
    static int progressFunction(void* _downloader, curl_off_t _dtotal, curl_off_t _dnow, curl_off_t _utotal, curl_off_t _unow){
        coutLocker.lock();
        printf("[Thread %ld] Download: %.3f%% Upload %.3f%%\r",
               ((MultiThreadDownloader_SingleThread*)_downloader)->recorder_thread.id,
               (_dtotal!=0)?((double)_dnow*100/_dtotal):0,
               (_utotal!=0)?((double)_unow*100/_utotal):0);
        coutLocker.unlock();
        return 0;
    }
};

void MultiThreadDownloader_Empty_Modifer(MultiThreadDownloader_SingleThread* _thread){ _thread->setAllowProgress(false); _thread->setSilence(true); return;}

class MultiThreadDownloader{
public:
    MultiThreadDownloader(size_t _limits) : threads_limit{_limits} {}
    MultiThreadDownloader(size_t _limits, const char* _url) : threads_limit{_limits},url{std::string(_url)} {}
    MultiThreadDownloader(size_t _limits, const char* _url, const char* _des) : threads_limit{_limits},url{std::string(_url)},des{std::string(_des)}{}
    void perform(void (*_modifier)(MultiThreadDownloader_SingleThread*)=MultiThreadDownloader_Empty_Modifer);
    void cleanup();
    void setURL(const char* _url) {this->url = std::string(_url);}
    void setURL(std::string _url) {this->url = _url;}
    void setTarget(const char* _des) {this->des = std::string(_des);}
    void setTarget(std::string _des) {this->des = _des;}
private:
    double getFileSize();
    std::string url = "";
    std::string des = "";
    const size_t threads_limit = 1;
    static size_t probeWriteFunction(void*,size_t,size_t,void*){return -1;}
    static void performSingleThread(MultiThreadDownloader_SingleThread*);
};

void MultiThreadDownloader::performSingleThread(MultiThreadDownloader_SingleThread* _thread_downloader){
    try {
        _thread_downloader->startThread();
        delete _thread_downloader;
    } catch (std::runtime_error& err) {
        throw MultiThreadDownloaderException(&(_thread_downloader->recorder_thread), std::string(err.what()));
    }
}
void MultiThreadDownloader::perform(void (*_modifier)(MultiThreadDownloader_SingleThread*)){
    std::vector<std::thread> thread_pool;
    size_t content_size = (size_t)MultiThreadDownloader::getFileSize();
    for (size_t i=0; i < threads_limit; i++) {
        MultiThreadDownloader_SingleThread* singleThread = new MultiThreadDownloader_SingleThread(i,
                                                                                                  (i==0)?0:(i*(content_size/(threads_limit))+1),
                                                                                                  (i+1)*(content_size/(threads_limit)),
                                                                                                  this->url.c_str(),
                                                                                                  this->des.c_str());
        _modifier(singleThread);
        thread_pool.push_back(std::thread(MultiThreadDownloader::performSingleThread,singleThread));
    }
    for (int i=0; i<thread_pool.size(); i++){
        thread_pool[i].join();
    }
    FILE* finalWrite = fopen(this->des.c_str(), "wb");
    size_t byteCount = 0;
    for (int i=0; i<thread_pool.size(); i++){
        char temp_str[256];
        sprintf(temp_str, "%d", i);
        std::string temp_file_path = this->des;
        temp_file_path += ".thread_"+std::string(temp_str);
        FILE* merge_part = fopen(temp_file_path.c_str(), "rb");
        do{
            if (merge_part == nullptr){break;}
            char moving;
            fread(&moving, sizeof(char), 1, merge_part);
            if (feof(merge_part)) {break;}
            byteCount++;
            fwrite(&moving, sizeof(char), 1, finalWrite);
        }while (!feof(merge_part));
        fclose(merge_part);
    }
    fclose(finalWrite);
    if (byteCount == 0){
        remove(this->des.c_str());
        this->cleanup();
        throw MultiThreadDownloaderException(nullptr, std::string("download target error"));
    }
    this->cleanup();
}

void MultiThreadDownloader::cleanup(){
    for (int i=0; i<this->threads_limit; i++){
        char temp_str[256];
        sprintf(temp_str, "%d", i);
        std::string temp_file_path = this->des;
        temp_file_path += ".thread_"+std::string(temp_str);
        remove(temp_file_path.c_str());
    }
}

double MultiThreadDownloader::getFileSize(){
    CURL* web_handler = curl_easy_init();
    curl_easy_setopt(web_handler, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(web_handler, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(web_handler, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(web_handler, CURLOPT_WRITEFUNCTION, MultiThreadDownloader::probeWriteFunction);
    curl_easy_setopt(web_handler, CURLOPT_WRITEDATA, nullptr);
    double content_length = 0;
    curl_easy_getinfo(web_handler, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
    curl_easy_cleanup(web_handler);
    return content_length;
}
#endif