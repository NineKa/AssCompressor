#ifndef __class__BilibiliFetcher__
#define __class__BilibiliFetcher__

#include "NetFetcher.h"
#include <regex>
#include <exception>
#include <vector>
#include <map>
#include "rapidxml/rapidxml.hpp"
#include "rapidjson/document.h"

#define Error_BilibiliFetcher_NO_CID "No cid provided"
#define Error_BilibiliFetcher_NOCAPUTRE  "CID Not Found on the Page"
#define BilibiliFetcher_UserAgent "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_3) AppleWebKit/600.5.17 (KHTML, like Gecko) Version/8.0.5 Safari/600.5.17"

struct DownloadUrl{
    long order;
    std::string url;
    std::vector<std::string> backupUrl;
};

class BilibiliFetcher : public NetFetcher{
public:
    BilibiliFetcher() : NetFetcher(){}
    BilibiliFetcher(long _cid) : NetFetcher() {this->cid = _cid;}
    BilibiliFetcher(const char* _cid) : NetFetcher() {this->cid = atol(_cid);}
    void performAll(){
        if (this->cid == 0) {
            const std::string err_des(Error_BilibiliFetcher_NO_CID);
            throw std::runtime_error(err_des);
        }
        this->performComment();
        this->performVideo();
        this->performRollBack();
    }
    void performRollBack(){
        if (this->cid == 0) {
            const std::string err_des(Error_BilibiliFetcher_NO_CID);
            throw std::runtime_error(err_des);
        }
        this->avaliableRollBackURL = constructURL(this->bilibiliAvaliableRollBackCons, this->cid);
        std::string rollBackJsonArray = this->quickFetch(this->avaliableRollBackURL.c_str());
        rapidjson::Document rollBackJsonObj;
        rollBackJsonObj.Parse<0>(rollBackJsonArray.c_str());
        if (rollBackJsonObj.HasParseError()) {return;}
        for (int i=0; i<rollBackJsonObj.Size(); i++){
            rapidjson::Value& rollBackDataNode = rollBackJsonObj[i]["timestamp"];
            rapidjson::Value& rollBackNewNode = rollBackJsonObj[i]["new"];
            this->avaliableRollBackDate.push_back(atol(rollBackDataNode.GetString()));
            this->newComment.insert(std::pair<long, long>(atol(rollBackDataNode.GetString()),atol(rollBackNewNode.GetString())));
        }
    }
    void performVideo(){
        if (this->cid == 0) {
            const std::string err_des(Error_BilibiliFetcher_NO_CID);
            throw std::runtime_error(err_des);
        }
        this->videoURL = constructURL(this->bilibiliVideoCons, this->cid);
        this->bufferedVidoXML = this->quickFetch(this->videoURL.c_str());
        BilibiliFetcher::fetchVideoURL(&this->videoURLArray, &this->bufferedVidoXML);
    }
    void performComment(){
        if (this->cid == 0) {
            const std::string err_des(Error_BilibiliFetcher_NO_CID);
            throw std::runtime_error(err_des);
        }
        this->commentURL = constructURL(this->bilibiliCommentCons, this->cid);
        this->bufferedCommentXML = this->quickFetch(this->commentURL.c_str());
    }
    std::string& getBufferedCommentXML() {return this->bufferedCommentXML;}
    std::string& getBufferedVideoXML() {return this->bufferedVidoXML;}
    std::string& getVideoURL() {return this->videoURL;}
    std::string& getCommentURL() {return this->commentURL;}
    std::string& getRollBackDateURL() {return this->avaliableRollBackURL;}
    std::map<long,long>& getDayNewComment() {return this->newComment;}
    static long getCID(long _aid) {return BilibiliFetcher::getCID(_aid, 1L);}
    static long getCID(long _aid, long _page);
    static long getCID(const char* _url);
    std::vector<DownloadUrl>& getVideoURLArray() {return this->videoURLArray;}
    std::vector<long>& getAvaliableRollBackDate() {return this->avaliableRollBackDate;}
private:
    const char* bilibiliCommentCons = "http://comment.bilibili.tv/%ld.xml";
    const char* bilibiliVideoCons = "http://interface.bilibili.tv/playurl?quality=2&sign=847136f0e908ff6868f34f3a81045c25&cid=%ld";
    const char* bilibiliAvaliableRollBackCons = "http://comment.bilibili.com/rolldate,%ld";
    std::string commentURL = "";
    std::string videoURL = "";
    std::string avaliableRollBackURL = "";
    std::map<long, long> newComment;
    std::string constructURL(const char* _formula, const long _data){
        char* temp_str = (char*)malloc(sizeof(char)*(strlen(bilibiliCommentCons)+256));
        sprintf(temp_str, _formula, _data);
        std::string return_str(temp_str);
        free(temp_str);
        temp_str = nullptr;
        return return_str;
    }
    std::string bufferedCommentXML = "";
    std::string bufferedVidoXML = "";
    long cid = 0;
    std::vector<DownloadUrl> videoURLArray;
    std::vector<long> avaliableRollBackDate;
    static void fetchVideoURL(std::vector<DownloadUrl>* _array, std::string* _xmlText){
        rapidxml::xml_document<> parsing_doc;
        char* xmlTextCStr = new char[_xmlText->length()+1];
        strcpy(xmlTextCStr, _xmlText->c_str());
        parsing_doc.parse<0>(xmlTextCStr);
        rapidxml::xml_node<>* parsing_root = parsing_doc.first_node();
        rapidxml::xml_node<>* durl_node = parsing_root->first_node("durl");
        while (durl_node != nullptr) {
            if (strcmp(durl_node->name(),"durl")!=0) {
                durl_node = durl_node -> next_sibling();
                continue;
            }
            DownloadUrl section;
            rapidxml::xml_node<>* url_node = durl_node->first_node("url");
            rapidxml::xml_node<>* backup_node = durl_node->first_node("backup_url");
            rapidxml::xml_node<>* order_node = durl_node->first_node("order");
            section.url = url_node->first_node()->value();
            section.order = atol(order_node->first_node()->value());
            if (backup_node != nullptr){
                rapidxml::xml_node<>* backup_url_node = backup_node->first_node("url");
                while (backup_url_node != nullptr) {
                    section.backupUrl.push_back(backup_url_node->first_node()->value());
                    backup_url_node = backup_url_node->next_sibling();
                }
            }
            _array->push_back(section);
            durl_node = durl_node ->next_sibling();
        }
        delete xmlTextCStr;
    }
};

long BilibiliFetcher::getCID(long _aid, long _page){
    const char* bilibiliPageCons = "http://www.bilibili.com/video/av%ld/index_%ld.html";
    char* url = new char[strlen(bilibiliPageCons)+256+256];
    sprintf(url, bilibiliPageCons, _aid, _page);
    std::string std_url(url);
    delete url;
    return BilibiliFetcher::getCID(std_url.c_str());
}

long BilibiliFetcher::getCID(const char* _url){
    NetFetcher localFetcher(_url);
    localFetcher.appendHeader(BilibiliFetcher_UserAgent);
    localFetcher.appendHeader("Accept:*/*");
    localFetcher.setFollowLocation(true);
    localFetcher.perform();
    std::regex regex_expr("cid=([0-9]*)&");
    std::smatch regex_result;
    std::regex_search(localFetcher.getBufferedData(),
                     regex_result,
                     regex_expr);
    if (regex_result.length() == 0){throw std::runtime_error(Error_BilibiliFetcher_NOCAPUTRE);}
    std::string first_catch = regex_result[1];
    return atol(first_catch.c_str());
}
#endif