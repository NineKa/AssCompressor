#ifndef AssCompressor_bilibiliWebManager_H
#define AssCompressor_bilibiliWebManager_H

#include "configure.h"
#include "netFetcher.h"
#include "pipeError.h"
#include "rapidjson/document.h"
#include "rapidxml/rapidxml.hpp"
#include <exception>
#include <string>
#include <vector>
#include <cstdlib>
#include <limits>
#include <utility>
#include <assert.h>

#ifdef DEBUG
#include <sstream>
#endif

#ifdef CXX11REGEX
    #include <regex>
#else
    #include <boost/regex.hpp>
#endif

class bilibiliWebManager{
    const std::string webURL;
    
    std::string cid;
    std::string aid;
    std::string commentURL;
    
    std::string bufferedWebpage;
    std::string bufferedCommentXML;
    std::string bufferedInterfaceXML;
    std::vector<std::pair<time_t, size_t>> rollAvaliableDate;
    static std::string regexMatchFirst(std::string& , const char*);
    static void stringSplit(std::vector<std::string>&, std::string&, char);
public:
    struct videoSection{
        static const size_t orderEmpty = std::numeric_limits<size_t>::max();
        static const size_t sizeEmpty = std::numeric_limits<size_t>::max();
        static const time_t durEmpty = std::numeric_limits<time_t>::max();
        
        std::string url;
        std::vector<std::string> backup_url;
        size_t order = bilibiliWebManager::videoSection::orderEmpty;
        size_t size  = bilibiliWebManager::videoSection::sizeEmpty;
        time_t duration = bilibiliWebManager::videoSection::durEmpty;
    };
    typedef std::pair<time_t, size_t> rollBackInfo;
    typedef std::vector<rollBackInfo> rollBackList;
    typedef std::vector<std::string>  keywordsList;
    typedef std::vector<videoSection> videoSectionList;
    
    bilibiliWebManager(std::string _url);
    
    std::string getCID(){return this->cid;}
    std::string getAID(){return this->aid;}
    std::string getBufferedWebPage(){return this->bufferedWebpage;}
    std::string getBufferedCommentXML(){return this->bufferedCommentXML;}
    std::string getBufferedInterface(){return this->bufferedInterfaceXML;}
    std::string getTitle(){
        return regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_TitleRegex);
    }
    std::string getDescription(){
        return regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_DesRegex);
    }
    std::string getAuthor(){
        return regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_AuthorRegex);
    }
    std::string getCommentURL(){return this->commentURL;}
    keywordsList getKeywordsList(){
        keywordsList returnList;
        std::string kwStr=regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_KeywordRegex);
        stringSplit(returnList, kwStr, ',');
        return returnList;
    }
    videoSectionList getVideoSectionList();
    time_t getTotalVideoLength();
    
    void fetchInterfaceXML();
    void fetchCommentXML();
    void fetchRollBackDate();
    
    static std::string bilibiliURL(const char* _aid, const char* _index){
        char* buffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
        sprintf(buffer, bilibiliWebManager_URL, _aid, _index);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    static std::string bilibiliURL(const char* _aid){
        return bilibiliWebManager::bilibiliURL(_aid, "1");
    }
    static std::string bilibiliURL(size_t _aid, size_t _index){
        char* aidbuffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
        char* indexbuffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
        sprintf(aidbuffer, "%lu", _aid);
        sprintf(indexbuffer, "%lu", _index);
        std::string returnStr = bilibiliWebManager::bilibiliURL(aidbuffer, indexbuffer);
        free(aidbuffer);
        free(indexbuffer);
        return returnStr;
    }
    static std::string bilibiliURL(size_t _aid){
        return bilibiliWebManager::bilibiliURL(_aid, 1L);
    }
    
    #ifdef DEBUG
    static std::string getAvaliableString(rollBackList);
    static std::string getKeywordsString(keywordsList);
    static std::string getVideoSectionListString(videoSectionList);
    #endif
};

std::string bilibiliWebManager::regexMatchFirst(std::string& _data,
                                                const char* _regex){
    #ifdef CXX11REGEX
    std::smatch pendingResult;
    std::regex_search(_data, pendingResult,
                      std::regex(std::string(_regex)));
    if (pendingResult.empty()) {
        throw std::runtime_error(BILIBILIWEBMANAGER_REGEX_FAIL);
    }
    return pendingResult[1];
    #else
    boost::smatch pendingResult;
    boost::regex_search(_data, pendingResult, boost::regex(std::string(_regex)));
    if (pendingResult.empty()) {
        throw std::runtime_error(BILIBILIWEBMANAGER_REGEX_FAIL);
    }
    return pendingResult[1];
    #endif
}

void bilibiliWebManager::stringSplit(std::vector<std::string>& _list,
                                     std::string& _content,
                                     char _mark){
    std::string bufferStr = "";
    for (std::string::iterator iter = _content.begin();
         iter != _content.end();
         iter++) {
        if (*iter != _mark) {
            bufferStr.append(1,*iter);
        }else{
            _list.push_back(bufferStr);
            bufferStr = "";
        }
    }
    if (!bufferStr.empty()) _list.push_back(bufferStr);
}

