#ifndef __Class__BilibiliPageFetcher__
#define __Class__BilibiliPageFetcher__

#include "BilibiliFetcher.h"
#include <regex>
#include <string>
#include <vector>
#include <exception>
#include <algorithm>

#define Error_BilibiliPageFetcher_REGEX_Missmatched "Unable to capture the data"

class BilibiliPageFetcherException : public std::exception{
public:
    BilibiliPageFetcherException(const char* _des) : des{_des}{}
    virtual const char* what(){return this->des;}
private:
    const char* des;
};

class BilibiliPageFetcher : public BilibiliFetcher{
public:
    BilibiliPageFetcher(long _aid) : BilibiliFetcher(BilibiliFetcher::getCID(_aid)){
        this->bilibiliPageURL = constructURL(_aid, 1L);
        this->bilibiliPageFetcherPerform();
    }
    BilibiliPageFetcher(long _aid, long _page) : BilibiliFetcher(BilibiliFetcher::getCID(_aid, _page)){
        this->bilibiliPageURL = constructURL(_aid, _page);
        this->bilibiliPageFetcherPerform();
    }
    BilibiliPageFetcher(const char* _url) : BilibiliFetcher(BilibiliFetcher::getCID(_url)){
        this->bilibiliPageURL = std::string(_url);
        this->bilibiliPageFetcherPerform();
    }
    std::vector<std::string> getKeywords(){return this->keywords;}
    std::string getDescription(){return this->description;}
    std::string getTitle(){return this->title;}
    std::string getUpName(){return this->upName;}
private:
    const char* bilibiliPageURLCons = "http://www.bilibili.com/video/av%ld/index_%ld.html";
    const char* regexExprCStr =
          "<meta name=\"keywords\" content=\"([\\s\\S]*)\" />\n<meta name=\"description\" content=\"([\\s\\S]*)\" />\n<meta name=\"title\" content=\"(.*)\" />[\\s\\S]*</head>";
    std::string bilibiliPageURL = "";
    std::string bilibiliBufferedPage = "";
    std::string title;
    std::string description;
    std::string upName;
    std::vector<std::string> keywords;
    std::string constructURL(long _aid, long _page){
        char* tempStr = new char[1024];
        sprintf(tempStr, bilibiliPageURLCons, _aid, _page);
        return std::string(tempStr);
    }
    void bilibiliPageFetcherPerform(){
        //this->bilibiliBufferedPage = this->quickFetch(bilibiliPageURL);
        
        NetFetcher localFetcher(this->bilibiliPageURL);
        localFetcher.appendHeader("User-Agent:Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_3) AppleWebKit/600.5.17 (KHTML, like Gecko) Version/8.0.5 Safari/600.5.17");
        localFetcher.setConnectionTimeout(0L);
        localFetcher.perform();
        this->bilibiliBufferedPage = localFetcher.getBufferedData();
        
        std::regex keywordsTitleDesRegex(regexExprCStr);
        std::smatch regexCapture;
        std::regex_search(this->bilibiliBufferedPage, regexCapture, keywordsTitleDesRegex);
        if (regexCapture.size()!=4){ throw BilibiliPageFetcherException(Error_BilibiliPageFetcher_REGEX_Missmatched);}
        std::string keywordsStr = regexCapture[1];
        this->description = regexCapture[2];
        this->title = regexCapture[3];
        this->SplitString(keywordsStr, this->keywords, ",");
        std::regex upNameRegex("<a href=\".*\" card=\"(.*)\" target=\"_blank\">");
        std::smatch upNameCapture;
        std::regex_search(this->bilibiliBufferedPage, upNameCapture, upNameRegex);
        if (upNameCapture.size()!=2){ throw BilibiliPageFetcherException(Error_BilibiliPageFetcher_REGEX_Missmatched);}
        this->upName = upNameCapture[1];
    }
    static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c){ //@egmkang
        std::string::size_type pos1, pos2;
        pos2 = s.find(c);
        pos1 = 0;
        while(std::string::npos != pos2)
        {
            v.push_back(s.substr(pos1, pos2-pos1));
            
            pos1 = pos2 + c.size();
            pos2 = s.find(c, pos1);
        }
        if(pos1 != s.length())
            v.push_back(s.substr(pos1));
    }
};

#endif