bilibiliWebManager::bilibiliWebManager(std::string _url) : webURL(_url){
    std::string temp_cookies;
    
    #if defined(bilibiliWebManager_DedeUserID) && defined(bilibiliWebManager_DedeUserID)
    const std::string dedeUserID(bilibiliWebManager_DedeUserID);
    const std::string sESSDATA(bilibiliWebManager_SESSDATA);
    if (!dedeUserID.empty() && !sESSDATA.empty()) {
        netFetcher::cookieList cookies;
        netFetcher::cookie c_DedeUserID(std::string("DedeUserID"), dedeUserID);
        netFetcher::cookie c_sESSDATA(std::string("SESSDATA"), sESSDATA);
        cookies.insert(c_DedeUserID);
        cookies.insert(c_sESSDATA);
        temp_cookies = netFetcher::cookieString(cookies);
    }else{
        temp_cookies = "";
    }
    this->bufferedWebpage = netFetcher::quickFetch(this->webURL.c_str(), temp_cookies.c_str());
    
    this->cid = regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_CIDRegex);
    this->aid = regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_AIDRegex);
    #else
    this->bufferedWebpage = netFetcher::quickFetch(this->webURL.c_str());
    this->cid = regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_CIDRegex);
    this->aid = regexMatchFirst(this->bufferedWebpage, bilibiliWebManager_AIDRegex);
    #endif
    
    #ifdef bilibiliWebManager_FetchCommentOnConstruct
    this->fetchCommentXML();
    #endif
    
    #ifdef bilibiliWebManager_FetchInterfaceOnConstruct
    this->fetchInterfaceXML();
    #endif
    
    #ifdef bilibiliWebManager_FetchRollBackOnConstruct
    this->fetchRollBackDate();
    #endif
}

bilibiliWebManager::videoSectionList bilibiliWebManager::getVideoSectionList(){
    bilibiliWebManager::videoSectionList returnList;
    char* buffer = (char*)malloc(sizeof(char)*(this->bufferedInterfaceXML.length()+1));
    strcpy(buffer, this->bufferedInterfaceXML.c_str());
    rapidxml::xml_document<> xmlParser;
    try {
        xmlParser.parse<0>(buffer);
    } catch (...) {
        free(buffer);
        xmlParser.clear();
        throw std::runtime_error(BILIBILIWEBMANAGER_INTERFACE_FAIL);
    }
    rapidxml::xml_node<>* rootNode = xmlParser.first_node();
    rapidxml::xml_node<>* durlNode = rootNode->first_node("durl");
    while (durlNode != nullptr) {
        if (strcmp(durlNode->name(), "durl")!=0) {
            durlNode = durlNode->next_sibling();
            continue;
        }
        videoSection newSection;
        rapidxml::xml_node<>* orderNode = durlNode->first_node("order");
        rapidxml::xml_node<>* lengthNode = durlNode->first_node("length");
        rapidxml::xml_node<>* sizeNode = durlNode->first_node("size");
        rapidxml::xml_node<>* urlNode = durlNode->first_node("url");
        rapidxml::xml_node<>* backupNode = durlNode->first_node("backup_url");
        if (orderNode == nullptr) {
            newSection.order = bilibiliWebManager::videoSection::orderEmpty;
        }else{
            newSection.order = static_cast<size_t>(atol(orderNode->first_node()->value()));
        }
        if (lengthNode == nullptr) {
            newSection.duration = bilibiliWebManager::videoSection::durEmpty;
        }else{
            newSection.duration = static_cast<time_t>(atol(lengthNode->first_node()->value()));
        }
        if (sizeNode == nullptr) {
            newSection.size = bilibiliWebManager::videoSection::sizeEmpty;
        }else{
            newSection.size = static_cast<size_t>(atol(sizeNode->first_node()->value()));
        }
        if (urlNode == nullptr) {
            xmlParser.clear();
            free(buffer);
            throw std::runtime_error(BILIBILIWEBMANAGER_INTERFACE_FAIL);
        }
        newSection.url = std::string(urlNode->first_node()->value());
        if (backupNode != nullptr) {
            for (rapidxml::xml_node<>* bkiter = backupNode->first_node("url");
                bkiter != nullptr;
                bkiter = bkiter -> next_sibling()) {
                if (strcmp(bkiter->name(), "url") != 0) continue;
                newSection.backup_url.push_back(std::string(bkiter->first_node()->value()));
            }
        }
        
        returnList.push_back(newSection);
        durlNode = durlNode->next_sibling();
    }
    xmlParser.clear();
    free(buffer);
    return returnList;
}

time_t bilibiliWebManager::getTotalVideoLength(){
    char* buffer = (char*)malloc(sizeof(char)*(this->bufferedInterfaceXML.length()+1));
    strcpy(buffer, this->bufferedInterfaceXML.c_str());
    rapidxml::xml_document<> xmlParser;
    try {
        xmlParser.parse<0>(buffer);
    }catch(...){
        free(buffer);
        throw std::runtime_error(BILIBILIWEBMANAGER_INTERFACE_FAIL);
    }
    rapidxml::xml_node<>* rootNode = xmlParser.first_node();
    rapidxml::xml_node<>* dataNode = rootNode->first_node("timelength");
    time_t returnValue = static_cast<time_t>(atol(dataNode->first_node()->value()));
    xmlParser.clear();
    free(buffer);
    return returnValue;
}

void bilibiliWebManager::fetchCommentXML(){
    char* commentURLBuffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
    sprintf(commentURLBuffer,bilibiliWebManager_CommentURL,this->cid.c_str());
    this->commentURL = std::string(commentURLBuffer);
    this->bufferedCommentXML = netFetcher::quickFetch(commentURLBuffer);
    free(commentURLBuffer);
}

void bilibiliWebManager::fetchInterfaceXML(){
    #ifndef bilibiliWebManager_AppKey
    #error Appkey is required for build
    #else
    char* interfaceURLBuffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
    sprintf(interfaceURLBuffer,bilibiliWebManager_Interface,bilibiliWebManager_AppKey,this->cid.c_str());
    this->bufferedInterfaceXML = netFetcher::quickFetch(interfaceURLBuffer);
    free(interfaceURLBuffer);
    #endif
}

void bilibiliWebManager::fetchRollBackDate(){
    char* rollBackURLBuffer = (char*)malloc(sizeof(char)*bilibiliWebManager_MAXBuffer);
    sprintf(rollBackURLBuffer,bilibiliWebManager_CommentRoll,this->cid.c_str());
    std::string rollBackJSON = netFetcher::quickFetch(rollBackURLBuffer);
    free(rollBackURLBuffer);
    rapidjson::Document jsonHandler;
    jsonHandler.Parse<rapidjson::kParseDefaultFlags>(rollBackJSON.c_str());
    if (jsonHandler.HasParseError()){
        throw std::runtime_error(BILIBILIWEBMANAGER_ROLLBACKJSON_FAIL);
    }
    assert(jsonHandler.IsArray());
    for (rapidjson::SizeType i = 0; i<jsonHandler.Size(); i++) {
        rapidjson::Value& currentNode = jsonHandler[i];
        const char* timestampStr = currentNode["timestamp"].GetString();
        time_t timestamp = static_cast<time_t>(atol(timestampStr));
        const char* numnewStr = currentNode["new"].GetString();
        size_t numnew = static_cast<size_t>(atol(numnewStr));
        std::pair<time_t, size_t> temp (timestamp, numnew);
        this->rollAvaliableDate.push_back(temp);
    }
}

#ifdef DEBUG
std::string bilibiliWebManager::getAvaliableString(bilibiliWebManager::rollBackList _list){
    std::stringstream oStrem;
    for (std::vector<std::pair<time_t, size_t>>::iterator iter = _list.begin();
         iter != _list.end();
         iter ++) {
        oStrem<<"{timestamp:"<<iter->first<<",new:"<<iter->second<<"},";
    }
    std::string returnString;
    oStrem>>returnString;
    return std::string("[")+returnString.substr(0,returnString.length()-1)+std::string("]");
}

std::string bilibiliWebManager::getKeywordsString(bilibiliWebManager::keywordsList _list){
    std::stringstream oStream;
    for (keywordsList::iterator iter = _list.begin();
         iter != _list.end();
         iter++) {
        oStream<<","<<*iter;
    }
    std::string returnStr;
    oStream>>returnStr;
    return std::string("[")+returnStr.substr(1, returnStr.length()-1)+std::string("]");
}

std::string bilibiliWebManager::getVideoSectionListString(bilibiliWebManager::videoSectionList _list){
    std::stringstream oStream;
    for (bilibiliWebManager::videoSectionList::iterator iter = _list.begin();
         iter != _list.end();
         iter++) {
        oStream<<",{order:";
        if (iter->order == bilibiliWebManager::videoSection::orderEmpty){
            oStream<<"NULL";
        }else{
            oStream<<iter->order;
        }
        oStream<<",duration:";
        if (iter->order == bilibiliWebManager::videoSection::durEmpty){
            oStream<<"NULL";
        }else{
            oStream<<iter->duration;
        }
        oStream<<",size:";
        if (iter->size == bilibiliWebManager::videoSection::sizeEmpty){
            oStream<<"NULL";
        }else{
            oStream<<iter->size;
        }
        oStream<<",url:"<<iter->url<<",backup:{";
        for (std::vector<std::string>::iterator iter2 = iter->backup_url.begin();
             iter2 != iter->backup_url.end();
             iter2++) {
            oStream<<*iter2<< ((iter2+1 != iter->backup_url.end())? "," : "");
        }
        oStream<<"}";
    }
    std::string returnStr = "";
    oStream>>returnStr;
    return std::string("[") + returnStr.substr(1, returnStr.length()-1) + std::string("]");
}
#endif

#endif